from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[3]
common = r'''
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <array>
#include <type_traits>
using uint32=std::uint32_t;
using uint8=std::uint8_t;
struct WorldPacket {
 unsigned opcode;std::vector<uint8> bytes;
 WorldPacket(unsigned op,unsigned):opcode(op){}
 WorldPacket& operator<<(uint32 v){for(unsigned i=0;i<4;++i)bytes.push_back(v>>(i*8));return *this;}
};
struct WorldSession{bool bot=false;std::vector<WorldPacket> sent;bool IsBot(){return bot;}
 void SendPacket(WorldPacket const* p){sent.push_back(*p);}};
struct Config{bool enabled=true;std::string model="hero", realm="live";unsigned mode=0;
 template<class T>T GetOption(char const* key,T fallback){
 std::string k=key;
 if constexpr(std::is_same_v<T,bool>)return enabled;
 if constexpr(std::is_same_v<T,std::string>)return k=="CoA.ClassModel"?model:realm;
 if constexpr(std::is_same_v<T,unsigned>)return mode;
 return fallback;}};
inline Config config;inline Config* sConfigMgr=&config;
struct Field{unsigned number=0;std::string text;
 template<class T>T Get()const{if constexpr(std::is_same_v<T,std::string>)return text;else return number;}};
struct ResultSet{std::vector<std::array<Field,2>> rows;unsigned index=0;
 Field const* Fetch(){return rows[index].data();}bool NextRow(){return ++index<rows.size();}};
struct DB{std::vector<std::array<Field,2>> rows;unsigned queries=0;std::string query;
 std::shared_ptr<ResultSet> Query(char const* q){++queries;query=q;if(rows.empty())return {};auto r=std::make_shared<ResultSet>();r->rows=rows;return r;}};
inline DB CharacterDatabase;
inline unsigned errors=0;
#define LOG_ERROR(...) (++errors)
'''
test = r'''
#include "Area52CreationPreview.cpp"
#include <cassert>
#include <iostream>
std::string wire(std::string description){Area52Build::Entry e;e.Category=8;e.Description=description;
 std::string hex;for(auto b:Area52Build::Encode(e)){hex+="0123456789abcdef"[b>>4];hex+="0123456789abcdef"[b&15];}return hex;}
unsigned field(WorldPacket const& p,unsigned index){unsigned v=0;for(unsigned b=0;b<4;++b)v|=unsigned(p.bytes[index*4+b])<<(8*b);return v;}
void set(unsigned id,std::string description){CharacterDatabase.rows={{{Field{id,{}},Field{0,wire(description)}}}};}
int main(){WorldSession session;
 config.enabled=false;SendArea52CreationPreview(&session);assert(session.sent.empty()&&CharacterDatabase.queries==0);
 config.enabled=true;config.model="coa";SendArea52CreationPreview(&session);assert(CharacterDatabase.queries==0);
 config.model="hero";session.bot=true;SendArea52CreationPreview(&session);assert(CharacterDatabase.queries==0);
 session.bot=false;config.mode=1;SendArea52CreationPreview(&session);assert(CharacterDatabase.queries==0);config.mode=0;
 for(auto stance:{26u,48u,0u,4294967295u}){
 set(86,"###AREA52_FEATURED_STANCE\n1:"+std::to_string(stance)+"\n###AREA52_FEATURED_WEAPONS\n1:0");
 session.sent.clear();SendArea52CreationPreview(&session);assert(session.sent.size()==1120);
 unsigned matched=0;
 for(unsigned i=0;i<session.sent.size();++i){auto const& p=session.sent[i];assert(p.opcode==0x6e3&&p.bytes.size()==112);
 auto expected=Area52Build::CreationPreviewDefaults[i];if(expected[1]==86){++matched;if(stance!=4294967295u)expected[12]=stance;}
 for(unsigned f=0;f<28;++f)assert(field(p,f)==expected[f]);}assert(matched==20);
 }
 assert(CharacterDatabase.query.find("r.revision=a.revision")!=std::string::npos);
 assert(CharacterDatabase.query.find("area52_build_featured")!=std::string::npos);
 set(86,"###AREA52_FEATURED_STANCE\n1:99");session.sent.clear();SendArea52CreationPreview(&session);assert(session.sent.empty()&&errors);
 CharacterDatabase.rows.clear();SendArea52CreationPreview(&session);assert(session.sent.size()==1120);
 for(unsigned i=0;i<session.sent.size();++i)for(unsigned f=0;f<28;++f)assert(field(session.sent[i],f)==Area52Build::CreationPreviewDefaults[i][f]);
 std::cout<<"PASS native sender wire, cold disabled/enabled lifecycle, realm/bot gates, repeated saved selections, default restoration and malformed no-partial-send\n";}
'''
with tempfile.TemporaryDirectory() as directory:
    p=Path(directory)
    (p/'stub.h').write_text(common)
    for name in ('Config.h','DatabaseEnv.h','QueryResult.h','Field.h','Log.h','WorldPacket.h','WorldSession.h'):
        (p/name).write_text('#include "stub.h"\n')
    (p/'test.cpp').write_text(test)
    subprocess.run(['g++','-std=c++20','-O2','-Wall','-Wextra','-Werror','-I',str(p),'-I',str(root/'src/server/coa'),str(p/'test.cpp'),'-o',str(p/'test')],check=True)
    subprocess.run([str(p/'test')],check=True)
