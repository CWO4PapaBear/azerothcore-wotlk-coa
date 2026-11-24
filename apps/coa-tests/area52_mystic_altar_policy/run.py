from pathlib import Path
import re, subprocess, tempfile, resource
ROOT = Path(__file__).resolve().parents[3]
def extract(s, signature):
    a=s.index(signature); b=s.index('{',a)+1; depth=1
    while depth:
        depth+=(s[b]=='{')-(s[b]=='}'); b+=1
    return s[a:b]
def main():
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
    freepick=(ROOT/'src/server/coa/AscensionFreepick.cpp').read_text()
    realm=(ROOT/'src/server/coa/Area52MysticRealm.h').read_text()
    compat=(ROOT/'src/server/coa/AscensionCompat.cpp').read_text()
    guardian=(ROOT/'modules/mod-worldforged-upgrades/src/WorldforgedGuardian.cpp').read_text()
    loader=(ROOT/'src/server/coa/CoAScriptLoader.cpp').read_text()
    upstream=(ROOT/'src/server/coa/RealmMysticEnchant.cpp').read_text()
    assert 'if (!Area52MysticRealm::UsesFreePickEngine())\n        AddAscensionFreepickScripts();' in loader
    assert 'if (Area52MysticRealm::UsesFreePickEngine())\n        return;' in extract(upstream,'void AddAscensionMysticEnchantScripts()')
    code=Path(__file__).with_name('harness.cpp').read_text()
    code=code.replace('ENGINE',extract(realm,'inline bool UsesFreePickEngine()'))
    policy=extract(freepick,'bool RealmOffersMysticAltars()')
    code=code.replace('POLICY',policy).replace('VENDOR',extract(guardian,'bool SellsAltars()'))
    code=code.replace('ITEMS',re.search(r'static constexpr std::array<uint32, 17> MysticAltarVanityItems = [^;]+;',compat)[0])
    code=code.replace('WITHHELD',extract(compat,'bool IsWithheldVanityItem(uint32 itemId) const'))
    for name in ['void DeliverVanityItem(', 'bool GrantFullVanityCollection(', 'void PurchaseVanityItemWithBazaarTokens(']:
        assert 'IsWithheldVanityItem' in extract(compat,name)
    with tempfile.TemporaryDirectory() as tmp:
        p=Path(tmp); cpp=p/'test.cpp'; exe=p/'test'
        cpp.write_text(code); subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True);subprocess.run([str(exe)],check=True)
        cpp.write_text(code.replace(policy,'bool RealmOffersMysticAltars(){return MysticAltars;}'))
        subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True)
        r=subprocess.run([str(exe)],capture_output=True)
        assert r.returncode!=0
    print('PASS native realm policy, all 17 altar IDs, vendor gate, Area 52 before upstream startup, upstream Free-Pick/Reborn, CoA/Wildcard exclusions, unrelated-item protection; original policy fails Area 52 startup case. Registration guards preserved.')
if __name__=='__main__':
    main()
