import argparse
import json
from pathlib import Path
import sys

from status import build, digest, patch_spell_dbc


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--audit', required=True)
    parser.add_argument('--evidence', required=True)
    parser.add_argument('--source', required=True)
    parser.add_argument('--expected-sha256', required=True)
    parser.add_argument('--mpq-tools', required=True)
    parser.add_argument('--output', required=True)
    args = parser.parse_args()
    source = Path(args.source).resolve()
    output = Path(args.output).resolve()
    if source == output or output.exists():
        raise ValueError('Choose a new staging path; this tool does not install or overwrite archives')
    if digest(source) != args.expected_sha256:
        raise ValueError('Installed client archive changed; review its effective data again')
    report = build(args.audit, args.evidence)
    sys.path.insert(0, str(Path(args.mpq_tools).resolve()))
    from lib.mpq import MPQArchive, write_archive
    with MPQArchive(source) as archive:
        files = {name: archive.read_file(name) for name in archive.read_file('(listfile)').decode().splitlines()
                 if name and name.lower() not in ('(listfile)', '(attributes)', '(signature)')}
    member = 'DBFilesClient\\Spell.dbc'
    files[member], changed = patch_spell_dbc(files[member], report)
    output.parent.mkdir(parents=True, exist_ok=True)
    write_archive(str(output), files)
    with MPQArchive(output) as archive:
        for name, content in files.items():
            if archive.read_file(name) != content:
                raise ValueError('Archive roundtrip mismatch: ' + name)
    result = {'installed': False, 'published': False, 'source_sha256': digest(source),
              'candidate_sha256': digest(output), 'effective_sha256': report['effective_sha256'],
              'changed_descriptions': len(changed), 'counts': report['counts']}
    output.with_suffix('.verification.json').write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(result))


if __name__ == '__main__':
    main()
