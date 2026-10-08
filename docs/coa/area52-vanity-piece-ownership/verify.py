from pathlib import Path
import json,struct,sys
root=Path(__file__).resolve().parents[3];p=root.parent/'Area52_Vanity_Piece_Ownership'
sys.path.insert(0,str(root.parents[1]/'work/github-upload/tools'))
from lib.mpq import MPQArchive
client=Path('D:/DML WOTLK Client Side/Ascension A52 Free Pick/ascension-live/ascension-live/Data/area-52/patch-D.MPQ')
catalog=json.loads((p/'catalog.json').read_text())
def records(data):
    _,n,fields,size,_=struct.unpack_from('<4s4I',data)
    return [struct.unpack_from('<'+'I'*(size//4),data,20+i*size) for i in range(n)]
with MPQArchive(client) as old,MPQArchive(p/'patch-D.MPQ') as new:
    key='DBFilesClient\\VanityCollection.dbc';before=records(old.read_file(key));after=records(new.read_file(key))
    assert after[:len(before)]==before
    byitem={r[1]:r for r in after}
    missing=set(catalog['equipment'])-{r[1] for r in before}
    assert len(missing)==454
    assert set(catalog['equipment'])<=byitem.keys()
    assert len({r[0] for r in after})==len(after)
    for item in missing:
        row=byitem[item]
        assert row[18]==item and not any(row[19:42]) and not any(row[12:18]) and not row[76]
    for name in old.read_file('(listfile)').decode().splitlines():
        if name and name.lower() not in ('(listfile)','(attributes)','(signature)') and name!=key:
            assert old.read_file(name)==new.read_file(name),name
source=(root/'src/server/coa/AscensionCompat.cpp').read_text()
redeem=source[source.index('    bool RedeemVanityBundle('):source.index('  void OnItemObtained(')]
assert 'for (uint32 entry : rewards)\n            trans->Append' in redeem
assert 'std::vector<uint32> ownership = rewards;' in redeem
assert 'packet << entry;' in redeem
load=source[source.index('  std::shared_ptr<PlayerCollectionState> LoadCollectionState('):source.index('  std::shared_ptr<PlayerCollectionState> LoginState(')]
assert 'AscensionFreePick::Applies(player)' in load
assert 'state->OwnedVanityItems.contains(bundle)' in load
assert 'state->OwnedVanityItems.insert(entry).second' in load
assert 'account_appearance_collection' not in load
print('PASS: 454 missing entries reproduced and repaired, all 522 pieces mapped; existing records/assets preserved; ownership/backfill hooks present. No gameplay claim.')
