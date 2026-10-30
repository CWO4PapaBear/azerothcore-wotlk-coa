from pathlib import Path
import json,subprocess,sys,tempfile,os,hashlib
root=Path.cwd()
sys.path.insert(0,str(root/'apps/coa-tests'))
from coa_talent_catalog import STUBS
data=json.loads(Path('/results/readiness-live-catalog.json').read_text())
audit=json.loads(Path('/results/readiness-live-audit.json').read_text())
connected={int(r['spell']) for r in audit['catalog'] if r['connected']=='true'}
spells={int(r['id']):r for r in audit['spells']}
repair=os.environ.get('AREA52_EXPECT_READINESS_REPAIR')=='1'
if repair:
    evidence=json.loads(Path('/results/readiness-connection-evidence.json').read_text())
    assert evidence['status']=='passed' and evidence['enchant']==951554
    for name,digest in evidence['sources'].items():assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest
    connected.add(951554)
calls=[]
groups={};rules={};ranks={}
for line in Path('/results/readiness-spell-groups.tsv').read_text().splitlines():
    kind,a,b=line.split('\t');a,b=int(a),int(b)
    if kind=='member':groups.setdefault(a,[]).append(b)
    elif kind=='rule':rules[a]=b
    else:ranks[a]=b
def members(group,seen=()):
    assert group not in seen,'Recursive spell group cycle'
    result=set()
    for spell in groups.get(group,[]):
        result.update(members(-spell,(*seen,group)) if spell<0 else [spell])
    return result
restricted={g:members(g) for g,rule in rules.items() if rule}
for spec,stat in [('mknight',3),('naturalist',4),('hoplite',0)]:
    picks=[r for r in data['playerbots_legendary_pick'] if r['spec']==spec]
    enchants=sorted([r for r in data['playerbots_legendary_enchant'] if r['spec']==spec],key=lambda r:int(r['slot']))
    selected={int(r['spell']) for r in enchants if int(r['spell'])}
    for group,entries in restricted.items():
        assert len({spell for spell in selected if ranks.get(spell,spell) in entries})<2,(spec,group)
    calls.append('check("'+spec+'",'+str(stat)+',{'+','.join('{'+r['spell']+','+r['level']+'}' for r in picks)+'},{'+','.join(r['spell'] for r in enchants)+'});')
source=r'''
#include "AscensionFreePick.h"
#include "Area52BuildSelection.h"
#include "AscensionMysticEnchantCatalog.h"
#include "DBCStores.h"
#include <iostream>
int main(){DbcDirectory="/data/dbc/";AscensionFreePick::Catalog abilities;AscensionMysticEnchant::Catalog enchants;
if(!AscensionFreePick::LoadCatalog(abilities)||!AscensionMysticEnchant::LoadCatalog(enchants,"/data/dbc/MysticEnchant.dbc"))return 2;
unsigned failures=0;std::set<unsigned> known;std::map<unsigned,unsigned> limits;
'''
for spell in sorted(connected):
    source+='known.insert('+str(spell)+');limits['+str(spell)+']='+str(max(1,int(spells[spell]['stack_amount'])))+';\n'
source+=r'''
auto check=[&](char const* spec,unsigned stat,std::vector<std::pair<unsigned,unsigned>> picks,AscensionMysticEnchant::Slots slots){
Area52Build::Entry plan;plan.PrimaryStat=stat;
unsigned worldforged=0;for(auto id:slots)if(id){auto const& e=enchants.BySpell.at(id);worldforged+=e.Worldforged&&e.Quality==3;}if(worldforged>3)++failures;
for(auto [spell,level]:picks){bool found=false;for(auto const& [id,e]:abilities.Entries)if(std::find(e.Spells.begin(),e.Spells.end(),spell)!=e.Spells.end())found=true;if(found){Area52Build::Spell p;p.Id=spell;p.Level=level;plan.Spells.push_back(p);}}
for(unsigned level=15;level<=80;++level){std::vector<AscensionFreePick::KnownEntry> wanted;auto result=Area52Build::Select(abilities,plan,level,wanted,{});
if(!result||!AscensionMysticEnchant::ValidateSlots(enchants,slots,level,known,limits))++failures;
std::cout<<spec<<" level "<<level<<" picks "<<bool(result)<<" slots "<<AscensionMysticEnchant::ValidateSlots(enchants,slots,level,known,limits)<<"\n";
for(auto id:slots){if(!id)continue;auto const& e=enchants.BySpell.at(id);
bool met=true;for(bool talent:{false,true}){auto const& required=talent?e.TalentInvestment:e.AbilityInvestment;bool yes=std::all_of(required.begin(),required.end(),[](auto r){return !r;});
for(unsigned i=0;i<3;++i){unsigned investment=0;for(auto const& p:wanted){auto const& a=abilities.Entries.at(p.EntryId);if(e.Classes[i]==1||(a.Class==e.Classes[i]&&(e.Tabs[i]==1||a.Tab==e.Tabs[i])))investment+=(talent?a.TE:a.AE)*p.Rank;}yes|=required[i]&&investment>=required[i];}met &=yes;}
if(e.Level>level||!known.contains(id)||!met){++failures;std::cout<<"  blocked "<<id<<" minlevel="<<e.Level<<" connected="<<known.contains(id)<<" investment="<<met<<"\n";}
}}
};
'''+''.join(calls)+('\nreturn failures?1:0;\n' if repair else '\n')+'}\n'
with tempfile.TemporaryDirectory() as tmp:
    out=Path(tmp)
    for name,content in STUBS.items():(out/name).write_text(content)
    (out/'check.cpp').write_text(source)
    includes=[out,root/'src/server/coa',root/'src/server/shared/DataStores',root/'src/common']
    sources=[out/'check.cpp',root/'src/server/coa/AscensionFreePickRules.cpp',root/'src/server/coa/AscensionMysticEnchantCatalog.cpp',root/'src/server/shared/DataStores/ClientDBC.cpp']
    subprocess.run(['clang++','-std=c++20',*['-I'+str(p) for p in includes],*map(str,sources),'-o',str(out/'check')],check=True)
    result=subprocess.run([str(out/'check')],check=True,text=True,capture_output=True)
    Path('/results/readiness-catalog-qualified.txt' if repair else '/results/readiness-catalog-diagnostic.txt').write_text(result.stdout)
    print(result.stdout)
