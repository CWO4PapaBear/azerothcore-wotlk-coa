from pathlib import Path
import subprocess,tempfile,struct
r=Path(__file__).resolve().parents[3]
def block(s,m):
 a=s.index(m);i=s.index('{',a)+1;n=1
 while n:n+=(s[i]=='{')-(s[i]=='}');i+=1
 return s[a:i]
sp=(r/'src/server/game/Spells/SpellEffects.cpp').read_text();au=(r/'src/server/game/Spells/Auras/SpellAuraEffects.cpp').read_text();up=(r/'src/server/game/Entities/Player/PlayerUpdates.cpp').read_text();unit=(r/'src/server/game/Entities/Unit/Unit.h').read_text();stat=(r/'src/server/game/Entities/Unit/StatSystem.cpp').read_text()
a=stat.index('    uint32 mask = 0;',stat.index('// Update ratings in exist SPELL_AURA_MOD_RATING_FROM_STAT'));b=stat.index('    AuraEffectList const& statFromStat',a)
code=Path(__file__).with_name('harness.cpp').read_text().replace('@ENUM@',block(unit,'enum CombatRating : uint8')+';\n'+next(line for line in unit.splitlines() if line.startswith('#define MAX_COMBAT_RATING '))).replace('@ENCHANT@',block(sp,'void Spell::EffectEnchantHeldItem(')).replace('@RATING@',block(up,'void Player::UpdateRating(')).replace('@HANDLER@',block(au,'void AuraEffect::HandleModRatingFromStat(')).replace('@STAT@',stat[a:b])
def records(name):
 b=(Path('/data/dbc')/name).read_bytes();_,n,f,z,_=struct.unpack_from('<4s4I',b);return {v[0]:v for i in range(n) if (v:=struct.unpack_from('<'+str(f)+'I',b,20+i*z))}
spells=records('Spell.dbc');durations=records('SpellDuration.dbc');enchants=records('SpellItemEnchantment.dbc')
assert enchants[45][2]==3 and enchants[45][11]==977848
assert spells[977849][71]==92 and spells[977849][110]==45 and durations[spells[977849][40]][1]==60000
assert spells[977848][96:98]==(220,220) and spells[977848][111:113]==(16777216,1792) and spells[977848][114:116]==(0,0)
assert spells[977848][81]+spells[977848][75]==12 and spells[977848][82]+spells[977848][76]==12
with tempfile.TemporaryDirectory() as tmp:
 p=Path(tmp);(p/'test.cpp').write_text(code);subprocess.run(['c++','-std=c++20',str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
print('PASS native enchant45 held-item application and native aura220 rating apply/remove/Strength-change:mainhand,other-enchant exclusion,renewal,12percentSTRmaskedratings; equipmentDB serialization separately qualified')
