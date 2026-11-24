
assert(GameTooltip == nil)
SPELL_RANGE = "%s yd range"

function UnitName() return "Tester" end
frames, sent = {}, {}
function CreateFrame()
 local f = {scripts={},events={}}
 function f:SetScript(k,v) self.scripts[k]=v end
 function f:RegisterEvent(k) self.events[k]=true end
 frames[#frames+1]=f
 return f
end
function SendAddonMessage(p,m,c,n) sent[#sent+1]={p,m,c,n} end
local line = {text="30 yd range"}
function line:GetText() return self.text end
function line:SetText(t) self.text=t end
GameTooltipTextLeft2=line
GameTooltip={shown=true,id=2912,hooks={}}
function GameTooltip:GetName() return "GameTooltip" end
function GameTooltip:GetSpell() return "Starfire","Rank 1",self.id end
function GameTooltip:IsShown() return self.shown end
function GameTooltip:NumLines() return 3 end
function GameTooltip:HookScript(k,v) self.hooks[k]=v end
Area52InstallRangeTooltip(GameTooltip)
Area52InstallRangeTooltip(GameTooltip)
assert(#frames==1)
local f=frames[1]
local function send() f.scripts.OnUpdate(f,.21) end
local function reply(extra, sender)
 local request=sent[#sent][2]
 f.scripts.OnEvent(f,"CHAT_MSG_ADDON","A52RANGE",request.." "..extra,"WHISPER",sender or "Tester")
end
GameTooltip.hooks.OnTooltipSetSpell()
assert(#sent==0)
send();assert(#sent==1 and sent[1][1]=="A52RANGE")
reply("0 36");assert(line.text=="36 yd range")
f.scripts.OnEvent(f,"UNIT_AURA","player");assert(line.text=="30 yd range")
send();reply("0 34");assert(line.text=="34 yd range")
f.scripts.OnEvent(f,"SPELLS_CHANGED");send();reply("0 0");assert(line.text=="30 yd range")
for i=1,50 do GameTooltip.hooks.OnTooltipSetSpell() end
local count=#sent;send();assert(#sent==count+1)
reply("0 36","Other");assert(line.text=="30 yd range")
local old=sent[#sent][2]
GameTooltip.id=8921;line.text="30 yd range";GameTooltip.hooks.OnTooltipSetSpell()
f.scripts.OnEvent(f,"CHAT_MSG_ADDON","A52RANGE",old.." 0 36","WHISPER","Tester")
assert(line.text=="30 yd range")
send();reply("0 0");assert(line.text=="30 yd range")
GameTooltip.id=2912;SPELL_RANGE="Portée : %s m";line.text="Portée : 30 m"
GameTooltip.hooks.OnTooltipSetSpell();send();reply("0 36");assert(line.text=="Portée : 36 m")
SPELL_RANGE="%s yd range";line.text="5-30 yd range"
GameTooltip.hooks.OnTooltipSetSpell();send();reply("5 36");assert(line.text=="5-36 yd range")
GameTooltip.hooks.OnHide();assert(line.text=="5-30 yd range")
reply("5 40");assert(line.text=="5-30 yd range")
GameTooltip.id=nil;GameTooltip.hooks.OnTooltipSetSpell();local n=#sent;send();assert(#sent==n)
GameTooltip.id=2912;GameTooltip.hooks.OnTooltipSetSpell();f.scripts.OnEvent(f,"PLAYER_LEAVING_WORLD");send();assert(#sent==n)
