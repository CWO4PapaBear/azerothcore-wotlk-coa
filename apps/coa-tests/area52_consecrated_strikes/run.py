from pathlib import Path
import subprocess,tempfile,struct
r=Path(__file__).resolve().parents[3]
def block(s,m):
 a=s.index(m);i=s.index('{',a)+1;n=1
 while n:n+=(s[i]=='{')-(s[i]=='}');i+=1
 return s[a:i]
s=(r/'src/server/coa/Area52ConsecratedStrikes.inc').read_text().replace(' override','')
pal=(r/'src/server/scripts/Spells/spell_paladin.cpp').read_text();pal=pal[pal.index('class spell_pal_seal_of_command_aura'):]
source=(r/'src/server/coa/AscensionMysticEnchant.cpp').read_text();a=source.index('bool const rankDue = grant.Enchant == 965295');b=source.index(';',a)+1
epics=(r/'src/server/coa/AscensionMysticKnightEpics.cpp').read_text();start=epics.index('    new ConsecratedStrikesCasts();');end=epics.index('    new ConsecratedWeaponHits();',start)
code=Path(__file__).with_name('harness.cpp').read_text().replace('@PRODUCTION@',s).replace('@SEAL@',block(pal,'void HandleProc(')).replace('@LEVEL@',source[a:b]).replace('@REGISTER@',epics[start:end])
def rows(name):
 b=(Path('/data/dbc')/name).read_bytes();_,n,f,z,_=struct.unpack_from('<4s4I',b);return [struct.unpack_from('<'+str(f)+'I',b,20+i*z) for i in range(n)]
charges=dict(rows('SpellCharges.dbc'));cats={v[0]:v for v in rows('SpellChargesCategory.dbc')};assert cats[charges[278745]][1:]==(3,8000)
spell={v[0]:v for v in rows('Spell.dbc') if v[0] in {278745,278746,965295}};assert spell[278745][39]==10 and spell[278745][80]+spell[278745][74]==120;assert spell[278746][80]+spell[278746][74]==30 and spell[278746][225]==1;assert spell[965295][35]==50
with tempfile.TemporaryDirectory() as tmp:
 p=Path(tmp);(p/'test.cpp').write_text(code);subprocess.run(['c++','-std=c++20','-I',str(r/'src/server/coa'),str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
print('PASS actual groundtrigger and physicalSeal handler, canonicalgrant membership and extractedlevel10 gate; native chargecategory3/8000 and120/30damage metadata. Full charge persistence/login qualified separately.')

grantSource=(r/'src/server/coa/AscensionMysticEnchant.cpp').read_text();grantCode=Path(__file__).with_name('grants.cpp').read_text().replace('GRANT_FUNCTION',block(grantSource,'void SetGrantedSpell('))
with tempfile.TemporaryDirectory() as tmp:
 p=Path(tmp);(p/'grants.cpp').write_text(grantCode);subprocess.run(['c++','-std=c++20',str(p/'grants.cpp'),'-o',str(p/'grants')],check=True);subprocess.run([str(p/'grants')],check=True)
print('PASS actual SetGrantedSpell for Templar: durable grant/actionbar across repeated restoration/logout; shared ownership, independent grants and abandonment removal')

playerSource=(r/'src/server/game/Entities/Player/Player.cpp').read_text();chargeCode=Path(__file__).with_name('charges.cpp').read_text().replace('@METHODS@','\n'.join(block(playerSource,sig) for sig in ['SpellChargeState Player::GetSpellCharges(', 'void Player::StoreSpellCharges(', 'void Player::ConsumeSpellCharge(']))
with tempfile.TemporaryDirectory() as tmp:
 p=Path(tmp);(p/'charges.cpp').write_text(chargeCode);subprocess.run(['c++','-std=c++20','-I',str(r/'src/server/game/Spells'),str(p/'charges.cpp'),'-o',str(p/'charges')],check=True);subprocess.run([str(p/'charges')],check=True)
print('PASS native Player charge consume/read/write and SpellChargeState:3charges,sequential8srecovery,emptyrejection,partialpool relog/restart,exactboundary and no reset on consume')
