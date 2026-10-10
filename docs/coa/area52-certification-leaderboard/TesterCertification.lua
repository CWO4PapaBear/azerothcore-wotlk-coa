local prefix = "A52CERT"
local mysticMask = 2147483648
local status = {}
local receiving
local ready = false
local pending
local leaderboard
local boardRequest
local boardRows
local frame = CreateFrame("Frame")

local function Active()
    return GetRealmName() == "Area 52 - Free-Pick" and C_Player and C_Player:IsHero()
end

local function Print(message)
    DEFAULT_CHAT_FRAME:AddMessage("|cff00ccffArea 52:|r " .. message)
end

local function Send(message)
    if Active() then
        SendAddonMessage(prefix, message, "WHISPER", UnitName("player"))
    end
end

local function RequestLeaderboard(page)
    if not Active() then return end
    if boardRequest then return end
    boardRequest = {}
    local request = boardRequest
    leaderboard.info:SetText("Loading certification totals...")
    Send("L|" .. page)
    C_Timer.After(15, function()
        if boardRequest == request then
            boardRequest = nil
            boardRows = nil
            leaderboard.info:SetText("No response. Click Refresh to try again.")
        end
    end)
end

local function OpenLeaderboard()
    if not Active() then return end
    if not leaderboard then
        leaderboard = CreateFrame("Frame", "Area52CertificationLeaderboard", UIParent)
        leaderboard:SetSize(440, 405)
        leaderboard:SetPoint("CENTER")
        leaderboard:SetFrameStrata("DIALOG")
        leaderboard:SetBackdrop({bgFile="Interface\\DialogFrame\\UI-DialogBox-Background",
            edgeFile="Interface\\DialogFrame\\UI-DialogBox-Border", tile=true, tileSize=32, edgeSize=32,
            insets={left=11,right=12,top=12,bottom=11}})
        leaderboard:EnableMouse(true)
        leaderboard:SetMovable(true)
        leaderboard:RegisterForDrag("LeftButton")
        leaderboard:SetScript("OnDragStart", function(self) self:StartMoving() end)
        leaderboard:SetScript("OnDragStop", function(self) self:StopMovingOrSizing() end)
        tinsert(UISpecialFrames, "Area52CertificationLeaderboard")
        local close = CreateFrame("Button", nil, leaderboard, "UIPanelCloseButton")
        close:SetPoint("TOPRIGHT", -5, -5)
        local title = leaderboard:CreateFontString(nil, "OVERLAY", "GameFontNormalLarge")
        title:SetPoint("TOP", 0, -22)
        title:SetText("Certification Leaderboard")
        local help = leaderboard:CreateFontString(nil, "OVERLAY", "GameFontHighlightSmall")
        help:SetPoint("TOP", 0, -48)
        help:SetText("Account totals across all characters; shown by a tester's character name.")
        local heading = leaderboard:CreateFontString(nil, "OVERLAY", "GameFontNormal")
        heading:SetPoint("TOPLEFT", 25, -78)
        heading:SetText("Rank     Tester")
        local total = leaderboard:CreateFontString(nil, "OVERLAY", "GameFontNormal")
        total:SetPoint("TOPRIGHT", -30, -78)
        total:SetText("Certified")
        leaderboard.rows = {}
        for i = 1, 10 do
            local name = leaderboard:CreateFontString(nil, "OVERLAY", "GameFontHighlight")
            name:SetPoint("TOPLEFT", 25, -80 - i * 22)
            local count = leaderboard:CreateFontString(nil, "OVERLAY", "GameFontHighlight")
            count:SetPoint("TOPRIGHT", -30, -80 - i * 22)
            leaderboard.rows[i] = {name=name,count=count}
        end
        leaderboard.info = leaderboard:CreateFontString(nil, "OVERLAY", "GameFontHighlightSmall")
        leaderboard.info:SetPoint("BOTTOM", 0, 60)
        local function Button(text, x, action)
            local button = CreateFrame("Button", nil, leaderboard, "UIPanelButtonTemplate")
            button:SetSize(110, 24)
            button:SetPoint("BOTTOM", x, 24)
            button:SetText(text)
            button:SetScript("OnClick", action)
            return button
        end
        leaderboard.previous = Button("Previous", -125, function() RequestLeaderboard(math.max(1, (leaderboard.page or 1)-1)) end)
        Button("Refresh", 0, function() RequestLeaderboard(leaderboard.page or 1) end)
        leaderboard.next = Button("Next", 125, function() RequestLeaderboard((leaderboard.page or 1)+1) end)
    end
    leaderboard:Show()
    RequestLeaderboard(1)
end

SLASH_AREA52CERTLEADERBOARD1 = "/a52leaderboard"
SlashCmdList["AREA52CERTLEADERBOARD"] = OpenLeaderboard

StaticPopupDialogs["AREA52_CERTIFY_ABILITY"] = {
    text = "%s\n\nHave you confirmed that this ability:\n\n1. Functions as expected\n2. Persists through logout and login\n3. Is removed from bars if unlearned.",
    button1 = "CERTIFY",
    button2 = CANCEL,
    timeout = 0,
    whileDead = true,
    hideOnEscape = true,
    preferredIndex = 3,
    OnAccept = function(self, data)
        if not data or not ready or status[data.id] ~= "V" or pending then return end
        pending = { id = data.id, sent = GetTime() }
        local request = pending
        Send("C|" .. data.id .. "|7")
        Print("Saving certification...")
        C_Timer.After(15, function()
            if pending == request then
                pending = nil
                Print("No certification response received. Please try again.")
            end
        end)
    end,
}

local function AddCertification(self, dropdown, level)
    if not Active() or not ready or level and level ~= 1 then return end
    if BuildCreatorUtil and BuildCreatorUtil.IsPickingSpells() then return end
    local entry = dropdown.targetEntry
    if not entry or status[entry.ID] ~= "V" then return end
    local id, name = entry.ID, entry.Name
    local info = UIDropDownMenu_CreateInfo()
    info.notCheckable = true
    info.text = "Mark as |cff00ff00CERTIFIED|r"
    info.disabled = pending ~= nil
    info.func = function()
        CloseDropDownMenus()
        StaticPopup_Show("AREA52_CERTIFY_ABILITY", name, nil, { id = id })
    end
    UIDropDownMenu_AddButton(info, level)
end

local function InstallMenu()
    if CharacterAdvancement and not CharacterAdvancement.area52CertificationMenu then
        CharacterAdvancement.area52CertificationMenu = true
        hooksecurefunc(CharacterAdvancement, "InitializeSpellDropDown", AddCertification)
        UIDropDownMenu_Initialize(CharacterAdvancement.SpellDropDownMenu, function(...)
            CharacterAdvancement:InitializeSpellDropDown(...)
        end, "MENU")
    end
end

local function ReplaceStatus(tooltip, left, right, spellID)
    if not Active() or not ready or not C_CharacterAdvancement then return end
    local entry = C_CharacterAdvancement.GetEntryBySpellID(spellID)
    if status[mysticMask + spellID] ~= "C" and (not entry or status[entry.ID] ~= "C") then return end
    local text = left:GetText()
    if text then
        local updated = text:gsub("|c[fF][fF][fF][fF][fF][fF]00VERIFIED|r", "|cff00ff00CERTIFIED|r")
        if updated ~= text then left:SetText(updated) end
    end
end

local function InstallTooltip()
    if ModTooltipSetSpell and not ModTooltipSetSpell.Area52Certification then
        ModTooltipSetSpell.Area52Certification = ReplaceStatus
    end
end

frame:RegisterEvent("PLAYER_LOGIN")
frame:RegisterEvent("ADDON_LOADED")
frame:RegisterEvent("CHAT_MSG_ADDON")
frame:SetScript("OnEvent", function(self, event, incomingPrefix, message, channel, sender)
    if event == "ADDON_LOADED" then
        InstallMenu()
        InstallTooltip()
    elseif event == "PLAYER_LOGIN" then
        InstallMenu()
        InstallTooltip()
        Send("SYNC")
    elseif incomingPrefix == prefix and channel == "WHISPER" and sender == UnitName("player") and Active() then
        if message:match("^LB|") and boardRequest then
            local page, pages, accounts = message:match("^LB|(%d+)|(%d+)|(%d+)$")
            if page then boardRows = {page=tonumber(page),pages=tonumber(pages),accounts=tonumber(accounts)} end
        elseif message:match("^LR|") and boardRows then
            local rank, name, total, own = message:match("^LR|(%d+)|([^|]+)|(%d+)|([01])$")
            if rank and #boardRows < 10 then
                boardRows[#boardRows+1] = {rank=rank,name=name,total=total,own=own=="1"}
            end
        elseif message == "LE" and boardRows then
            leaderboard.page = boardRows.page
            for i, row in ipairs(leaderboard.rows) do
                local data = boardRows[i]
                row.name:SetText(data and (data.rank..".     "..data.name..(data.own and " (you)" or "")) or "")
                row.count:SetText(data and data.total or "")
            end
            leaderboard.info:SetText(boardRows.accounts == 0 and "No tester certifications recorded yet." or
                ("Page "..boardRows.page.." / "..boardRows.pages.."  -  "..boardRows.accounts.." testers"))
            if boardRows.page > 1 then leaderboard.previous:Enable() else leaderboard.previous:Disable() end
            if boardRows.page < boardRows.pages then leaderboard.next:Enable() else leaderboard.next:Disable() end
            boardRows = nil
            boardRequest = nil
        elseif message == "BEGIN" then
            receiving = {}
            ready = false
        elseif message == "END" and receiving then
            status = receiving
            receiving = nil
            ready = true
        elseif message:match("^[VC]|") then
            local target = receiving or status
            local value = message:sub(1, 1)
            for id in message:gmatch("|(%d+)") do target[tonumber(id)] = value end
        elseif message:match("^OK|") then
            local id = tonumber(message:sub(4))
            status[id] = "C"
            if pending and pending.id == id then
                pending = nil
                Print("|cff00ff00CERTIFIED|r — thank you for testing. Updated for everyone.")
            end
        elseif message:match("^ERROR|") then
            if boardRequest then
                boardRequest = nil
                boardRows = nil
                leaderboard.info:SetText(message:sub(7))
            end
            pending = nil
            Print(message:sub(7))
        end
    end
end)

StaticPopupDialogs["AREA52_CERTIFY_MYSTIC"] = {
    text = "%s\n\nHave you confirmed that this Mystic Enchant:\n\n1. Functions as expected\n2. Its effects and granted abilities persist through logout and login while equipped\n3. Its effects and granted abilities are removed when unequipped, unless granted by another source.",
    button1 = "CERTIFY", button2 = CANCEL, timeout = 0,
    whileDead = true, hideOnEscape = true, preferredIndex = 3,
    OnAccept = StaticPopupDialogs["AREA52_CERTIFY_ABILITY"].OnAccept,
}
SLASH_AREA52CERTIFYMYSTIC1 = "/a52certme"
SlashCmdList["AREA52CERTIFYMYSTIC"] = function(message)
    if not Active() or not ready then Print("Certification data is not ready."); return end
    local spell = tonumber(message:match("^%s*(%d+)%s*$"))
    if not spell or spell <= 0 or spell >= mysticMask then
        Print("Use /a52certme followed by the Mystic Enchant spell ID."); return
    end
    local id = mysticMask + spell
    if status[id] ~= "V" then Print("That Mystic Enchant is not awaiting certification."); return end
    if pending then Print("Please wait for the current certification request."); return end
    StaticPopup_Show("AREA52_CERTIFY_MYSTIC", GetSpellInfo(spell) or tostring(spell), nil, { id = id })
end
