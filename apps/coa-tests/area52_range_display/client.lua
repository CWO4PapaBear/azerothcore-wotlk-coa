function Area52InstallRangeTooltip(tooltip)
if tooltip:GetName() ~= "GameTooltip" or Area52RangeView then return end
local state = { serial = 0 }
Area52RangeView = state
local prefix = "A52RANGE"
local frame = CreateFrame("Frame")
local function restore()
    if state.line and state.line:GetText() == state.written then
        state.line:SetText(state.original)
    end
    state.line, state.original, state.written = nil, nil, nil
end
local function request()
    local _, _, id = GameTooltip:GetSpell()
    restore()
    state.serial = state.serial + 1
    state.id = id
    state.pending = id and true or nil
    if state.pending and not state.delay then state.delay = 0.2 end
end
frame:SetScript("OnUpdate", function(self, elapsed)
    if not state.delay then return end
    state.delay = state.delay - elapsed
    if state.delay > 0 then return end
    state.delay = nil
    if state.pending and state.id and GameTooltip:IsShown() then
        SendAddonMessage(prefix, state.serial.." "..state.id, "WHISPER", UnitName("player"))
    end
    state.pending = nil
end)
local function rangePattern(template)
    return "^"..template:gsub("%%[sd]", "@NUMBER@"):gsub("([%^%$%(%)%%%.%[%]%*%+%-%?])", "%%%1")
        :gsub("@NUMBER@", "[%%d%%.]+%%s*%%-?%%s*[%%d%%.]*").."$"
end
frame:SetScript("OnEvent", function(self, event, a, message, channel, sender)
    if event == "PLAYER_LEAVING_WORLD" then
        restore(); state.serial = state.serial + 1; state.id, state.pending, state.delay = nil, nil, nil
        return
    end
    if event ~= "CHAT_MSG_ADDON" then
        if event ~= "UNIT_AURA" or a == "player" then request() end
        return
    end
    if a ~= prefix or channel ~= "WHISPER" or sender ~= UnitName("player") then return end
    local serial, id, minimum, maximum = message:match("^(%d+) (%d+) ([%d%.]+) ([%d%.]+)$")
    serial, id, minimum, maximum = tonumber(serial), tonumber(id), tonumber(minimum), tonumber(maximum)
    local _, _, current = GameTooltip:GetSpell()
    if serial ~= state.serial or id ~= current or id ~= state.id or not GameTooltip:IsShown() then return end
    if not maximum or maximum <= 0 then return end
    for i = 1, math.min(GameTooltip:NumLines(), 8) do
        for _, side in ipairs({"Left", "Right"}) do
            local line = _G[GameTooltip:GetName().."Text"..side..i]
            local text = line and line:GetText()
            local template = SPELL_RANGE
            if text and template and text:match(rangePattern(template)) then
                state.line, state.original = line, text
                local value = minimum > 0 and string.format("%g-%g", minimum, maximum)
                    or string.format("%g", maximum)
                state.written = string.format(template, value)
                line:SetText(state.written)
                return
            end
        end
    end
end)
frame:RegisterEvent("CHAT_MSG_ADDON")
frame:RegisterEvent("UNIT_AURA")
frame:RegisterEvent("SPELLS_CHANGED")
frame:RegisterEvent("PLAYER_ENTERING_WORLD")
frame:RegisterEvent("PLAYER_LEAVING_WORLD")
GameTooltip:HookScript("OnTooltipSetSpell", request)
GameTooltip:HookScript("OnHide", function() restore(); state.serial = state.serial + 1; state.id, state.pending, state.delay = nil, nil, nil end)
end
