from pathlib import Path
import json,subprocess,tempfile,os
src=Path(os.environ['THRUST_EXTENSION_SOURCE']).read_text()
def extract(start,end):return src[src.index(start):src.index(end,src.index(start))]
fixture='''
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <cassert>
#define __cdecl
constexpr uint32_t kTableSize=0x2AF60;
int32_t g_flat[kTableSize]{},g_pct[kTableSize]{};
using FamilyMap=std::unordered_map<uint32_t,std::unordered_map<uint8_t,int32_t>>;
FamilyMap g_spellFlat,g_spellPct;
uint32_t U32(const void* p,uint32_t o){uint32_t v;memcpy(&v,(const uint8_t*)p+o,4);return v;}
struct CDataStore {std::vector<uint8_t> m_buffer;size_t m_read=0;};
void AddSpellModifier(uint32_t,int32_t,uint32_t,bool){assert(false && "Unexpected per-spell packet");}
'''+extract('    int32_t SpellModifier(', '    // FUN_10328ac0')+extract('    uint8_t ReadU8(', '    typedef uint32_t(__cdecl* C2_t)')
r=json.loads(Path(os.environ['THRUST_PACKET_RESULT']).read_text());sc=json.loads(Path(os.environ['THRUST_PACKET_SCENARIO']).read_text());steps=sc['steps'];seen={614:0,615:0};checks=[];phase=0
fixture+='\nint main(){ alignas(4) uint8_t rec[0x2B0]{}; uint32_t family=8,flags=8388610;memcpy(rec+0x240,&family,4);memcpy(rec+0x244,&flags,4);\n'
for step in r['steps']:
 if 'packet_hex' not in step:continue
 op=steps[int(step['index'])]['opcode'];packets=step['packet_hex'];packets=[packets] if isinstance(packets,str) else packets
 for h in packets[seen[op]:]:
  b=bytes.fromhex(h);assert len(b)==11 and b[0]==0,(op,h)
  fixture+=' { CDataStore p{{'+','.join(str(x) for x in b)+'},0};SetSpellModifier(nullptr,'+str(op)+',0,&p);assert(p.m_read==11);}\n'
 seen[op]=len(packets)
 if op==615:
  expected=[0,-3,-5,0][phase];phase+=1
  fixture+='{int32_t flat=999,pct=999;auto modified=SpellModifierSum(rec,14,&flat,&pct);std::cout<<"phase '+str(phase)+' flat="<<flat<<" pct="<<pct<<" modified="<<(modified&255)<<"\\n";assert(flat=='+str(expected)+');assert(pct=='+str(0 if expected==0 else 100)+');}\n'
fixture+='}\n'
with tempfile.TemporaryDirectory() as t:
 p=Path(t);(p/'replay.cpp').write_text(fixture);subprocess.run(['c++','-std=c++20',str(p/'replay.cpp'),'-o',str(p/'replay')],check=True);subprocess.run([str(p/'replay')],check=True)
print('Captured packet replay passed against extracted extension reader and accumulator; native binary hooks not exercised.')
