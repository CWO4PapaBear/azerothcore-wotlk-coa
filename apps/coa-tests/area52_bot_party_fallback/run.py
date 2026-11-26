from pathlib import Path
import subprocess,tempfile,os

module=Path(os.environ.get('AREA52_BOT_MODULE',str(Path(__file__).resolve().parents[3]/'modules/mod-playerbots')))
source=(module/'src/Ai/Coa/CoaGenericContext.cpp').read_text()
methods=source[source.index('static Player* CoaMainTank('):source.index('Unit* CoaGroundHealAction::GetTarget()')]
stub=r'''
#include <cassert>
#include <vector>
struct Unit {
 bool alive=true, world=true, hostile=true, combat=false; int map=1;
 Unit* victim=nullptr; Unit* GetVictim(){return victim;}
 bool IsAlive(){return alive;} bool IsInWorld(){return world;}
 int GetMap(){return map;} bool IsInCombat(){return combat;}
};
struct Player:Unit {
 bool tank=false,main=false,isBot=false; float health=100;
 Unit* selected=nullptr;
 Unit* GetSelectedUnit(){return selected;}
 float GetHealthPct(){return health;}
 bool IsValidAttackTarget(Unit* u){return u->hostile;}
};
struct PlayerbotAI {
 static bool IsTank(Player* p){return p->tank;}
 static bool IsMainTank(Player* p){return p->main;}
};
std::vector<Player*> party;
#define GET_PLAYERBOT_AI(p) ((p)->isBot ? (p) : nullptr)
std::vector<Player*> CoaGroupInReach(Player*){return party;}
bool CoaPullsAheadOfTank(Player*,Unit*){return false;}
'''
tests=r'''
int main(){
 Player bot,human,tank,healer; Unit mob,idle;
 party={&bot,&human}; human.health=40; bot.health=80;
 assert(CoaSupportTarget(&bot)==&human);
 human.selected=&idle; assert(!CoaAssistTarget(&bot));
 human.combat=true; mob.combat=true; human.victim=&mob;
 assert(CoaAssistTarget(&bot)==&mob);
 human.victim=nullptr; human.selected=&mob; assert(!CoaAssistTarget(&bot));
 mob.victim=&human; assert(CoaAssistTarget(&bot)==&mob);
 mob.victim=nullptr; human.victim=&mob;
 healer.isBot=true;healer.combat=true;idle.combat=true;healer.victim=&idle;
 party={&bot,&healer,&human};assert(CoaAssistTarget(&bot)==&mob);party={&bot,&human};
 mob.alive=false; assert(!CoaAssistTarget(&bot)); mob.alive=true;
 mob.hostile=false; assert(!CoaAssistTarget(&bot)); mob.hostile=true;
 mob.map=2; assert(!CoaAssistTarget(&bot)); mob.map=1;
 mob.world=false; assert(!CoaAssistTarget(&bot)); mob.world=true;
 tank.tank=true; tank.victim=&idle; party.push_back(&tank);
 assert(CoaSupportTarget(&bot)==&tank);
 assert(CoaAssistTarget(&bot)==&idle);
 tank.victim=nullptr; assert(CoaAssistTarget(&bot)==&mob);
 party={&bot}; bot.victim=&mob; bot.combat=true;
 assert(CoaAssistTarget(&bot)==&mob); assert(CoaSupportTarget(&bot)==&bot);
 party={}; assert(!CoaSupportTarget(&bot));
}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'check.cpp').write_text(stub+methods+tests)
 subprocess.run(['g++','-std=c++20','-Wall','-Wextra',str(p/'check.cpp'),'-o',str(p/'check')],check=True)
 subprocess.run([str(p/'check')],check=True)
print('PASS production support/assist selection: tank priority, no-tank combat, idle refusal, hostile/map/alive/world guards, solo support')
print('Scope: group reach and pull gate are adapters; full runtime and remaining rotation consumers still require verification')
