import copy
import json
from pathlib import Path
import sys
import tempfile
import unittest
import uuid

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from library import encode
from publication import Queue, publish
from worker import process
from test_library import sample


class Remote:
    repository = 'CWO4PapaBear/Area-52-Build-Architect-Library'

    def __init__(self):
        self.current = 'initial'
        self.commits = {'initial': {'LICENSE': b'preserve', 'README.md': b'preserve'}}
        self.parents = {}
        self.writes = 0
        self.fail_before = self.fail_after = self.read_failure = False

    def head(self):
        if self.read_failure:
            raise RuntimeError('secret error body must not escape')
        return self.current

    def read(self, commit, path):
        return self.commits[commit].get(path)

    def create_commit(self, head, files):
        new = str(uuid.uuid4())
        self.commits[new] = dict(self.commits[head])
        for path, body in files.items():
            if body is None:
                self.commits[new].pop(path, None)
            else:
                self.commits[new][path] = body
        self.parents[new] = head
        return new

    def advance(self, before, new):
        self.writes += 1
        if self.fail_before:
            raise RuntimeError('not written')
        assert self.current == before
        self.current = new
        if self.fail_after:
            raise RuntimeError('response lost')

    def descendant(self, ancestor, head):
        while head in self.parents:
            head = self.parents[head]
            if head == ancestor:
                return True
        return False


class PublicationTests(unittest.TestCase):
    def test_progression_rechecked_before_remote_mutation(self):
        request=self.queue.enqueue(str(uuid.uuid4()),7,'123',1,encode(sample()),'Alias','v1','CC0-1.0')
        remote=Remote()
        results=process(self.queue,remote,lambda build:'prerequisite missing at level 20')
        self.assertEqual(results[0]['state'],'conflict')
        self.assertEqual(remote.writes,0)
        self.assertEqual(self.queue.get(request)['state'],'conflict')
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.path = Path(self.temp.name) / 'private.sqlite'
        self.queue = Queue(self.path)
        self.installation = str(uuid.uuid4())
        self.remote = Remote()

    def tearDown(self):
        self.queue.connection.close()
        self.temp.cleanup()

    def enqueue(self, owner=7, revision=1, author='Public alias'):
        return self.queue.enqueue(self.installation, owner, '456', revision, encode(sample()), author, 'test-v1', 'CC0-1.0')

    def test_git_tree_deletion_uses_null_sha_without_upload(self):
        from publication import GitHub
        remote = GitHub(Remote.repository)
        calls = []
        def api(endpoint, payload=None, method=None):
            calls.append((endpoint, payload, method))
            if endpoint == 'git/blobs':
                return {'sha': 'blob'}
            if endpoint == 'git/commits/head':
                return {'tree': {'sha': 'base'}}
            if endpoint == 'git/trees':
                self.assertEqual(payload['base_tree'], 'base')
                self.assertEqual(payload['tree'], [
                    {'path': 'old', 'mode': '100644', 'type': 'blob', 'sha': None},
                    {'path': 'new', 'mode': '100644', 'type': 'blob', 'sha': 'blob'}])
                return {'sha': 'tree'}
            self.assertEqual(payload['parents'], ['head'])
            return {'sha': 'commit'}
        remote.api = api
        self.assertEqual(remote.create_commit('head', {'old': None, 'new': b'data'}), 'commit')
        self.assertEqual(sum(endpoint == 'git/blobs' for endpoint, _, _ in calls), 1)

    def test_idempotent_queue_and_owner_isolation(self):
        first = self.enqueue()
        self.assertEqual(first, self.enqueue())
        with self.assertRaises(ValueError):
            self.enqueue(author='Changed')
        other = self.enqueue(owner=8)
        get_id = lambda request: json.loads(self.queue.get(request)['record'])['id']
        self.assertNotEqual(get_id(first), get_id(other))
        self.assertEqual(get_id(first), get_id(self.enqueue(revision=2)))

    def test_update_rename_and_deleted_path_readback(self):
        from layout import build_path, catalog_path, entry
        first = self.enqueue()
        self.assertEqual(publish(self.queue, first, self.remote), 'published')
        old = json.loads(self.queue.get(first)['record'])
        request = self.queue.enqueue(self.installation, 7, '456', 2,
            encode(dict(sample(), Name='Renamed')), 'Public alias', 'test-v1', 'CC0-1.0')
        self.remote.fail_after = True
        self.assertEqual(publish(self.queue, request, self.remote), 'published')
        tree = self.remote.commits[self.remote.current]
        self.assertNotIn(build_path(old), tree)
        self.assertNotIn(catalog_path(entry(old)), tree)
        current = json.loads(tree['index.json'])
        self.assertEqual(current['schema'], 2)
        self.assertEqual(len(current['builds']), 1)
        self.assertEqual(current['builds'][0]['id'], old['id'])
        self.assertEqual(current['builds'][0]['revision'], 2)
        self.assertEqual(self.remote.commits[json.loads(self.queue.get(first)['intent'])['commit']][build_path(old)],
                         self.queue.get(first)['record'].encode())
        self.queue.finish(request, 'uncertain')
        self.remote.current = self.remote.create_commit(self.remote.current, {build_path(old): b'restored old path'})
        self.assertEqual(publish(self.queue, request, self.remote), 'conflict')
        self.assertEqual(self.remote.writes, 2)

    def test_same_name_and_alias_cross_installation_isolation(self):
        first = self.enqueue()
        second = self.queue.enqueue(str(uuid.uuid4()), 7, '456', 1, encode(sample()),
                                    'Public alias', 'test-v1', 'CC0-1.0')
        for request in (first, second):
            self.assertEqual(publish(self.queue, request, self.remote), 'published')
        rows = json.loads(self.remote.commits[self.remote.current]['index.json'])['builds']
        self.assertEqual(len({row['id'] for row in rows}), 2)
        self.assertEqual(len({row['path'] for row in rows}), 2)
        self.assertEqual(len({row['author'] for row in rows}), 1)

    def test_stale_source_and_public_revision_rejected(self):
        old = self.enqueue(revision=2)
        with self.assertRaises(ValueError):
            self.enqueue(revision=1)
        new = self.enqueue(revision=3)
        self.assertEqual(publish(self.queue, new, self.remote), 'published')
        with self.assertRaises(ValueError):
            publish(self.queue, old, self.remote)
        self.assertEqual(self.remote.writes, 1)

    def test_external_record_change_rejected_before_write(self):
        from library import canonical, make_record
        from layout import entry, build_path
        first = self.enqueue()
        publish(self.queue, first, self.remote)
        old = json.loads(self.queue.get(first)['record'])
        changed = make_record(dict(sample(), Name='External edit'), 'Public alias', 'test-v1', old['id'])
        self.remote.current = self.remote.create_commit(self.remote.current, {
            build_path(changed): canonical(changed),
            'index.json': canonical({'schema': 2, 'builds': [entry(changed)]})})
        with self.assertRaisesRegex(ValueError, 'owner publication history'):
            publish(self.queue, self.enqueue(revision=2), self.remote)
        self.assertEqual(self.remote.writes, 1)

    def test_uncertain_prior_blocks_replacement(self):
        first = self.enqueue()
        self.remote.fail_before = True
        publish(self.queue, first, self.remote)
        with self.assertRaisesRegex(ValueError, 'uncertain publication'):
            publish(self.queue, self.enqueue(revision=2), self.remote)
        self.assertEqual(self.remote.writes, 1)

    def test_legacy_index_requires_reviewed_migration(self):
        from library import canonical, make_record
        from layout import entry
        old = make_record(sample(), 'Alias', 'v1')
        item = entry(old)
        item['path'] = 'builds/' + old['id'] + '/1.json'
        self.remote.commits[self.remote.current]['index.json'] = canonical({'schema': 1, 'builds': [item]})
        with self.assertRaisesRegex(ValueError, 'migration required'):
            publish(self.queue, self.enqueue(), self.remote)
        self.assertEqual(self.remote.writes, 0)

    def test_lost_successful_response_and_no_resend(self):
        request = self.enqueue()
        self.remote.fail_after = True
        self.assertEqual(publish(self.queue, request, self.remote), 'published')
        self.queue.connection.close()
        self.queue = Queue(self.path)
        self.assertEqual(publish(self.queue, request, self.remote), 'published')
        self.assertEqual(self.remote.writes, 1)
        self.assertEqual(self.remote.commits[self.remote.current]['LICENSE'], b'preserve')
        public = json.loads(self.queue.get(request)['record'])
        self.assertNotIn('owner', public)
        self.assertNotIn('source', public)

    def test_failed_write_never_blindly_retried(self):
        request = self.enqueue()
        self.remote.fail_before = True
        self.assertEqual(publish(self.queue, request, self.remote), 'uncertain')
        self.remote.fail_before = False
        self.assertEqual(publish(self.queue, request, self.remote), 'uncertain')
        self.assertEqual(self.remote.writes, 1)

    def test_readback_loss_and_descendant_recovery(self):
        request = self.enqueue()
        advance = self.remote.advance
        def lose(before, new):
            advance(before, new)
            self.remote.read_failure = True
        self.remote.advance = lose
        self.assertEqual(publish(self.queue, request, self.remote), 'uncertain')
        self.assertNotIn('secret', self.queue.get(request)['error'])
        self.remote.read_failure = False
        new = self.remote.create_commit(self.remote.current, {'README.md': b'unrelated edit'})
        self.remote.current = new
        self.assertEqual(publish(self.queue, request, self.remote), 'published')
        self.assertEqual(self.remote.writes, 1)

    def test_conflicting_descendant_rejected(self):
        request = self.enqueue()
        self.remote.fail_before = True
        publish(self.queue, request, self.remote)
        intent = json.loads(self.queue.get(request)['intent'])
        new = self.remote.create_commit(intent['commit'], {'index.json': b'wrong'})
        self.remote.current = new
        self.assertEqual(publish(self.queue, request, self.remote), 'conflict')
        self.assertEqual(self.remote.writes, 1)


if __name__ == '__main__':
    unittest.main()
