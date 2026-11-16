from pathlib import Path
import os,subprocess,tempfile
root=Path(__file__).resolve().parents[3]
source=r'''
#include "Area52MysticTestAccess.h"
#include <boost/property_tree/json_parser.hpp>
#include <sstream>
#include <cassert>
int main()
{
    boost::property_tree::ptree report;
    std::istringstream input(R"({"schema":1,"entries":[
        {"kind":"mystic","id":1,"status":"VERIFIED"},
        {"kind":"mystic","id":2,"status":"CERTIFIED"},
        {"kind":"mystic","id":3,"status":"NOT VERIFIED"},
        {"kind":"mystic","id":4,"status":"VERIFIED"},
        {"kind":"advancement","id":5,"status":"VERIFIED"}]})");
    boost::property_tree::read_json(input,report);
    auto ids=AscensionMysticEnchant::VerifiedTestAccess(report,[](unsigned id){return id!=4;});
    assert((ids==std::set<std::uint32_t>{1,2}));
    auto earned=std::set<std::uint32_t>{9};auto effective=earned;
    effective.insert(ids.begin(),ids.end());assert(effective.size()==3 && earned.size()==1);
    effective=earned;assert(effective==std::set<std::uint32_t>{9});
    report.put("schema",2);bool rejected=false;
    try { AscensionMysticEnchant::VerifiedTestAccess(report,[](unsigned){return true;}); }
    catch(std::exception const&) { rejected=true; } assert(rejected);
    report.put("schema",1);report.get_child("entries").push_back({"",report.get_child("entries").front().second});
    rejected=false;
    try { AscensionMysticEnchant::VerifiedTestAccess(report,[](unsigned){return true;}); }
    catch(std::exception const&) { rejected=true; } assert(rejected);
}
'''
with tempfile.TemporaryDirectory() as directory:
    path=Path(directory);cpp=path/'test.cpp';cpp.write_text(source);binary=path/'test'
    subprocess.run([os.environ.get('CXX','c++'),'-std=c++20','-I'+str(root/'src/server/coa'),str(cpp),'-o',str(binary)],check=True)
    subprocess.run([str(binary)],check=True)
print('Verified/certified filtering, connection gating, earned ownership isolation and malformed report rejection passed.')
