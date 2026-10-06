import os
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[3]


def block(text, marker):
    start = text.index(marker)
    opening = text.index('{', start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]


MAIN = r'''
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <map>
#include <set>
using uint32=std::uint32_t; using uint64=std::uint64_t;
constexpr uint32 Enchant=81116,Weapon=983701,Effusion=983702,Knight=983715,Horror=983722,Anomaly=983723;
constexpr int SPEC_MASK_ALL=3,PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS=16,PROC_FLAG_DONE_MELEE_AUTO_ATTACK=4;
constexpr std::array<uint32,5> HolyWrath={2812,10318,27139,48816,48817};
struct SpellInfo { uint32 Id=983708;int StackAmount=20; };
struct Aura { int Stack=0; SpellInfo Info; int GetStackAmount(){return Stack;} SpellInfo* GetSpellInfo(){return &Info;} };
struct Player;
constexpr int POWER_MANA=0,UNIT_STATE_ISOLATED=1;using SpellEffIndex=int;
struct Unit {
 int Mana=200,Maximum=1000;bool Living=true,Isolated=false;
 virtual ~Unit()=default; virtual Player* ToPlayer(){return nullptr;}
 bool IsAlive(){return Living;}bool HasUnitState(int){return Isolated;}
 uint32 GetMaxPower(int){return Maximum;}uint32 GetPower(int){return Mana;}
 void EnergizeBySpell(Unit* target,uint32,uint32 gain,int){target->Mana=std::min(target->Maximum,target->Mana+int(gain));}
};
struct Player:Unit {
 bool Allowed=true,Alive=true,World=true; std::set<uint32> Known,Temporary;std::map<uint32,uint32> Replacements;
 std::map<uint32,Aura> Auras;std::map<uint32,int> Casts;
 Player* ToPlayer() override{return this;} bool IsAlive(){return Alive;} bool IsInWorld(){return World;}
 bool HasAura(uint32 id){return Auras.contains(id);} bool HasSpell(uint32 id){return Known.contains(id);}
 bool HasActiveSpell(uint32 id){return Known.contains(id);} int GetGUID(){return 1;}
 void learnSpell(uint32 id,bool){Known.insert(id);Temporary.insert(id);}
 void removeSpell(uint32 id,int,bool){if(Temporary.erase(id))Known.erase(id);}
 void SetTemporarySpellReplacement(uint32 id,uint32 v){Replacements[id]=v;}
 uint32 GetTemporarySpellReplacement(uint32 id){return Replacements[id];}
 void CastSpell(Unit*,uint32 id,bool){++Casts[id];if(id==Knight||id==Horror)Auras[id].Stack=1;if(id==Anomaly)++Auras[id].Stack;}
 Aura* GetAura(uint32 id,int){return HasAura(id)?&Auras[id]:nullptr;}
 void RemoveAurasDueToSpell(uint32 id,int){Auras.erase(id);}
};
bool Applies(Player const* p){return p&&p->Allowed;}
struct Spell {
 bool Triggered=false;std::map<uint32,uint64> Values;
 bool IsTriggered()const{return Triggered;} uint64 GetScriptValue(uint32 key)const{auto i=Values.find(key);return i==Values.end()?0:i->second;}
 void SetScriptValue(uint32 key,uint64 value){Values[key]=value;}
};
struct DamageInfo {uint32 Damage=10;uint32 GetDamage()const{return Damage;}};
struct ProcEventInfo {
 Player* Actor;Unit* Target;Spell* Cast;DamageInfo Damage;uint32 Type=16;
 Player* GetActor(){return Actor;} Unit* GetActionTarget(){return Target;} Spell const* GetProcSpell(){return Cast;}
 DamageInfo* GetDamageInfo(){return &Damage;}uint32 GetTypeMask(){return Type;}
};
uint32 Now=0;uint32 getMSTime(){return Now;}uint32 getMSTimeDiff(uint32 a,uint32 b){return b-a;}
SYNC
struct Handler {
 Player* Target;uint32 LastExplosion=0;bool Exploded=false;
 Player* GetTarget(){return Target;}void PreventDefaultAction(){}
 CHECK
 PROC
};
struct ManaHandler {
 Unit* Caster;Unit* Target;int Percent=20;SpellInfo Info;bool Prevented=false;
 Unit* GetCaster(){return Caster;}Unit* GetHitUnit(){return Target;}
 SpellInfo* GetSpellInfo(){return &Info;}int GetEffectValue(){return Percent;}
 void PreventHitDefaultEffect(int){Prevented=true;}
 RESTORE
};
int main(){
 Unit mana;ManaHandler restore{&mana,&mana};restore.Restore(0);assert(mana.Mana==360&&restore.Prevented);
 mana.Mana=200;restore.Percent=8;restore.Restore(0);assert(mana.Mana==264);
 mana.Mana=1000;restore.Restore(0);assert(mana.Mana==1000);
 mana.Mana=0;restore.Percent=20;restore.Restore(0);assert(mana.Mana==200);
 mana.Isolated=true;restore.Restore(0);assert(mana.Mana==200);

 Player p;Unit enemy; p.Auras[Enchant].Stack=1;p.Known.insert(2812);
 Synchronize(&p);assert(p.HasSpell(Effusion)&&p.HasSpell(983703)&&p.Replacements[2812]==983703);
 Synchronize(&p);assert(p.Temporary.size()==2);
 p.Known.erase(2812);Synchronize(&p);assert(!p.HasSpell(983703)&&!p.Replacements[2812]);
 p.Known.insert(2812);Synchronize(&p);Synchronize(&p,true);
 assert(!p.HasSpell(Effusion)&&!p.HasSpell(983703)&&p.HasSpell(2812));
 p.Known.insert(Effusion);Synchronize(&p,true);assert(p.HasSpell(Effusion));
 Handler h{&p};Spell spell;ProcEventInfo e{&p,&enemy,&spell};
 h.Proc(e);assert(p.Casts.empty());p.Auras[Weapon].Stack=1;
 h.Proc(e);assert(p.Casts[983711]==1&&p.Casts[983712]==1&&p.Auras[Anomaly].Stack==1);
 h.Proc(e);assert(p.Casts[983712]==1);
 spell.Values.clear();Now=2999;h.Proc(e);assert(p.Casts[983711]==1&&p.Casts[983712]==2);
 spell.Values.clear();Now=3000;h.Proc(e);assert(p.Casts[983711]==2);
 for(int i=3;i<20;++i){spell.Values.clear();h.Proc(e);}
 assert(!p.HasAura(Anomaly)&&p.HasAura(Horror)&&p.Casts[Horror]==1);
 e.Cast=nullptr;e.Type=4;int before=p.Casts[983718];h.Proc(e);assert(p.Casts[983718]==before+1);
 assert(p.Casts[983712]==20);
 e.Cast=&spell;e.Type=16;spell.Values.clear();spell.Triggered=true;assert(!h.Check(e));
 spell.Triggered=false;e.Damage.Damage=0;assert(!h.Check(e));e.Damage.Damage=10;
 p.Allowed=false;assert(!h.Check(e));p.Allowed=true;p.Alive=false;assert(!h.Check(e));
 p.Alive=true;e.Actor=nullptr;assert(!h.Check(e));
}
'''


def main():
    source = (ROOT / 'src/server/coa/AscensionEldritchKnight.cpp').read_text()
    code = MAIN.replace('SYNC', block(source, 'void Synchronize('))
    code = code.replace(' CHECK\n', block(source, 'bool Check(ProcEventInfo&'))
    code = code.replace(' PROC\n', block(source, 'void Proc(AuraEffect const*'))
    code = code.replace('void Proc(AuraEffect const*, ProcEventInfo& event)', 'void Proc(ProcEventInfo& event)')
    code = code.replace(' RESTORE\n', block(source, 'void Restore(SpellEffIndex'))
    compiler = shutil.which(os.environ.get('CXX', 'c++'))
    assert compiler, 'C++20 compiler required'
    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory)
        (path / 'test.cpp').write_text(code)
        subprocess.run([compiler, '-std=c++20', str(path / 'test.cpp'), '-o', str(path / 'test')], check=True)
        subprocess.run([str(path / 'test')], check=True)
    print('PASS: temporary spell ownership, rank removal, proc isolation, multi-hit deduplication, 3s cooldown, 20-stack Horror')


if __name__ == '__main__':
    main()
