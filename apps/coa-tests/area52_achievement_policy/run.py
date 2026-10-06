import os
from pathlib import Path
import subprocess
import tempfile
root = Path(__file__).resolve().parents[3]
code = r"""
#include "AscensionArea52AchievementPolicy.h"
#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
using namespace Area52Achievements;
int main(int argc,char** argv)
{
    assert(Enabled(true,"hero","live",0,true));
    assert(Enabled(true,"hero","live",0x400,true));
    assert(!Enabled(true,"coa","live",0,true));
    assert(!Enabled(true,"hero","seasonal",0,true));
    assert(!Enabled(true,"hero","live",0x40,true));
    assert(!Enabled(true,"hero","live",0x80000000,true));
    assert(!Enabled(false,"hero","live",0,true));
    assert(!Enabled(true,"hero","live",0,false));
    std::map<std::uint32_t,std::uint32_t> cats={{16133,16135},{99991,16133},{17016,17013}};
    Record a{7399,81,"Realm First! Explore Karazhan",{1,2},{{43,1062,1}},true,false,false};
    Record b=a;b.Id=11399;
    auto rejected=Build({b,a},cats);assert(rejected==std::set<std::uint32_t>{11399});
    b.Criteria={{43,407,1}};assert(Build({a,b},cats).empty());b=a;b.Id=11399;
    b.Requirements={1,3};assert(Build({a,b},cats).empty());b=a;b.Id=11399;
    b.HasConditions=true;assert(Build({a,b},cats).empty());b.HasConditions=false;b.HasRewards=true;
    assert(Build({a,b},cats).empty());b.HasRewards=false;b.RealmFirst=false;assert(Build({a,b},cats).empty());
    for(auto name:{"Realm First! Nightmare Mode: Obtained a Mount","WildCard Level 10","Realm First! Draft Level 10","Seasonal: Reached Level 10","Felforged Level 30"})
    {Record r=a;r.Name=name;assert(OtherMode(r,cats));}
    for(auto name:{"Ironman","'Tis the Season","Realm First! Explore Nightmare Vale in Tirisfal Glades","Realm First! Crafted Resolute Cape","Level 10"})
    {Record r=a;r.Name=name;assert(!OtherMode(r,cats));}
    b=a;b.Category=99991;assert(OtherMode(b,cats));
    cats[222]=223;cats[223]=222;b.Category=222;assert(!OtherMode(b,cats));
    assert(Build({},cats).empty());
    if(argc==3)
    {
        std::ifstream in(argv[1]);std::size_t n;in>>n;cats.clear();
        for(std::size_t i=0;i<n;++i){std::uint32_t id,parent;in>>id>>parent;cats[id]=parent;}
        in>>n;std::vector<Record> records;
        for(std::size_t i=0;i<n;++i)
        {
            Record r;std::size_t count;in>>r.Id>>r.Category>>r.RealmFirst>>r.HasConditions>>r.HasRewards>>std::quoted(r.Name)>>count;
            for(std::size_t j=0;j<count;++j){std::uint32_t x;in>>x;r.Requirements.push_back(x);}
            in>>count;
            for(std::size_t j=0;j<count;++j){std::size_t size;in>>size;std::vector<std::uint32_t> row;for(std::size_t k=0;k<size;++k){std::uint32_t x;in>>x;row.push_back(x);}r.Criteria.push_back(row);}
            records.push_back(r);
        }
        assert(in.good());std::ofstream out(argv[2]);for(auto id:Build(records,cats))out<<id<<'\n';
    }
}
"""
with tempfile.TemporaryDirectory() as directory:
    cpp = Path(directory) / 'policy.cpp'
    binary = Path(directory) / 'policy'
    cpp.write_text(code)
    subprocess.run([os.environ.get('CXX', 'c++'), '-std=c++20', '-I', str(root / 'src/server/coa'), str(cpp), '-o', str(binary)], check=True)
    args = [str(binary)]
    if os.environ.get('AREA52_ACHIEVEMENT_FIXTURE'):
        args.extend([os.environ['AREA52_ACHIEVEMENT_FIXTURE'], os.environ['AREA52_ACHIEVEMENT_RESULT']])
    subprocess.run(args, check=True)
print('PASS: mode gates, equivalent realm-first families, distinct criteria, SQL conditions/rewards, category ancestry and legitimate similar names')
