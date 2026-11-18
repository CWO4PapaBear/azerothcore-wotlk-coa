import hashlib
import json
from pathlib import Path
import subprocess

SOURCE_COMMIT = 'c34210b657d71081a11cffef815a219a72acdc31'
MANIFEST = 'docs/coa/baselines/area52-20261118/inherited-sql.json'


def preserved_paths(root):
    root = Path(root)
    path = root / MANIFEST
    if not path.exists():
        return set()
    data = json.loads(path.read_text())
    if data.get('source_commit') != SOURCE_COMMIT or not isinstance(data.get('files'), dict):
        raise ValueError('Invalid inherited SQL provenance')
    result = subprocess.run(['git', 'ls-tree', '-rz', SOURCE_COMMIT], cwd=root,
                            check=True, capture_output=True).stdout
    tree = {}
    for entry in result.split(b'\0'):
        if entry:
            meta, name = entry.split(b'\t', 1)
            tree[name.decode()] = meta.split()[2].decode()
    for name, blob in data['files'].items():
        relative = Path(name)
        if relative.is_absolute() or '..' in relative.parts or not name.endswith('.sql'):
            raise ValueError('Invalid inherited SQL path')
        item = root / relative
        if tree.get(name) != blob or item.is_symlink() or not item.is_file():
            raise ValueError('Inherited SQL provenance or file missing: ' + name)
        raw = item.read_bytes()
        actual = hashlib.sha1(b'blob ' + str(len(raw)).encode() + b'\0' + raw).hexdigest()
        if actual != blob:
            raise ValueError('Inherited SQL changed: ' + name)
    return set(data['files'])
