from pathlib import Path
import re, struct, subprocess, tempfile
ROOT=Path(__file__).resolve().parents[3]
def extract(text, signature):
 a=text.index(signature);b=text.index('{',a)+1;depth=1
 while depth:
  depth+=(text[b]=='{')-(text[b]=='}');b+=1
 return text[a:b]
def main():
 player=(ROOT/'src/server/game/Entities/Player/Player.cpp').read_text()
 proc=(ROOT/'src/server/coa/AscensionTalentDataProcs.cpp').read_text()
 grants=re.findall(r'\{977831,\s*\d+,\s*\d+\}',(ROOT/'src/server/coa/AscensionMysticSpellGrants.h').read_text())
 assert len(grants)==14
 code=Path(__file__).with_name('harness.cpp').read_text()
 code=code.replace('GRANTS',','.join(grants))
 code=code.replace('ORIGINS',extract(player,'std::vector<uint32> Player::GetSpellInteractionOrigins('))
 code=code.replace('MATCHES',extract(player,'bool Player::MatchesSpellInteraction('))
 a=player.index('    if (mod->spellId == 760198 && mod->op == SPELLMOD_CASTING_TIME')
 code=code.replace('WANING_MATCH',player[a:player.index('\n\n',a)])
 effects=(ROOT/'src/server/game/Spells/Auras/SpellAuraEffects.cpp').read_text()
 code=code.replace('LUNAR',extract(effects,'if (caster && AscensionFreePick::Applies(caster->ToPlayer()) && caster->HasAura(977782))'))
 code=code.replace(' PROC\n',' '+extract(proc[proc.index('class aura_area52_new_moon'):],'void Proc(')+'\n')
 dbc=Path('/data/dbc/Spell.dbc').read_bytes();_,count,fields,size,_=struct.unpack_from('<4s4I',dbc)
 rows={};masks=set();matched=0
 ids={760198,983846,81214,983848}
 for grant in grants:ids.update(map(int,re.findall(r'\d+',grant)))
 for i in range(count):
  row=struct.unpack_from('<'+str(fields)+'I',dbc,20+i*size)
  if row[0] in ids:rows[row[0]]=row
  if row[208]==7:
   for j in range(3):
    mask=row[122+j*3:125+j*3]
    if mask[0]&2:masks.add(mask);matched+=1
 code=code.replace('IS_AFFECTED',extract((ROOT/'src/server/game/Spells/SpellInfo.cpp').read_text(),'bool SpellInfo::IsAffected('))
 code=code.replace('MASK_CASES','\n'.join('assert(sun.IsAffected(7,{%du,%du,%du}) && moon.IsAffected(7,{%du,%du,%du}));' % (m+m) for m in sorted(masks)))
 with tempfile.TemporaryDirectory() as tmp:
  cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test'
  subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True)
  subprocess.run([str(exe)],check=True)
 print('PASS native IsAffected across %d Moonfire-compatible effect masks and all 14 rank pairs' % matched)
 for grant in grants:
  _,sun,moon=map(int,re.findall(r'\d+',grant))
  assert rows[sun][208:212]==rows[moon][208:212]
  assert rows[sun][71:74]==rows[moon][71:74] and rows[sun][95:98]==rows[moon][95:98]
  assert rows[sun][225]==68 and rows[moon][225]==64
 assert rows[983846][86]==6 and rows[983846][49]==5
 assert rows[760198][86]==1 and rows[760198][49]==5 and rows[760198][36]==0
 assert rows[760198][34]==0 and rows[983846][34]==0
 assert rows[760198][95]==108 and rows[760198][110]==10
 assert struct.unpack('<i',struct.pack('<I',rows[760198][80]))[0]+1==-5
 mgr=(ROOT/'src/server/game/Spells/SpellMgr.cpp').read_text()
 hook=mgr.index('sScriptMgr->OnLoadSpellCustomAttr(spellInfo)')
 assert mgr.index('spellInfo->_InitializeExplicitTargetMask()',hook)>hook
 metadata=(ROOT/'src/server/coa/Area52LegendaryBatch.inc').read_text()
 body=extract(metadata,'void OnLoadSpellCustomAttr(')
 assert body.index('CoA.GameModeMask')<body.index('info->Id == 983846')
 assert 'info->Effects[EFFECT_0].TargetA = SpellImplicitTargetInfo(TARGET_UNIT_CASTER)' in body
 assert 'info->ProcCharges = 1' in body
 assert 'MatchesSpellInteraction(spellInfo->Id, 2912) || MatchesSpellInteraction(spellInfo->Id, 30451)' in player
 remove=extract(player,'void Player::RemoveSpellMods(')
 assert 'GetSpellProcEntry(mod->spellId)' in remove and 'IsUsingCharges()' in remove
 assert remove.index('spell->m_appliedMods.erase(iterMod)')<remove.index('DropCharge(')
 helpers=Path(__file__).with_name('helpers.cpp').read_text()
 segment=body[body.index('{')+1:body.index('        if (info->Id == 983908)')]
 helpers=helpers.replace('METADATA',segment).replace('REMOVE_MODS',remove)
 with tempfile.TemporaryDirectory() as tmp:
  cpp=Path(tmp)/'helpers.cpp';cpp.write_text(helpers);exe=Path(tmp)/'helpers'
  subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True)
  subprocess.run([str(exe)],check=True)
 print('PASS DBC parity: all 14 ranks share native Moonfire family modifiers; Spellfire school retained; helper target and five-stack/-5% records reproduced; startup target cache refresh and native charged-mod removal integration retained')
if __name__=='__main__':main()
