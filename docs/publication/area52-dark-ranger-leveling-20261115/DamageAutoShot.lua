-- The normal action buttons already flash for native repeat casts. This tracks
-- repeat casts started by the server before the client has a native repeat.
local prefix = "A52Shot"
local sequence
local buttons = setmetatable({}, {__mode = "k"})

local function isAutoShot(button)
    if not button.action then return false end
    local kind, id, _, spell = GetActionInfo(button.action)
    if kind == "macro" then
        _, _, spell = GetMacroSpell(id)
    end
    return (kind == "spell" or kind == "macro") and (spell == 75 or (kind == "spell" and id == 75 and not spell))
end

local function refresh(button)
    if sequence and isAutoShot(button) then
        ActionButton_StartFlash(button)
    end
end

local function cancel(button)
    if not sequence or not isAutoShot(button) or IsModifiedClick("PICKUPACTION") then return end
    local kind, id = GetActionInfo(button.action)
    if kind == "macro" then
        -- A macro can show Auto Shot while executing another action, or use
        -- /cast !Auto Shot specifically to keep firing. Trust its native action.
        return
    end
    SendAddonMessage(prefix, "stop " .. sequence, "WHISPER", UnitName("player"))
end

local function register(button)
    if buttons[button] then return end
    buttons[button] = true
    button:HookScript("PostClick", cancel)
end

hooksecurefunc("ActionButton_OnLoad", register)
hooksecurefunc("ActionButton_UpdateFlash", refresh)
local events = CreateFrame("Frame")
events:RegisterEvent("PLAYER_ENTERING_WORLD")
events:RegisterEvent("CHAT_MSG_ADDON")
events:SetScript("OnEvent", function(_, event, tag, value, channel, sender)
    if event == "PLAYER_ENTERING_WORLD" then
        sequence = nil
        for _, name in ipairs({"ActionButton","BonusActionButton","MultiBarRightButton",
            "MultiBarLeftButton","MultiBarBottomRightButton","MultiBarBottomLeftButton"}) do
            for i = 1, 12 do
                local button = _G[name .. i]
                if button then register(button) end
            end
        end
        SendAddonMessage(prefix, "query", "WHISPER", UnitName("player"))
    elseif tag == prefix and channel == "WHISPER" and sender == UnitName("player") and value:match("^%d+$") then
        sequence = value ~= "0" and value or nil
        for button in pairs(buttons) do
            if button.action then ActionButton_UpdateFlash(button) end
        end
    end
end)

