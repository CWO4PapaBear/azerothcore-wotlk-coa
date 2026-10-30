from pathlib import Path
import sys, json, struct
T=Path(__file__).resolve().parent
sys.path.insert(0,str(T.parent/'duality-sidebar-20261018/test-deps'))
from lupa.lua51 import LuaRuntime
from creation_references import link_references
source=(T/'catalog/CharacterCreationArchetypes.dbc').read_bytes()
ids=[e['id'] for g in json.loads((T/'archetypes.json').read_text()) for e in g['entries']]
candidate=link_references(source,ids)
assert candidate==link_references(candidate,ids)
_,count,fields,size,_=struct.unpack_from('<4s4I',source)
pool=20+count*size
for index in range(count):
 offset=20+index*size
 archetype=struct.unpack_from('<I',source,offset)[0]
 before=source[offset:offset+size];after=candidate[offset:offset+size]
 assert before[:8]==after[:8] and before[12:]==after[12:]
 if archetype in ids:
  pos=pool+struct.unpack_from('<I',after,8)[0]
  assert candidate[pos:candidate.index(b'\0',pos)].decode()==str(2**63+archetype)
 else:assert before==after
assert candidate[pool:len(source)]==source[pool:]
l=LuaRuntime(unpack_returned_tuples=True)
l.execute('''
timers={};calls={};cleared={};level=1;name='Newhero';realm='Area 52 - Free-Pick'
GetRealmName=function() return realm end
UnitName=function() return name end;UnitLevel=function() return level end
C_Timer={After=function(_,fn) table.insert(timers,fn) end}
C_BuildCreator={ActivateBuild=function(id,a,b) assert(a and b);table.insert(calls,id) end}
NewCharacterSetupUtil={SetCharacterData=function(n,k,v) assert(k=='archetypeBuildID' and v==nil);table.insert(cleared,n) end}
UIErrorsFrame={AddMessage=function() warnings=(warnings or 0)+1 end}
CreateFrame=function() return {RegisterEvent=function() end,SetScript=function(self,key,fn) handler=fn end} end
function tick() local fn=table.remove(timers,1);assert(fn);fn() end
''')
l.execute((T/'DraftFirstLogin.lua').read_text())
l.execute('''
assert(Area52DraftFirstLogin.Begin(name,'9223372036854775827'))
assert(#calls==0 and #cleared==0);tick();assert(#calls==1)
handler(nil,nil,'ACTIVATE_BUILD_NOT_AVAILABLE');assert(#cleared==0);tick();assert(#calls==2)
handler(nil,nil,'ACTIVATE_BUILD_OK');assert(#cleared==1);tick();assert(#calls==2)
Area52DraftFirstLogin.Begin(name,'9223372036854775827');tick()
handler(nil,nil,'ACTIVATE_BUILD_NOT_FOUND');tick();assert(#cleared==1 and warnings==1)
Area52DraftFirstLogin.Begin(name,'9223372036854775827')
for i=1,13 do tick() end
assert(#cleared==1 and warnings==2)
Area52DraftFirstLogin.Begin(name,'9223372036854775827');level=2;local n=#calls;tick();assert(#calls==n)
realm='Other';assert(not Area52DraftFirstLogin.Begin(name,'123'))
''')
l.execute('assert(loadstring(...))',(T/'client/NewCharacterSetupUtil.lua').read_text())
l.execute('''
ACCEPT='Accept';CANCEL='Cancel';StaticPopupDialogs={};Enum={BuildCategory={BuildDraft='BuildDraft'}}
BuildCreatorUtil={ToggleBuildActive=function() originalCalled=true end,IsActiveBuildID=function() return active or false end}
StaticPopup_Show=function(_,text,_,data) warning=text;change=data end
C_BuildCreator.DeactivateBuild=function(id) deactivated=id end
''')
l.execute((T/'DraftActivationUI.lua').read_text())
l.execute('''
realm='Area 52 - Free-Pick';level=9
BuildCreatorUtil.ToggleBuildActive('100','BuildDraft',false)
assert(change.automatic and warning:find('before level 10'))
level=10;BuildCreatorUtil.ToggleBuildActive('100','BuildDraft',false)
assert(warning:find('forfeits'))
active=true;BuildCreatorUtil.ToggleBuildActive('100','BuildDraft',true)
assert(warning:find('ALL equipped') and change.deactivate)
StaticPopupDialogs.AREA52_DRAFT_BUILD_CHANGE.OnAccept(nil,change);assert(deactivated=='100')
active=false;BuildCreatorUtil.ToggleBuildActive('200','Leveling',false)
assert(not change.automatic and warning:find('Ordinary builds'))
realm='Other';BuildCreatorUtil.ToggleBuildActive('100','BuildDraft',false);assert(originalCalled)
''')
l.execute('assert(loadstring(...))',(T/'client/BuildCreatorUtil.lua').read_text())
(T/'creation-checks.json').write_text(json.dumps({'passed':True,'references':len(ids),'checks':['exact uint64 strings','unrelated fields and original strings preserved','idempotence','startup delay retry','acknowledgement before clearing','unassigned selection retained','bounded missing response retries','level and realm isolation'],'scope':'Lua 5.1 API adapters and actual DBC binary; no live client'},indent=2))
print('Creation references and first-login acknowledgement checks passed')
