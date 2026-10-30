from pathlib import Path
import sys,json,hashlib
T=Path(__file__).resolve().parent;W=T.parent
sys.path.insert(0,str(W/'duality-sidebar-20261018/test-deps'))
from lupa.lua51 import LuaRuntime
l=LuaRuntime(unpack_returned_tuples=True)
l.execute('''
Enum={};
CANCEL='Cancel';CLOSE='Close';CREATE_BUILD='Create Build';realm='Area 52 - Free-Pick';gm=false;discard=0;original=0
GetRealmName=function() return realm end
C_Player={IsGM=function() return gm end}
BuildCreatorUtil={IsCategoryBuildDraft=function(c) return c=='BuildDraft' or c=='BuildDraftEndGamePvE' or c=='BuildDraftEndGamePvP' end}
UIErrorsFrame={AddMessage=function(self,s) errorText=s end}
StaticPopupDialogs={};StaticPopup_Show=function(k,a,b,data) popup=k;popupData=data end
BuildCreatorMixin={CreateBuild=function(self,c,d) original=original+1;category=c;deserialize=d end}
BuildCreatorFrame=setmetatable({ToggleCreateButton={SetText=function() end},ShowBuildCategories=function() browsed=true end,SelectCategory=function(self,c) selected=c end},{__index=BuildCreatorMixin})
C_BuildEditor={DiscardPendingBuild=function() discard=discard+1;pending={Spells={},RandomEnchants={},ArmorTypes={},WeaponTypes={}} end}
for _,k in ipairs({'Name','Icon','Category','Description','Roles','PrimaryStat','DifficultyRating'}) do local key=k;C_BuildEditor['Set'..key]=function(v) pending[key]=v end end
for method,key in pairs({AddSpell='Spells',AddRandomEnchant='RandomEnchants',AddArmorType='ArmorTypes',AddWeaponType='WeaponTypes'}) do local k=key;C_BuildEditor[method]=function(v) table.insert(pending[k],v) end end
function frame() return {SetHeight=function(self,v) self.height=v end,SetSize=function(self,w,h) self.width=w;self.height=h end,SetPoint=function(self,a,x,y) self.x=x;self.y=y end,SetText=function(self,t) self.text=t end,SetScript=function(self,k,v) self[k]=v end} end
CreateFrame=function() return frame() end
sample={ID=123,Owner=456,Featured=true,Name='Test build',Icon='icon',Description='Rotation\\nHoly – test',Category='BuildDraft',Roles='PLAYER_ROLE_TANK',PrimaryStat='STAT_INTELLECT',DifficultyRating='BUILD_DIFFICULTY_RATING_NORMAL',
Spells={{Spell=100,Level=1,IsCoreAbility=true,Comment='first',Flags=1},{Spell=101,Level=63,IsOptimalAbility=true}},RandomEnchants={{Enchant=200,Stacks=3,Level=20}},ArmorTypes={{Type='ITEM_SUBCLASS_ARMOR_PLATE'}},WeaponTypes={{Type='ITEM_SUBCLASS_WEAPON_SWORD'}}}
''')
# Load real client enum blocks, not numeric approximations.
import re
native=(T/'catalog/Enum.lua').read_text()
for name in ('BuildCategory','BuildCreateCategory','BuildRoles','BuildDifficulty','ArmorSubClassSID','WeaponSubClassSID'):
 l.execute(re.search(r'Enum\.'+name+r' = \{.*?\n\}',native,re.S).group())
l.execute((T/'BuildImport.lua').read_text())
l.execute('''
code=Area52BuildImport.Encode(sample);copy=Area52BuildImport.Decode(code)
assert(copy.Name==sample.Name and copy.Description==sample.Description and copy.Spells[2].Level==63)
assert(copy.ID==nil and copy.Owner==nil and copy.Featured==nil)
assert(code==Area52BuildImport.Encode(copy))
assert(Area52BuildImport.Encode(Area52BuildImport.Decode('  '..code..'  '))==code)
for _,bad in ipairs({'','https://ascension.gg/build/1','A52B1:zz','A52B1:0',code:sub(1,-3),code..'00',string.rep('x',100001)}) do assert(not pcall(Area52BuildImport.Decode,bad)) end
assert(not Area52BuildImport.Apply({Name='bad'}));assert(discard==0)
assert(Area52BuildImport.Apply(sample));assert(discard==0 and popupData.Category=='Leveling')
StaticPopupDialogs.AREA52_CONFIRM_BUILD_COPY.OnAccept({},popupData)
assert(discard==1 and pending.Name==sample.Name and pending.Category=='Leveling' and #pending.Spells==2 and #pending.RandomEnchants==1)
assert(pending.Description==sample.Description and pending.ID==nil and category=='Leveling' and deserialize==true)
assert(sample.Category=='BuildDraft' and sample.ID==123)
gm=true;Area52BuildImport.Apply(sample);assert(popupData.Category=='BuildDraft')
StaticPopupDialogs.AREA52_CONFIRM_BUILD_COPY.OnAccept({},popupData);assert(pending.Category=='BuildDraft')
gm=false;Area52BuildImport.Commit(sample);assert(pending.Category=='Leveling')
local before=discard
BuildCreatorFrame:CreateBuild('Import',false);assert(popup=='AREA52_BUILD_IMPORT_METHOD' and discard==before)
StaticPopupDialogs.AREA52_BUILD_IMPORT_METHOD.OnAccept();assert(browsed and selected=='None' and discard==before)
assert(StaticPopupDialogs.AREA52_BUILD_IMPORT_METHOD.OnCancel==nil)
StaticPopupDialogs.AREA52_BUILD_IMPORT_METHOD.OnAlt();assert(popup=='AREA52_IMPORT_BUILD_CODE')
assert(StaticPopupDialogs.AREA52_IMPORT_BUILD_CODE.OnAccept({editBox={GetText=function() return 'invalid' end}})==true and discard==before)
StaticPopupDialogs.AREA52_IMPORT_BUILD_CODE.OnAccept({editBox={GetText=function() return code end}});assert(popup=='AREA52_CONFIRM_BUILD_COPY' and discard==before)
view={Controls=frame(),MainFrame=frame(),build=sample};Area52BuildImport.Attach(view);assert(#view.Area52ImportButtons==3 and view.Controls.height+view.MainFrame.height==669)
local buttons=view.Area52ImportButtons;Area52BuildImport.Attach(view);assert(buttons==view.Area52ImportButtons)
buttons[2].OnClick();assert(popup=='AREA52_COPY_BUILD_CODE' and popupData==code)
local edit={SetText=function(self,s) self.text=s end,SetFocus=function(self) self.focus=true end,HighlightText=function(self) self.highlight=true end}
StaticPopupDialogs.AREA52_COPY_BUILD_CODE.OnShow({editBox=edit},popupData);assert(edit.text==code and edit.focus and edit.highlight)
realm='Other';local count=original;BuildCreatorFrame:CreateBuild('Import',false);assert(original==count+1)
local other={};Area52BuildImport.Attach(other);assert(other.Area52ImportButtons==nil)
''')
# Compile both complete staged production files, without executing unrelated UI initialization.
for name in ('BuildCreator.lua','BuildView.lua','EditableBuildView.lua'):
 l.execute('assert(loadstring(...))',(T/'client'/name).read_text())
# Bounded malformed decoder inputs and duplicate/list/range failures.
for raw in ['t2:s1:an1:s1:an2:','t1:s1:at1:s1:at1:s1:at1:s1:at1:s1:at1:s1:an1:','s99999999:x','t513:']:
 assert not l.eval('function(c) return pcall(Area52BuildImport.Decode,c) end')('A52B1:'+raw.encode().hex())[0]
l.execute('''
for _,mutation in ipairs({function(b) b.Spells[2].Spell=100 end,function(b) b.PrimaryStat=5 end,function(b) b.Spells[1].Level=81 end,function(b) b.RandomEnchants[1].Stacks=0 end,function(b) b.Spells.bad={} end}) do
 local b=Area52BuildImport.Decode(code);mutation(b);assert(not pcall(Area52BuildImport.Validate,b))
end
''')
r=json.loads((T/'readback.json').read_text())
assert hashlib.sha256((W/'package/package.json').read_bytes()).hexdigest()==r['stable_package_sha256']
result={'passed':True,'checks':['roundtrip including UTF8/newline and selections','identity stripped','malformed/oversize/deep/duplicate rejection','invalid and unconfirmed import preserves draft','confirmed copy restores all editor fields','GM-only category remapping','source unchanged','internal selection and code popup routing','cancel has no import action','copy highlights complete code','button lifecycle and layout bounds','other realm unchanged','complete production Lua 5.1 syntax','stable package unchanged'],'limitations':'Offline UI/native-editor adapters; no live client, installation, publication or gameplay certification.'}
print('Import code checks passed; continuing selector checks')

# Exercise the grouped selector in Lua 5.1 against the generated native catalog.
l.execute(r"""
function widget()
 return {shown=true,SetShown=function(self,v) self.shown=v end,SetText=function(self,v) self.text=v end,
 SetScale=function(self,v) self.scale=v end,SetPoint=function() end,Disable=function(self) self.disabled=true end,Enable=function(self) self.disabled=false end}
end
CreateFrame=function() return widget() end
UIDropDownMenu_SetWidth=function(self,w) self.width=w end
UIDropDownMenu_SetText=function(self,t) self.text=t end
UIDropDownMenu_Initialize=function(self,f) self.initialize=f end
UIDropDownMenu_EnableDropDown=function(self) self.disabled=false end
UIDropDownMenu_DisableDropDown=function(self) self.disabled=true end
UIDropDownMenu_CreateInfo=function() return {} end
UIDropDownMenu_AddButton=function(info,level) table.insert(menu,info) end
CloseDropDownMenus=function() closed=true end
EditableBuildViewMixin={UpdatePendingBuild=function(self,deserialize)
 if deserialize then self.MainFrame.Name:SetText(pending.Name) end
 pending.Description=Area52DraftSelector.Description(self,pending.Description)
 self.Controls.PublishBuild:Enable()
end}
C_BuildEditor.GetPendingBuild=function() return pending end
""")
l.execute((T/'ArchetypeCatalog.lua').read_text())
l.execute((T/'DraftSelector.lua').read_text())
l.execute(r"""
realm='Area 52 - Free-Pick';gm=true
pending={ID='draft-1',Name='Old name',Category='BuildDraft',Description='###OVERVIEW\nGuide text'}
editor=setmetatable({MainFrame={Name=widget()},Controls={PublishBuild=widget()}},{__index=EditableBuildViewMixin})
editor:UpdatePendingBuild(true)
assert(not editor.MainFrame.Name.shown and editor.Area52ArchetypeDropdown.shown and editor.Controls.PublishBuild.disabled)
menu={};Area52DraftSelector.Menu(editor,1,nil);assert(#menu==9)
local total=0
for _,group in ipairs(Area52DraftArchetypes) do
 menu={};Area52DraftSelector.Menu(editor,2,group.id);assert(#menu==#group.entries)
 for i,item in ipairs(menu) do
  item.func();assert(editor.Area52ArchetypeID==group.entries[i].id and pending.Name==group.entries[i].name)
  assert(Area52DraftSelector.ReadID(pending.Description)==group.entries[i].id and Area52DraftSelector.Valid(editor))
  assert(pending.Description:find('Guide text',1,true));total=total+1
 end
end
assert(total==56)
local chosen=editor.Area52ArchetypeID
editor.Area52ArchetypeID=nil;editor:UpdatePendingBuild(true);assert(editor.Area52ArchetypeID==chosen)
local desc=pending.Description;editor:UpdatePendingBuild();assert(pending.Description==desc)
gm=false;Area52DraftSelector.Choose(editor,1);assert(editor.Area52ArchetypeID==chosen)
editor:UpdatePendingBuild(true);assert(editor.Controls.PublishBuild.disabled and editor.Area52ArchetypeDropdown.disabled)
gm=true;pending.Category='Leveling';editor:UpdatePendingBuild()
assert(editor.MainFrame.Name.shown and not editor.Area52ArchetypeDropdown.shown and editor.Area52ArchetypeID==nil)
assert(not pending.Description:find('AREA52_ARCHETYPE_ID',1,true))
pending.Category='BuildDraft';editor:UpdatePendingBuild();assert(editor.Controls.PublishBuild.disabled)
pending.Description='###OVERVIEW\nText\n\n###AREA52_ARCHETYPE_ID\n999999';editor:UpdatePendingBuild(true);assert(not Area52DraftSelector.Valid(editor))
realm='Other';local outsider={};Area52DraftSelector.Before(outsider,true);Area52DraftSelector.After(outsider);assert(outsider.Area52ArchetypeDropdown==nil)
""")
result['checks'].extend(['real client string enums','56 archetype selections across nine role/category groups','draft name hidden and ordinary name restored','GM gating','stable ID description metadata roundtrip','unknown ID blocks publish','metadata removed for ordinary builds'])
(T/'checks.json').write_text(json.dumps(result,indent=2));print('Grouped archetype selector checks PASSED')


# Native description reader must hide reserved identity metadata from player guide text.
import re
util=(W/'bot-integration-review/build-import-inspection/patch-B.MPQ/Interface/FrameXML/Util/BuildCreatorUtil.lua').read_text()
reader=util.split('function BuildCreatorUtil.UnpackDescriptionByKeys(description)',1)[1].split('function BuildCreatorUtil.GetOverviewFromDescription',1)[0]
l.execute("BuildCreatorUtil.DescriptionSectionInverted={OVERVIEW=true};BuildCreatorUtil.DescriptionSection={Overview='OVERVIEW'}")
l.execute('function BuildCreatorUtil.UnpackDescriptionByKeys(description)'+reader)
l.execute(r"""
local sections=BuildCreatorUtil.UnpackDescriptionByKeys('###OVERVIEW\nGuide\n\n###AREA52_ARCHETYPE_ID\n97')
assert(sections.OVERVIEW:find('Guide') and not sections.OVERVIEW:find('97') and sections.AREA52_ARCHETYPE_ID==nil)
""")
# Retain source hashes for the exact qualified candidate and native catalog inputs.
result['checks'].append('actual native description reader excludes selector metadata')
result['sha256']={str(p.relative_to(T)):hashlib.sha256(p.read_bytes()).hexdigest() for p in [T/'client.patch',T/'client/patch-E.MPQ',T/'DraftSelector.lua',T/'BuildImport.lua',T/'ArchetypeCatalog.lua',T/'catalog/CharacterCreationArchetypes.dbc',T/'catalog/CharacterCreationArchetypeCategories.dbc',T/'catalog/CharacterCreationArchetypeRoles.dbc',T/'catalog/Enum.lua']}
(T/'checks.json').write_text(json.dumps(result,indent=2));print('All UI and native-description checks PASSED')
