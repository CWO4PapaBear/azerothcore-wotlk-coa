EditableBuildViewMixin = {}

local PRIMARY_STAT_STRING = {
    [1] = "STAT_STRENGTH",
    [2] = "STAT_AGILITY",
    [3] = "STAT_INTELLECT",
    [4] = "STAT_SPIRIT",
    [5] = "STAT_STAMINA",
}
local CONTENT_TOP_OFFSET = 14

local function ShouldShowMysticEnchantSection()
    return not C_GameMode:IsGameModeActive(Enum.GameMode.WildCard)
end

function EditableBuildViewMixin:OnLoad()
    self.categories = {}

    self.MainFrame.Author:SetPoint("TOPLEFT", self.MainFrame.Name, "BOTTOMLEFT", 0, -2)
    self.MainFrame.Background:SetAtlas("buildcreator-view", Const.TextureKit.IgnoreAtlasSize)
    self.MainFrame.SpecIcon:SetRounded(true)
    self.MainFrame.SpecIcon:SetBorderSize(96, 96)
    self.MainFrame.Icon:SetRounded(true)
    self.MainFrame.Icon:SetBorderSize(68, 68)
    self.MainFrame.Icon:SetBorderOffset(0, -1)
    self.MainFrame.Icon:SetBorderAtlas("build-draft-border")

    self.CategoryScroll.Background:SetAtlas("professions-background-summarylist", Const.TextureKit.IgnoreAtlasSize)
    self.CategoryScroll:SetGetNumResultsFunction(function() return #self.categories end)
    self.CategoryScroll:SetTemplate("BuildViewSectionListItemTemplate")
    local selectedTexture = self.CategoryScroll:GetSelectedHighlight()
    selectedTexture:SetAtlas("Garr_ListButton-Selection", Const.TextureKit.IgnoreAtlasSize)
    selectedTexture:SetBlendMode("ADD")
    UIDropDownMenu_Initialize(self.MainFrame.DifficultyDropDown, GenerateClosure(self.InitializeDifficultyDropDown, self))
    UIDropDownMenu_Initialize(self.MainFrame.CategoryDropDown, GenerateClosure(self.InitializeCategoryDropDown, self))
end 

function EditableBuildViewMixin:InitializeDifficultyDropDown(dropdown, level, menuList)
    if not self:IsVisible() then return end
    local info, color

    info = UIDropDownMenu_CreateInfo()
    color = BuildCreatorUtil.DifficultyColors[Enum.BuildDifficulty.Standard]
    info.text = color:WrapText(_G[Enum.BuildDifficulty.Standard])
    info.iconAtlas = BuildCreatorUtil.DifficultyAtlas[Enum.BuildDifficulty.Standard]
    info.func = function()
        C_BuildEditor.SetDifficultyRating(Enum.BuildDifficulty.Standard)
        self:UpdatePendingBuild()
    end
    UIDropDownMenu_AddButton(info, level)

    info = UIDropDownMenu_CreateInfo()
    color = BuildCreatorUtil.DifficultyColors[Enum.BuildDifficulty.Intermediate]
    info.text = color:WrapText(_G[Enum.BuildDifficulty.Intermediate])
    info.iconAtlas = BuildCreatorUtil.DifficultyAtlas[Enum.BuildDifficulty.Intermediate]
    info.func = function()
        C_BuildEditor.SetDifficultyRating(Enum.BuildDifficulty.Intermediate)
        self:UpdatePendingBuild()
    end
    UIDropDownMenu_AddButton(info, level)

    info = UIDropDownMenu_CreateInfo()
    color = BuildCreatorUtil.DifficultyColors[Enum.BuildDifficulty.Advanced]
    info.text = color:WrapText(_G[Enum.BuildDifficulty.Advanced])
    info.iconAtlas = BuildCreatorUtil.DifficultyAtlas[Enum.BuildDifficulty.Advanced]
    info.func = function()
        C_BuildEditor.SetDifficultyRating(Enum.BuildDifficulty.Advanced)
        self:UpdatePendingBuild()
    end
    UIDropDownMenu_AddButton(info, level)

    info = UIDropDownMenu_CreateInfo()
    color = BuildCreatorUtil.DifficultyColors[Enum.BuildDifficulty.Expert]
    info.text = color:WrapText(_G[Enum.BuildDifficulty.Expert])
    info.iconAtlas = BuildCreatorUtil.DifficultyAtlas[Enum.BuildDifficulty.Expert]
    info.func = function()
        C_BuildEditor.SetDifficultyRating(Enum.BuildDifficulty.Expert)
        self:UpdatePendingBuild()
    end
    UIDropDownMenu_AddButton(info, level)

    info = UIDropDownMenu_CreateInfo()
    color = BuildCreatorUtil.DifficultyColors[Enum.BuildDifficulty.Impossible]
    info.text = color:WrapText(_G[Enum.BuildDifficulty.Impossible])
    info.iconAtlas = BuildCreatorUtil.DifficultyAtlas[Enum.BuildDifficulty.Impossible]
    info.func = function()
        C_BuildEditor.SetDifficultyRating(Enum.BuildDifficulty.Impossible)
        self:UpdatePendingBuild()
    end
    UIDropDownMenu_AddButton(info, level)
end

function EditableBuildViewMixin:InitializeCategoryDropDown(dropdown, level, menuList)
    if not self:IsVisible() then return end
    local info

    info = UIDropDownMenu_CreateInfo()
    info.text = CHANGE_BUILD_CATEGORY
    info.isTitle = true
    UIDropDownMenu_AddButton(info, level)

    if not C_GameMode:IsGameModeActive(Enum.GameMode.WildCard) then
        info = UIDropDownMenu_CreateInfo()
        info.text = BUILD_EDITOR_CATEGORY_LEVELING
        info.func = function()
            BuildCreatorFrame:ConvertBuildEditorToCategory(Enum.BuildCreateCategory.Leveling)
        end
        UIDropDownMenu_AddButton(info, level)
    end
    
    local isLevel60 = GetMaxLevel() == 60

    info = UIDropDownMenu_CreateInfo()
    info.text = isLevel60 and BUILD_EDITOR_CATEGORY_LEVEL60PVE or BUILD_EDITOR_CATEGORY_LEVEL70PVE
    info.func = function()
        local category = isLevel60 and Enum.BuildCreateCategory.Level60PvE or Enum.BuildCreateCategory.Level70PvE
        BuildCreatorFrame:ConvertBuildEditorToCategory(category)
    end
    UIDropDownMenu_AddButton(info, level)

    info = UIDropDownMenu_CreateInfo()
    info.text = isLevel60 and BUILD_EDITOR_CATEGORY_LEVEL60PVP or BUILD_EDITOR_CATEGORY_LEVEL70PVP
    info.func = function()
        local category = isLevel60 and Enum.BuildCreateCategory.Level60PvP or Enum.BuildCreateCategory.Level70PvP
        BuildCreatorFrame:ConvertBuildEditorToCategory(category)
    end
    UIDropDownMenu_AddButton(info, level)

    if C_AccountInfo.GetGMLevel() > 0 then
        info = UIDropDownMenu_CreateInfo()
        info.text = BUILD_EDITOR_CATEGORY_BUILDDRAFT
        info.func = function()
            BuildCreatorFrame:ConvertBuildEditorToCategory(Enum.BuildCreateCategory.BuildDraft)
        end
        UIDropDownMenu_AddButton(info, level)
        
        info = UIDropDownMenu_CreateInfo()
        info.text = BUILD_EDITOR_CATEGORY_BUILDDRAFTENDGAMEPVE
        info.func = function()
            BuildCreatorFrame:ConvertBuildEditorToCategory(Enum.BuildCreateCategory.BuildDraftEndGamePvE)
        end
        UIDropDownMenu_AddButton(info, level)
        
        info = UIDropDownMenu_CreateInfo()
        info.text = BUILD_EDITOR_CATEGORY_BUILDDRAFTENDGAMEPVP
        info.func = function()
            BuildCreatorFrame:ConvertBuildEditorToCategory(Enum.BuildCreateCategory.BuildDraftEndGamePvP)
        end
        UIDropDownMenu_AddButton(info, level)
    end
end

function EditableBuildViewMixin:OnShow()
    self:UpdatePendingBuild(self.deserializeNext)
    self.deserializeNext = nil
end

function EditableBuildViewMixin:OnHide()
    if self.Area52ImportCancel then self.Area52ImportCancel();self.Area52ImportCancel=nil end
    self.Area52ImportSerial=(self.Area52ImportSerial or 0)+1
    self.Area52ImportPending=false
    BuildCreatorUtil.ReleaseHeldSpell()
end

function EditableBuildViewMixin:OnVerticalScroll(offset)
    local lastIndex = 0
    offset = offset + CONTENT_TOP_OFFSET + 5
    for index, categoryData in ipairs(self.categories) do
        if not categoryData.offset or categoryData.offset <= offset then
            lastIndex = index
        else
            break
        end
    end
    self.CategoryScroll:SetSelectedIndex(lastIndex, ScrollListMixin.UpdateType.OnlyNewIndex)
end

function EditableBuildViewMixin:GetCategoryHeader(index)
    local header = self.categories[index] and self.categories[index].header
    if header == "SPELLS_AND_TALENTS" and C_Player:IsDefaultClass() then
        header = "TALENTS"
    end
    return header and (_G["BUILDCREATOR_SECTION_"..header] or "BUILD_CREATOR_SECTION_"..header) or UNKNOWN
end

function EditableBuildViewMixin:SelectCategory(index)
    local data = self.categories[index]
    if not data then return end

    if self.selectedCategory == data then return end
    self.selectedCategory = data
    local frame = self.MainFrame.ScrollFrame
    local minOffset, maxOffset = frame.ScrollBar:GetMinMaxValues()
    local offset = math.clamp(data.offset - CONTENT_TOP_OFFSET, minOffset, maxOffset)
    frame:SetVerticalScroll(offset)
end

function EditableBuildViewMixin:UpdatePendingBuild(deserialize)
    local author = UnitName("player")
    self.MainFrame.Author:SetFormattedText(BUILD_AUTHOR_S, author)
    self.build = C_BuildEditor.GetPendingBuild()

    local content = self.MainFrame.ScrollFrame.Content
    local height = CONTENT_TOP_OFFSET
    local top = content:GetTop()

    if deserialize then
        local sectionText = BuildCreatorUtil.UnpackDescriptionByKeys(self.build.Description)
        for _, section in ipairs(content.sections) do
            section:Deserialize(sectionText[section.category])
        end
        
        self.MainFrame.Name:SetText(self.build.Name)
        self.MainFrame.Icon:SelectIcon("Interface\\Icons\\"..self.build.Icon)
        self.MainFrame.Role:SetRole(self.build.Roles)
        self.MainFrame.PrimaryStat:SetStat(self.build.PrimaryStat)
    end

    self.MainFrame.PrimaryStat:SetShown(C_Player:IsHero())

    local difficulty = self.build.DifficultyRating
    local difficultyColor = BuildCreatorUtil.DifficultyColors[difficulty]
    UIDropDownMenu_SetText(self.MainFrame.DifficultyDropDown, difficultyColor:WrapText(_G[difficulty]))
    UIDropDownMenu_SetText(self.MainFrame.CategoryDropDown, _G["BUILDCREATOR_CATEGORY_"..self.build.Category:upper()])

    local equipment = content.Equipment
    equipment:ClearAllPoints()
    if ShouldShowMysticEnchantSection() then
        equipment:SetPoint("TOPLEFT", content.MysticEnchants, "BOTTOMLEFT", 0, -16)
        equipment:SetPoint("TOPRIGHT", content.MysticEnchants, "BOTTOMRIGHT", 0, -16)
    else
        equipment:SetPoint("TOPLEFT", content.Spells, "BOTTOMLEFT", 0, -16)
        equipment:SetPoint("TOPRIGHT", content.Spells, "BOTTOMRIGHT", 0, -16)
    end

    local description
    local categoryIndex = 1
    for _, section in ipairs(content.sections) do
        local text = section:Serialize(self.build)
        local shouldShowSection = (section.category ~= "MYSTIC_ENCHANTS" or ShouldShowMysticEnchantSection())
            and (not section.Area52FeaturedOnly or Area52FeaturedEditor.Applies())

        if shouldShowSection then
            height = height + section:GetHeight() + (section.Area52FeaturedOnly and 16 or 0)
            self.categories[categoryIndex] = {
                header = section.category,
                offset = top - section:GetTop(),
            }
            categoryIndex = categoryIndex + 1

            if text then -- sections that dont return text update pending build themselves
                if not description then
                    description = text
                else
                    description = description .. "\n\n" .. text
                end
            end
        end
    end

    for index = categoryIndex, #self.categories do
        self.categories[index] = nil
    end
    self.MainFrame.ScrollFrame.Content:SetHeight(height)
    
    self:UpdateIcon()
    self:UpdateRole()
    self:UpdatePrimaryStat()
    if not deserialize then
        C_BuildEditor.SetDescription(Area52DraftSelector.Description(self, description))
        C_BuildEditor.SetName(self.MainFrame.Name:ToText())
        C_BuildEditor.SetIcon(self.MainFrame.Icon:ToText())
        C_BuildEditor.SetRoles(self.MainFrame.Role:ToText())
        C_BuildEditor.SetPrimaryStat(self.MainFrame.PrimaryStat:ToText())
        if string.isNilOrEmpty(self.build.ID) then
            BuildCreatorFrame:SerializePendingBuild()
        end
    end

    -- update publish button
    local canPublish, failReasons = Area52CanPublishBuild(self)
    local publishButton = self.Controls.PublishBuild
    if canPublish==true or canPublish==1 then
        publishButton.tooltipTitle = nil
        publishButton.tooltipText = nil
        publishButton:Enable()
    else
        publishButton.tooltipTitle = CANNOT_PUBLISH_BUILD
        publishButton.tooltipText = nil
        for _, reason in ipairs(failReasons) do
            if publishButton.tooltipText then
                publishButton.tooltipText = publishButton.tooltipText .. "\n"
            else
                publishButton.tooltipText = ""
            end
            publishButton.tooltipText = publishButton.tooltipText .. Area52BuildPublishReason(reason)
        end
        publishButton.tooltipText = RED_FONT_COLOR:WrapText(publishButton.tooltipText)
        publishButton:Disable()
    end
end

function EditableBuildViewMixin:OnEditModeHide()
    self:UpdatePendingBuild()
end

function EditableBuildViewMixin:OnEditModeShow()
    self:UpdatePendingBuild()
end

function EditableBuildViewMixin:LoadPendingBuild()
    
end 

function EditableBuildViewMixin:UpdateIcon()
    local icon = self.MainFrame.Icon:ToText()
    if not icon or icon:len() == 0 then
        icon = "Interface\\Icons\\INV_Misc_QuestionMark"
    end
    C_BuildEditor.SetIcon(icon)
end

function EditableBuildViewMixin:UpdateRole()
    local role = self.MainFrame.Role:ToText()
    if not role or role:len() == 0 then
        role = "PLAYER_ROLE_NONE"
    end
    C_BuildEditor.SetRoles(role)
    local roleKey = BuildCreatorUtil.ConvertBuildRoleToLFGRole(role)
    if roleKey then
        self.MainFrame.ContentBackground:SetTexture("Interface\\BuildCreator\\BCBackground-"..roleKey, "REPEAT", "REPEAT")
        self.MainFrame.Role.tooltipTitle = BUILD_CREATOR_ROLE_S:format(_G[roleKey])
        self.MainFrame.Role.tooltipText = _G["BUILD_CREATOR_ROLE_"..roleKey]
    else
        self.MainFrame.ContentBackground:SetTexture(nil)
        self.MainFrame.Role.tooltipTitle = nil
        self.MainFrame.Role.tooltipText = nil
    end
    if GameTooltip:IsOwned(self.MainFrame.Role) then
        self.MainFrame.Role:CallScript("OnEnter")
    end
end

function EditableBuildViewMixin:UpdatePrimaryStat()
    local stat = self.MainFrame.PrimaryStat:ToText()
    if not stat or stat:len() == 0 then
        stat = "STAT_STRENGTH"
    end
    C_BuildEditor.SetPrimaryStat(stat)
end 

function EditableBuildViewMixin:SetCategory(category)
    self.levelingBuild = category == Enum.BuildCreateCategory.Leveling or category == Enum.BuildCreateCategory.BuildDraft
    C_BuildEditor.SetCategory(category)
end

function EditableBuildViewMixin:IsLevelingBuild()
    return self.levelingBuild
end 

function EditableBuildViewMixin:ImportCurrentBuild()
    self.MainFrame.Icon:SelectIcon("Interface\\Icons\\INV_Misc_QuestionMark")
    self.MainFrame.Role:SetRole("PLAYER_ROLE_NONE")
    local primaryStat = C_PrimaryStat:GetActivePrimaryStat()
    if not primaryStat then
        primaryStat = Enum.PrimaryStat.Strength
    end
    self.MainFrame.PrimaryStat:SetStat(PRIMARY_STAT_STRING[primaryStat])
    
    local spell = BuildCreatorUtil.CreateSpellInfo(0) -- reused
    for _, spellID in ipairs(C_CharacterAdvancement.GetKnownSpells()) do
        -- only add talents if default class, otherwise add everything
        if not C_Player:IsDefaultClass() or C_CharacterAdvancement.IsTalentSpellID(self.spellID) then
            local talentRank = C_CharacterAdvancement.GetTalentRankBySpellID(spellID)
            if talentRank then
                local entry = C_CharacterAdvancement.GetEntryBySpellID(spellID)
                for i = 1, talentRank do
                    spell.Spell = entry.Spells[i]
                    C_BuildEditor.AddSpell(spell)
                end
            else
                spell.Spell = spellID
                C_BuildEditor.AddSpell(spell)
            end
        end
    end
    
    if ShouldShowMysticEnchantSection() then
        local enchant = BuildCreatorUtil.CreateEnchantInfo(0) -- reused
        local isHero = C_Player:IsHero()
        for spellID, stacks in pairs(MysticEnchantUtil.GetAppliedEnchants("player")) do
            local enchantInfo = C_MysticEnchant.GetEnchantInfoBySpell(spellID)
            enchant.Enchant = spellID
            enchant.Stacks = stacks
            enchant.Level = isHero and 1 or enchantInfo.RequiredLevel
            C_BuildEditor.AddRandomEnchant(enchant)
        end
    end
    
    self:UpdatePendingBuild(true)
end

Area52DraftArchetypes = {
{id=1,label="Tank / Melee",entries={
{id=19,name="Blade Warden"},
{id=20,name="Bulwark"},
{id=143,name="Carnage Incarnate"},
{id=22,name="Relentless"},
{id=21,name="Swashbuckler"},
{id=24,name="Vanguard Sentinel"},
}},
{id=2,label="Tank / Magic",entries={
{id=26,name="Demoniac"},
{id=31,name="Icebound"},
{id=25,name="Templar of Flame"},
}},
{id=3,label="Tank / Hybrid",entries={
{id=33,name="Beast of Nightmare"},
{id=29,name="Crimson Champion"},
{id=27,name="Mystic Knight"},
{id=32,name="Plague Bearer"},
{id=30,name="Stormguard"},
{id=34,name="Thunderpaw"},
}},
{id=5,label="Healer / Pure Healer",entries={
{id=2,name="Dawnkeeper"},
{id=3,name="Eternal Caretaker"},
{id=10,name="Grove Keeper"},
{id=12,name="Hierophant"},
{id=1,name="Naturalist"},
{id=6,name="Sacred Sentinel"},
{id=16,name="Steward of Eternity"},
{id=7,name="Tide Mender"},
}},
{id=6,label="Healer / Combat Healer",entries={
{id=13,name="Cryotherapist"},
{id=17,name="Fire Sage"},
{id=15,name="Grove Ranger"},
{id=14,name="Tree of Wrath"},
{id=18,name="Vampire Lord"},
}},
{id=9,label="Damage / Melee",entries={
{id=86,name="Defias Gunslinger"},
{id=78,name="Dragon Warrior"},
{id=129,name="Gutripper"},
{id=77,name="Heavyweight"},
{id=73,name="Hoplite"},
{id=79,name="Son of Gruul"},
{id=74,name="Warden"},
{id=141,name="Wind Rager"},
}},
{id=10,label="Damage / Ranged",entries={
{id=85,name="Bestial Ranger"},
{id=140,name="Dark Ranger"},
{id=139,name="Locust Ranger"},
{id=82,name="Marksman"},
{id=84,name="Paratrooper"},
}},
{id=11,label="Damage / Magic",entries={
{id=101,name="Ascendant"},
{id=97,name="Cryomancer"},
{id=90,name="Druid of the Eclipse"},
{id=88,name="Evoker"},
{id=144,name="Fel Apostate"},
{id=95,name="Shadowseer"},
{id=104,name="Stormcaller"},
}},
{id=12,label="Damage / Hybrid",entries={
{id=132,name="Crusader"},
{id=136,name="Divine Sentinel"},
{id=75,name="Druid of the Fang"},
{id=134,name="Firelord"},
{id=131,name="Hellcat"},
{id=80,name="Master of Shadows"},
{id=142,name="Storm Giant"},
{id=133,name="White Walker"},
}},
}

Area52DraftSelector = {}
local Selector = Area52DraftSelector
function Selector.CanEditFeatured()
    return C_AccountInfo and type(C_AccountInfo.GetGMLevel) == "function"
        and (tonumber(C_AccountInfo.GetGMLevel()) or 0) > 0
end
local byID = {}
for _, group in ipairs(Area52DraftArchetypes) do
    for _, entry in ipairs(group.entries) do byID[entry.id] = entry end
end
function Selector.Applies(build)
    return GetRealmName() == 'Area 52 - Free-Pick' and build and build.Category == Enum.BuildCategory.BuildDraft
end
function Selector.ReadID(description)
    local id = (description or ''):match('###AREA52_ARCHETYPE_ID\n(%d+)')
    return id and tonumber(id)
end
function Selector.Description(view, description)
    if GetRealmName() ~= 'Area 52 - Free-Pick' then return description end
    description = (description or ''):gsub('\n*###AREA52_ARCHETYPE_ID[^#]*','')
    if Selector.Applies(C_BuildEditor.GetPendingBuild()) and Area52DraftSelector.CanEditFeatured() and byID[view.Area52ArchetypeID] then
        return description .. '\n\n###AREA52_ARCHETYPE_ID\n' .. view.Area52ArchetypeID
    end
    return description
end
function Selector.Valid(view)
    local build = C_BuildEditor.GetPendingBuild()
    if not Selector.Applies(build) then return true end
    local entry = byID[view.Area52ArchetypeID]
    return Area52DraftSelector.CanEditFeatured() and entry and build.Name == entry.name
        and Selector.ReadID(build.Description) == entry.id
end
function Selector.Choose(view, id)
    if not Area52DraftSelector.CanEditFeatured() or not Selector.Applies(C_BuildEditor.GetPendingBuild()) or not byID[id] then return end
    view.Area52ArchetypeID = id
    view.MainFrame.Name:SetText(byID[id].name)
    C_BuildEditor.SetName(byID[id].name)
    view:UpdatePendingBuild()
    CloseDropDownMenus()
end
function Selector.Menu(view, level, menuList)
    if not Area52DraftSelector.CanEditFeatured() then return end
    if level == 1 then
        for _, group in ipairs(Area52DraftArchetypes) do
            local info = UIDropDownMenu_CreateInfo()
            info.text=group.label;info.hasArrow=true;info.notCheckable=true;info.menuList=group.id
            UIDropDownMenu_AddButton(info,level)
        end
    elseif level == 2 then
        for _, group in ipairs(Area52DraftArchetypes) do
            if group.id == menuList then
                for _, entry in ipairs(group.entries) do
                    local id=entry.id
                    local info=UIDropDownMenu_CreateInfo()
                    info.text=entry.name;info.checked=view.Area52ArchetypeID==id
                    info.func=function() Selector.Choose(view,id) end
                    UIDropDownMenu_AddButton(info,level)
                end
            end
        end
    end
end
function Selector.Before(view, deserialize)
    if GetRealmName() ~= 'Area 52 - Free-Pick' then return end
    local build=C_BuildEditor.GetPendingBuild()
    if deserialize or view.Area52LastBuildID ~= build.ID or view.Area52LastCategory ~= build.Category then
        view.Area52ArchetypeID=Selector.ReadID(build.Description)
    end
    view.Area52LastBuildID=build.ID;view.Area52LastCategory=build.Category
    if not Selector.Applies(build) then view.Area52ArchetypeID=nil end
end
function Selector.After(view)
    if GetRealmName() ~= 'Area 52 - Free-Pick' then return end
    local applies=Selector.Applies(C_BuildEditor.GetPendingBuild())
    view.MainFrame.Name:SetShown(not applies)
    if applies and not view.Area52ArchetypeDropdown then
        local dropdown=CreateFrame('Frame',view:GetName()..'Area52ArchetypeDropdown',view.MainFrame,'UIDropDownMenuTemplate2')
        dropdown:SetScale(0.8)
        dropdown:SetPoint('TOPLEFT',view.MainFrame.Name,'TOPLEFT',-16,4)
        UIDropDownMenu_SetWidth(dropdown,530)
        UIDropDownMenu_Initialize(dropdown,function(_,level,menuList) Selector.Menu(view,level,menuList) end)
        view.Area52ArchetypeDropdown=dropdown
    end
    if view.Area52ArchetypeDropdown then
        view.Area52ArchetypeDropdown:SetShown(applies)
        local dropdown=view.Area52ArchetypeDropdown
        local enabled=Area52DraftSelector.CanEditFeatured()
        local color=enabled and HIGHLIGHT_FONT_COLOR or GRAY_FONT_COLOR
        dropdown.Text:SetVertexColor(color.r,color.g,color.b)
        if enabled then dropdown.Button:Enable() else dropdown.Button:Disable() end
        dropdown.isDisabled=not enabled and 1 or nil
        local entry=byID[view.Area52ArchetypeID]
        UIDropDownMenu_SetText(view.Area52ArchetypeDropdown,entry and entry.name or 'Choose a character-creation archetype')
    end
    if applies and not Selector.Valid(view) then
        view.Controls.PublishBuild:Disable()
        view.Controls.PublishBuild.tooltipTitle='Select a character-creation archetype'
        view.Controls.PublishBuild.tooltipText=not Selector.CanEditFeatured()
            and 'Your account does not have Featured Build editing privileges.'
            or 'Select a character-creation archetype whose name matches this Featured Build.' 
    end
end
local originalUpdate=EditableBuildViewMixin.UpdatePendingBuild
function EditableBuildViewMixin:UpdatePendingBuild(deserialize)
    Selector.Before(self,deserialize)
    originalUpdate(self,deserialize)
    Selector.After(self)
end

local area52LibraryPreviousUpdate = EditableBuildViewMixin.UpdatePendingBuild
function EditableBuildViewMixin:UpdatePendingBuild(...)
    area52LibraryPreviousUpdate(self, ...)
    if Area52BuildLibrary then Area52BuildLibrary.AttachEditor(self) end
end

local reasons = {
    UNKNOWN="The build could not be validated. Reopen the editor and try again.",
    LOW_LEVEL="Your character has not reached the level required to publish builds.",
    NO_SPELLS="Add at least one ability or talent.",
    NO_WEAPON_TYPES="Select at least one weapon type.",
    NO_ARMOR_TYPES="Select at least one armor type.",
    NO_NAME="Enter a build name.", DEFAULT_NAME="Replace the default build name.",
    NAME_TOO_LONG="Shorten the build name.", NO_ICON="Choose a build icon.",
    DEFAULT_ICON="Replace the question-mark icon with a build icon.",
    ICON_TOO_LONG="Choose another build icon.",
    NO_DESCRIPTION="Add a build description.", DEFAULT_DESCRIPTION="Replace the default build description.",
    DESCRIPTION_TOO_LONG="Shorten the build description.",
    ALREADY_EXISTS="This build already exists. Open the saved build before editing it.",
    UNCOMMON_QUALITY_RESTRICTIONS="The build exceeds its uncommon rarity allowance.",
    RARE_QUALITY_RESTRICTIONS="The build exceeds its rare rarity allowance.",
    EPIC_QUALITY_RESTRICTIONS="The build exceeds its epic rarity allowance.",
    LEGENDARY_QUALITY_RESTRICTIONS="The build exceeds its legendary rarity allowance.",
    BAD_CATEGORY="This build category is not available to your character. Featured builds require a GM.",
    NOT_ENOUGH_AE_INVESTMENT="Spend more ability points for the selected endgame category, or select Leveling.",
    NOT_ENOUGH_TE_INVESTMENT="Spend more talent points for the selected endgame category, or select Leveling.",
    SPELL_COMMENT_TOO_LONG="Shorten an ability or talent comment.",
    RANDOM_ENCHANT_COMMENT_TOO_LONG="Shorten a Mystic Enchant comment.",
    WEAPON_TYPE_COMMENT_TOO_LONG="Shorten a weapon-type comment.",
    ARMOR_TYPE_COMMENT_TOO_LONG="Shorten an armor-type comment.",
    NAME_PROFANITY="The build name was rejected by the text filter.",
    DESCRIPTION_PROFANITY="The build description was rejected by the text filter.",
    SPELL_COMMENT_PROFANITY="An ability or talent comment was rejected by the text filter.",
    RANDOM_ENCHANT_COMMENT_PROFANITY="A Mystic Enchant comment was rejected by the text filter.",
    WEAPON_TYPE_COMMENT_PROFANITY="A weapon-type comment was rejected by the text filter.",
    ARMOR_TYPE_COMMENT_PROFANITY="An armor-type comment was rejected by the text filter.",
    MULTIPLE_ABILITY_TYPES="An ability has more than one role tag. Keep only one tag per ability.",
    NOT_FOUND="The saved build is no longer available. Refresh the build list.",
    NO_PERMISSION="You do not have permission to change this build. Make a personal copy first.",
    CLASS_MISMATCH="This build belongs to a different character class."
}
local spellReasons = {
    SET_SPELL_LEVEL_TOO_LOW="requires a later learning level",
    SET_SPELL_LEVEL_TOO_HIGH="is scheduled beyond the realm's level limit",
    SET_SPELL_LEVEL_BAD_RANK="has ranks scheduled out of order",
    SET_SPELL_LEVEL_BAD_SPELL="is not available in the advancement catalog",
    SET_SPELL_LEVEL_DOESNT_EXIST="is missing from the pending build"
}
local function progressionDetails()
    local lines={}
    local summary={levels=0,entries=0,same=0,unexplained=0,unavailable=0}
    local function add(text) if #lines<8 then lines[#lines+1]=text end end
    local ok,build=pcall(C_BuildEditor.GetPendingBuild)
    if not ok or type(build)~="table" then
        return {"Stage: reading the pending build (GetPendingBuild failed)."},summary
    end
    local spells=type(build.Spells)=="table" and build.Spells or {}
    local function location(index,spell)
        local name,rank=GetSpellInfo(spell.Spell)
        return string.format("Entry %d, level %s: %s%s [spell %s]",index,tostring(spell.Level),name or "Unknown spell",rank and rank~="" and (" ("..rank..")") or "",tostring(spell.Spell))
    end
    local foundAE,foundTE=false,false
    for level=1,80 do
        if type(C_BuildEditor.GetEssenceForLevel)~="function" then summary.unavailable=summary.unavailable+1;break end
        local success,_,ae,_,te=pcall(C_BuildEditor.GetEssenceForLevel,level)
        if success and type(ae)=="number" and type(te)=="number" then
            summary.levels=summary.levels+1
            if not foundAE and ae<0 then add(string.format("Level %d: short by %d ability points.",level,-ae));foundAE=true end
            if not foundTE and te<0 then add(string.format("Level %d: short by %d talent points.",level,-te));foundTE=true end
        else
            summary.unavailable=summary.unavailable+1
            add(string.format("Stage: point budget at level %d could not be read.",level))
        end
    end
    for index,spell in ipairs(spells) do
        if index>512 then add("Entry scan stopped at the 512-entry diagnostic limit.");break end
        if type(spell)~="table" or type(spell.Spell)~="number" or type(spell.Level)~="number" then
            add(string.format("Entry %d: missing numeric spell ID or learning level.",index))
        elseif type(C_BuildEditor.CanSetSpellLevel)=="function" then
            summary.entries=summary.entries+1
            local success,allowed,errors=pcall(C_BuildEditor.CanSetSpellLevel,spell.Spell,spell.Level)
            if not success then
                summary.unavailable=summary.unavailable+1
                add(location(index,spell)..": CanSetSpellLevel could not be read.")
            else
                if type(errors)=="string" then errors={errors} end
                local meaningful,same=false,false
                if type(errors)=="table" then
                    for _,reason in ipairs(errors) do
                        if reason=="SET_SPELL_LEVEL_SAME" then same=true
                        elseif type(reason)=="string" then
                            meaningful=true
                            add(location(index,spell)..": "..(spellReasons[reason] or _G[reason] or reason)..".")
                        end
                    end
                end
                if same then summary.same=summary.same+1 end
                if (allowed==false or allowed==0) and not meaningful and not same then
                    summary.unexplained=summary.unexplained+1
                    add(location(index,spell)..": the level-check API rejected the request without a reason.")
                end
            end
        else summary.unavailable=summary.unavailable+1 end
    end
    return lines,summary
end
function Area52BuildValidationDetails()
    local lines,summary=progressionDetails()
    local header="This build cannot be learned in its current order. Save is disabled."
    local coverage=string.format("Diagnostic coverage: %d/80 level budgets, %d spell entries; %d unchanged-level responses, %d unavailable checks.",summary.levels,summary.entries,summary.same,summary.unavailable)
    if #lines==0 then
        return header.."\nNo specific failing entry was exposed by the point-budget or level-edit APIs. Unchanged-level responses do not validate progression.\n"..coverage
    end
    return header.."\n"..table.concat(lines,"\n").."\n"..coverage.."\nLevel-edit diagnostics locate reported issues; they do not reproduce the full Publish validator."
end
function Area52BuildPublishReason(reason)
    if type(reason)=="string" and reason:sub(1,22)=="AREA52_RARITY_MESSAGE:" then return reason:sub(23) end
    local featuredReasons={
        AREA52_BUILD_VALIDATION_UNAVAILABLE="The complete build validation could not finish or did not provide a failure reason. Save is disabled. Reopen the editor and try again.",
        AREA52_PUBLISH_ARCHETYPE_INVALID="Featured Draft assignment: select a character-creation archetype as a GM; its name and archetype ID must match.",
        AREA52_PUBLISH_CATEGORY_RESTORE_FAILED="Featured Draft validation could not restore the category. Close and reopen the editor before saving or publishing.",
        AREA52_PUBLISH_VALIDATION_UNAVAILABLE="Featured Draft progression validation could not complete. Nothing was published; reopen the editor and try again."
    }
    if featuredReasons[reason] then return featuredReasons[reason] end
    if GetRealmName()~="Area 52 - Free-Pick" then return _G[reason] or reason end
    if type(reason)=="string" and (reason:find("NON_TRAVERSIBLE",1,true) or reason:find("NOT_TRAVERSIBLE",1,true)) then return Area52BuildValidationDetails() end
    if reason=="PUBLISH_BUILD_NOT_TRAVERSIBLE" or reason=="PUBLISH_BUILD_LEVEL_INAPPROPRIATE_SPELL" or reason=="BUILD_NOT_TRAVERSIBLE" or reason=="BUIL_NOT_TRAVERSIBLE" then
        return Area52BuildValidationDetails()
    end
    local key=type(reason)=="string" and reason:match("^PUBLISH_BUILD_(.+)$")
    return reasons[key] or _G[reason] or "The build did not pass validation. Review its selections and try again."
end

-- Area 52 Featured Drafts use the same authored leveling progression as Leveling builds.
-- Keep the real Draft category/name/assignment for saving and character creation.
local function Area52BuildRarityReasons()
    local build=C_BuildEditor.GetPendingBuild()
    if type(build)~="table" or type(build.Spells)~="table" then error("missing build") end
    local names={"Uncommon","Rare","Epic","Legendary"}
    local limits, entries, picks, firstFailure={},{},{},{}
    for _,name in ipairs(names) do
        local quality=Enum.SpellQuality[name]
        local limit=quality and C_CharacterAdvancement.GetQualityLimit(quality)
        if type(limit)~="number" or limit<0 or limit%1~=0 then error("missing rarity budget") end
        limits[name]=limit
    end
    for _,spell in ipairs(build.Spells) do
        local entry=C_CharacterAdvancement.GetEntryBySpellID(spell.Spell)
        local level=tonumber(spell.Level)
        if not entry or not entry.ID or not level or level<1 or level>80 or level%1~=0 then error("missing selection") end
        local cost=entry.QualityCost
        if type(cost)~="number" or cost<0 or cost%1~=0 then error("missing rarity cost") end
        entries[entry.ID]=entry
        picks[entry.ID]=math.min(picks[entry.ID] or level,level)
    end
    for level=1,80 do
        local used={Uncommon=0,Rare=0,Epic=0,Legendary=0}
        for id,entry in pairs(entries) do
            if picks[id]<=level then
                local exempt=false
                for _,masteryID in ipairs(entry.Masteries or {}) do
                    if picks[masteryID] and picks[masteryID]<=level then exempt=true;break end
                end
                if not exempt and entry.QualityCost>0 then
                    local matched=false
                    for _,name in ipairs(names) do
                        if entry.Quality==name or entry.Quality==Enum.SpellQuality[name] then
                            used[name]=used[name]+entry.QualityCost;matched=true;break
                        end
                    end
                    if not matched and entry.Quality~="Common" and entry.Quality~="Normal" and entry.Quality~="None"
                        and entry.Quality~=0 and not (type(entry.Quality)=="number" and entry.Quality==Enum.SpellQuality.Normal) then
                        error("unknown rarity")
                    end
                end
            end
        end
        for _,name in ipairs(names) do
            if used[name]>limits[name] and not firstFailure[name] then
                firstFailure[name]=string.format("AREA52_RARITY_MESSAGE:%s rarity gem budget exceeded at level %d: %d used / %d allowed (%d over).",
                    name,level,used[name],limits[name],used[name]-limits[name])
            end
        end
    end
    local reasons={}
    for _,name in ipairs(names) do if firstFailure[name] then reasons[#reasons+1]=firstFailure[name] end end
    return #reasons==0,reasons
end

function Area52ValidateBuildRarity()
    if GetRealmName()~="Area 52 - Free-Pick" then return true,{} end
    local ok,allowed,reasons=pcall(Area52BuildRarityReasons)
    if not ok then return false,{"AREA52_RARITY_MESSAGE:Unable to validate rarity gem budgets. Reopen the editor before saving."} end
    return allowed,reasons
end

local function Area52ValidateBuildInternal(view)
    if view.Area52CategoryRestoreFailed then return false,{"AREA52_PUBLISH_CATEGORY_RESTORE_FAILED"} end
    local rarityOK,rarityReasons=Area52ValidateBuildRarity()
    if not rarityOK then return false,rarityReasons end
    local build=C_BuildEditor.GetPendingBuild()
    local selector=Area52DraftSelector
    if not selector or not selector.Applies(build) then return C_BuildEditor.CanPublishBuild() end
    if not selector.Valid(view) then return false,{"AREA52_PUBLISH_ARCHETYPE_INVALID"} end
    local allowed,reasons=C_BuildEditor.CanPublishBuild()
    if allowed==true or allowed==1 then return allowed,reasons end
    -- Do not suppress metadata, ownership, quality, or other failures.
    if type(reasons)~="table" or #reasons~=1 or reasons[1]~="PUBLISH_BUILD_NOT_TRAVERSIBLE" then return allowed,reasons end
    local category=build.Category
    local ok,result,details=pcall(function()
        C_BuildEditor.SetCategory(Enum.BuildCategory.Leveling)
        if C_BuildEditor.GetPendingBuild().Category~=Enum.BuildCategory.Leveling then error("category projection failed") end
        return C_BuildEditor.CanPublishBuild()
    end)
    local restored=pcall(C_BuildEditor.SetCategory,category)
    local readable,current=pcall(C_BuildEditor.GetPendingBuild)
    if not restored or not readable or not current or current.Category~=category then
        view.Area52CategoryRestoreFailed=true
        return false,{"AREA52_PUBLISH_CATEGORY_RESTORE_FAILED"}
    end
    view.Area52CategoryRestoreFailed=nil
    if not ok then return false,{"AREA52_PUBLISH_VALIDATION_UNAVAILABLE"} end
    if not selector.Valid(view) then return false,{"AREA52_PUBLISH_ARCHETYPE_INVALID"} end
    return result,details
end

function Area52CanPublishBuild(view)
    local ok,allowed,reasons=pcall(Area52ValidateBuildInternal,view)
    if not ok then return false,{"AREA52_BUILD_VALIDATION_UNAVAILABLE"} end
    if type(reasons)=="string" then reasons={reasons} end
    local failures={}
    if type(reasons)=="table" then
        for _,reason in ipairs(reasons) do
            if type(reason)=="string" and reason~="" then failures[#failures+1]=reason end
        end
    end
    if (allowed==true or allowed==1) and #failures==0 then return true,{} end
    if #failures==0 then failures={"AREA52_BUILD_VALIDATION_UNAVAILABLE"} end
    return false,failures
end

Area52FeaturedEditor = {}
local F = Area52FeaturedEditor
F.Slots = {
    {"Head",25,1,1}, {"Shoulders",10,3,2}, {"Chest",13,5,4},
    {"Gloves",16,10,8}, {"Boots",28,8,11}, {"Bracers",36,9,7},
    {"Belt",40,6,9}, {"Back",45,16,3}, {"Legs",50,7,10},
    {"Main hand",22,0,13}, {"Off hand / shield",22,0,14}, {"Ranged",22,0,12},
    {"Ground mount",20}, {"Flying mount",60},
}
BUILDCREATOR_SECTION_AI_BOT_ROTATION = "AI Bot Rotation"
BUILDCREATOR_SECTION_AREA52_FEATURED_VANITY = "Featured Vanity Rewards"
BuildCreatorUtil.DescriptionSection.AIBotRotation = "AI_BOT_ROTATION"
BuildCreatorUtil.DescriptionSectionInverted.AI_BOT_ROTATION = "AIBotRotation"

function F.Applies()
    return Area52DraftSelector.CanEditFeatured() and Area52DraftSelector.Applies(C_BuildEditor.GetPendingBuild())
end

function F.Read(description)
    local items = {}
    for i=1,14 do items[i]=0 end
    local raw = (description or ""):match("###AREA52_FEATURED_VANITY\n([^\n]*)")
    if not raw then return items end
    local _,count=(description or ""):gsub("###AREA52_FEATURED_VANITY\n","")
    if count~=1 then return nil end
    if not raw:match("^1:[%d,]+$") then return nil end
    local index = 0
    for token in (raw:sub(3)..","):gmatch("([^,]*),") do
        index=index+1
        local id=tonumber(token)
        if index>14 or not id or id<0 or id>4294967295 then return nil end
        items[index]=id
    end
    return index==14 and items or nil
end

function F.Accepts(slot, item)
    if not item or item<=0 then return false end
    if slot>=13 then
        return F.MountKinds and (F.MountKinds[item] == "both" or F.MountKinds[item] == (slot==14 and "flying" or "ground"))
    end
    local kind=GetItemInventoryType(item)
    if slot<=9 then return kind==F.Slots[slot][3] or (slot==3 and kind==20) end
    if slot==10 then return kind==13 or kind==17 or kind==21 end
    if slot==11 then return kind==13 or kind==14 or kind==22 or kind==23 end
    return kind==15 or kind==25 or kind==26
end

function F.Message(message)
    UIErrorsFrame:AddMessage(message,1,0.3,0.2)
end

function F.ImportAssigned(view,replace)
    if not F.Applies() or not view.Area52ArchetypeID or view.Area52FeaturedInvalid then return end
    if view.Area52ImportCancel then view.Area52ImportCancel();view.Area52ImportCancel=nil end
    view.Area52ImportPending=false
    local serial=(view.Area52ImportSerial or 0)+1
    view.Area52ImportSerial=serial
    local id=view.Area52ArchetypeID
    local context={view=view,id=C_BuildEditor.GetPendingBuild().ID,archetype=id,generation=view.Area52FeaturedGeneration}
    local original={unpack(view.Area52FeaturedItems)}
    local candidates,seen={},{}
    local function add(item)
        if type(item)=="number" and item>0 and not seen[item] then
            seen[item]=true;candidates[#candidates+1]=item
        end
    end
    local appearances=C_ItemSet.GetAppearances(90000+id) or {}
    local keys={}
    for key in pairs(appearances) do keys[#keys+1]=key end
    table.sort(keys)
    for _,key in ipairs(keys) do
        local appearance=appearances[key]
        if appearance and appearance~=0 then
            local kind,item=C_Appearance.GetAppearanceDisplayInfo(appearance)
            if kind=="APPEARANCE_DISPLAY_TYPE_ITEM" then add(item) end
        end
    end
    local bundle=C_VanityCollection.GetItem(9901000+id)
    local bundleItems={}
    for _,item in pairs(bundle and bundle.contentsPreview or {}) do
        if type(item)=="number" then bundleItems[#bundleItems+1]=item end
    end
    table.sort(bundleItems)
    for _,item in ipairs(bundleItems) do add(item) end
    if #candidates==0 then F.Message("The archetype catalog has no available items. Nothing was changed.");return end
    local function apply()
        if view.Area52ImportSerial~=serial then return end
        view.Area52ImportPending=false
        if view.Area52FeaturedInvalid or not F.ValidContext(context) then return end
        view.Area52ImportCancel=nil
        for slot=1,14 do
            if original[slot]~=view.Area52FeaturedItems[slot] then
                F.Message("Archetype loading was cancelled because your selections changed.");return
            end
        end
        for _,item in ipairs(candidates) do
            if not GetItemInfo(item) then
                F.Message("Some archetype items have not loaded. Try Load archetype set again.");return
            end
        end
        local assigned=F.Read("")
        for _,item in ipairs(candidates) do
            for slot=1,14 do
                if assigned[slot]==0 and F.Accepts(slot,item) then assigned[slot]=item;break end
            end
        end
        for slot=1,14 do
            if replace or view.Area52FeaturedItems[slot]==0 then view.Area52FeaturedItems[slot]=assigned[slot] end
        end
        if view.Area52FeaturedItems[10]>0 and GetItemInventoryType(view.Area52FeaturedItems[10])==17 then view.Area52FeaturedItems[11]=0 end
        view.Area52FeaturedImported=id
        F.Refresh(view)
        view:UpdatePendingBuild()
    end
    local missing={}
    for _,item in ipairs(candidates) do if not GetItemInfo(item) then missing[#missing+1]=item end end
    if #missing==0 then apply()
    else
        view.Area52ImportPending=true
        view.Area52ImportCancel=ItemQueryListener:AddCancelableCallback(missing,apply)
    end
end

function F.SlotTooltip(view,slot,button)
    local item=view.Area52FeaturedItems and view.Area52FeaturedItems[slot] or 0
    GameTooltip:SetOwner(button,"ANCHOR_RIGHT")
    if item>0 then GameTooltip:SetHyperlink("item:"..item)
    else GameTooltip:SetText(F.Slots[slot][1]..": Not assigned") end
    GameTooltip:AddLine("Featured reward at level "..F.Slots[slot][2],1,0.82,0)
    GameTooltip:AddLine("Left-click: choose. Right-click: clear.",0.8,0.8,0.8)
    GameTooltip:Show()
end

function F.ValidContext(context)
    local build=C_BuildEditor.GetPendingBuild()
    return context and Area52DraftSelector.CanEditFeatured() and Area52DraftSelector.Applies(build)
        and context.view.Area52FeaturedGeneration==context.generation
        and context.id==build.ID and context.archetype==context.view.Area52ArchetypeID
end

function F.ClosePicker(returnToEditor)
    local context=F.picker
    F.picker=nil
    if F.PickerBar then F.PickerBar:Hide() end
    if context and context.wardrobe and AppearanceWardrobeFrame then
        AppearanceWardrobeFrame.Collection.SearchBox:SetText(context.search or "")
        AppearanceWardrobeFrame.Collection:RefreshCategory()
    end
    if context and context.store then
        StoreCollectionFrame.SearchBox:SetText(context.search or "")
        StoreCollectionFrame:RefreshFeaturedPicker()
    end
    if returnToEditor then Collections:GoToTab(Collections.Tabs.HeroArchitect) end
end

function F.Choose(items)
    local context=F.picker
    if not F.ValidContext(context) then F.ClosePicker(false); return false end
    local view=context.view
    view.Area52FeaturedItems=items
    F.ClosePicker(true)
    F.Refresh(view)
    view:UpdatePendingBuild()
    return true
end

function F.PickAppearance(appearance)
    local context=F.picker
    if not context then return false end
    if not F.ValidContext(context) then F.ClosePicker(false); return true end
    if context.slot>=13 then F.Message("Choose a mount from the Vanity tab."); return true end
    if not appearance or appearance==0 then return true end
    local displayType,id=C_Appearance.GetAppearanceDisplayInfo(appearance)
    local items={unpack(context.view.Area52FeaturedItems)}
    if context.slot==0 then
        if displayType~="APPEARANCE_DISPLAY_TYPE_ITEM_SET" then F.Message("Choose an armor set from the Wardrobe Sets category."); return true end
        local replacements={}
        for _,member in pairs(C_ItemSet.GetAppearances(id) or {}) do
            local kind,item
            if member and member~=0 then kind,item=C_Appearance.GetAppearanceDisplayInfo(member) end
            if kind=="APPEARANCE_DISPLAY_TYPE_ITEM" then
                for slot=1,9 do
                    if F.Accepts(slot,item) then
                        if replacements[slot] and replacements[slot]~=item then F.Message("This set contains multiple choices for one slot. Select its pieces individually."); return true end
                        replacements[slot]=item
                    end
                end
            end
        end
        if not next(replacements) then F.Message("This set has no supported armor pieces."); return true end
        for slot=1,9 do items[slot]=replacements[slot] or 0 end
    else
        if displayType~="APPEARANCE_DISPLAY_TYPE_ITEM" or not F.Accepts(context.slot,id) then
            F.Message("Choose an item for "..F.Slots[context.slot][1].."."); return true
        end
        items[context.slot]=id
        if context.slot==10 and GetItemInventoryType(id)==17 then items[11]=0 end
        if context.slot==11 and items[10]>0 and GetItemInventoryType(items[10])==17 then
            F.Message("Clear the two-handed weapon before selecting an off-hand item."); return true
        end
    end
    F.Choose(items)
    return true
end

function F.PickMount(item)
    local context=F.picker
    if not context then return false end
    if not F.ValidContext(context) then F.ClosePicker(false); return true end
    if context.slot<13 or not F.Accepts(context.slot,item) then
        F.Message("Choose a supported "..(context.slot==14 and "flying" or "ground").." mount. This item cannot be assigned to that slot."); return true
    end
    local items={unpack(context.view.Area52FeaturedItems)}
    items[context.slot]=item
    F.Choose(items)
    return true
end

local function Button(parent,text,width,click)
    local button=CreateFrame("Button",nil,parent,"UIPanelButtonTemplate")
    button:SetSize(width,24); button:SetText(text); button:SetScript("OnClick",click)
    return button
end

function F.OpenPicker(view,slot)
    if not F.Applies() then return end
    F.ClosePicker(false)
    local build=C_BuildEditor.GetPendingBuild()
    local context={view=view,slot=slot,id=build.ID,archetype=view.Area52ArchetypeID,generation=view.Area52FeaturedGeneration}
    if slot>=13 then
        VanityCollection_LoadUI()
        Collections:GoToTab(Collections.Tabs.Vanity)
        context.store=true;context.search=StoreCollectionFrame.SearchBox:GetText()
        StaticPopup_Hide("VANITY_PURCHASE");StaticPopup_Hide("VANITY_PURCHASE_MULTI")
    else
        AppearanceUI_LoadUI()
        Collections:GoToTab(Collections.Tabs.Wardrobe)
        context.wardrobe=true;context.search=AppearanceWardrobeFrame.Collection.SearchBox:GetText()
    end
    F.picker=context
    if slot>=13 then
        StoreCollectionFrame.SearchBox:SetText("");StoreCollectionFrame:RefreshFeaturedPicker()
    else
        if slot==0 then AppearanceUI.OpenToItemSet("")
        else AppearanceUI.OpenCategory("APPEARANCE_TYPE_ITEM",F.Slots[slot][4]); AppearanceWardrobeFrame.Collection.SearchBox:SetText("") end
        AppearanceWardrobeFrame.Collection:RefreshCategory()
    end
    if not F.PickerBar then
        local bar=CreateFrame("Frame",nil,Collections)
        bar:SetSize(650,40);bar:SetPoint("TOP",Collections,"TOP",0,-35);bar:SetFrameStrata("DIALOG")
        bar.Text=bar:CreateFontString(nil,"OVERLAY","GameFontNormal")
        bar.Text:SetPoint("LEFT",0,0)
        bar.Cancel=Button(bar,"Cancel selection",140,function() F.ClosePicker(true) end)
        bar.Cancel:SetPoint("RIGHT",0,0)
        bar:SetScript("OnUpdate",function(self,elapsed)
            self.checkElapsed=(self.checkElapsed or 0)+elapsed
            if self.checkElapsed<0.25 then return end
            self.checkElapsed=0
            local context=F.picker
            if context and (not F.ValidContext(context) or not Collections:IsShown()
                or Collections:GetCurrentTabID()~=(context.store and Collections.Tabs.Vanity or Collections.Tabs.Wardrobe)) then F.ClosePicker(false) end
        end)
        Collections:HookScript("OnHide",function() F.ClosePicker(false) end)
        F.PickerBar=bar
    end
    F.PickerBar.Text:SetText("Assign to Featured Draft: "..(slot==0 and "Full armor set" or F.Slots[slot][1]))
    F.PickerBar:Show()
end

function F.Refresh(view,skipQuery)
    local panel=view.Area52VanitySection
    local items=view.Area52FeaturedItems
    if not panel or not items then return end
    if view.Area52PreviewCancel then view.Area52PreviewCancel();view.Area52PreviewCancel=nil end
    view.Area52PreviewSerial=(view.Area52PreviewSerial or 0)+1
    local serial=view.Area52PreviewSerial
    local missing={}
    AppearancePlayerModelMixin.Dress(panel.Model)
    for slot,definition in ipairs(F.Slots) do
        local id=items[slot] or 0
        local itemName=id>0 and GetItemInfo(id)
        if id>0 and not itemName then missing[#missing+1]=id end
        local name=id>0 and (itemName or ("Item "..id)) or "Not assigned"
        local button=panel.Buttons[slot]
        if id>0 then button:SetActiveItemDisplay(id)
        else
            local invSlot=AppearanceUtil.GetCategorySlotID(definition[4])
            local _,icon
            if invSlot then _,icon=GetInventorySlotInfoByID(invSlot) end
            button:SetInactiveIconDisplay(icon or "Interface\\Icons\\Ability_Mount_RidingHorse")
        end
        button.NoItemTexture:SetShown(id==0)
    end
    if not skipQuery and #missing>0 then
        view.Area52PreviewCancel=ItemQueryListener:AddCancelableCallback(missing,function()
            if view.Area52PreviewSerial~=serial then return end
            view.Area52PreviewCancel=nil
            F.Refresh(view,true)
        end)
    end
end

function F.Ensure(view)
    BuildViewSectionMixin.CategoryIcons.AI_BOT_ROTATION = "Interface\\Icons\\inv_gizmo_02"
    BuildViewSectionMixin.CategoryIcons.AREA52_FEATURED_VANITY = "Interface\\Icons\\inv_chest_chain"
    if view.Area52VanitySection then return end
    AppearanceUI_LoadUI()
    local content=view.MainFrame.ScrollFrame.Content
    local ai=CreateFrame("Frame",view:GetName().."Area52AI",content,"EditableBuildViewSectionTemplate")
    ai:SetSize(764,190);ai:SetCategory("AI_BOT_ROTATION");ai.Area52FeaturedOnly=true
    ai.Text=CreateFrame("Button",ai:GetName().."Text",ai,"EditableBuildMultilineTextTemplate")
    ai.Text:SetSize(648,128);ai.Text:SetPoint("TOPLEFT",58,-36);ai.Text:SetPoint("TOPRIGHT",-58,-36)
    local panel=CreateFrame("Frame",view:GetName().."Area52Vanity",content,"EditableBuildViewSectionTemplate")
    local scroll=view.MainFrame.ScrollFrame
    local scale=math.min(1,math.max(0.25,(scroll:GetHeight()-76)/538))
    panel:SetSize(764,76+538*scale);panel:SetCategory("AREA52_FEATURED_VANITY");panel.Area52FeaturedOnly=true
    panel.Serialize=function() return nil end;panel.Deserialize=function() end
    local canvas=CreateFrame("Frame",nil,scroll)
    canvas:SetSize(380,538);canvas:SetScale(scale);canvas:SetPoint("TOP",panel,"TOP",0,-44/scale)
    panel.Canvas=canvas
    panel.Model=CreateFrame("DressUpModel",nil,canvas)
    local model=panel.Model
    model:SetAllPoints(canvas)
    model.Background=model:CreateTexture(nil,"BACKGROUND");model.Background:SetAllPoints()
    model.Background:SetAtlas(AppearanceUtil.GetBackgroundAtlas())
    Mixin(model,ModelMixin)
    model:SetPosition(0,0.12,0)
    ModelMixin.OnLoad(model)
    model:SetEnableDragRotation(true);model:SetEnableScrollZoom(true);model:SetEnableDragMove(true)
    model:SetDragScale(0.001);model:SetMaxPosOffset(0.2,0.18);model:SetMinMaxDistance(-1,1)
    model:SetUnit("player");model:SetCamera(1)
    model.categoryID=1
    model.InternalUndress=AppearancePlayerModelMixin.InternalUndress
    model.GetSlotIllusionDisplayID=function() return 0 end
    model.GetSlotDisplayID=function(_,inventorySlot)
        for slot=1,12 do
            if AppearanceUtil.GetCategorySlotID(F.Slots[slot][4])==inventorySlot then
                local item=view.Area52FeaturedItems and view.Area52FeaturedItems[slot]
                return item and item>0 and item or nil
            end
        end
    end
    local elapsed=0
    panel:HookScript("OnHide",function() canvas:Hide() end)
    panel:HookScript("OnUpdate",function(self,delta)
        elapsed=elapsed+delta;if elapsed<0.05 then return end;elapsed=0
        local top,bottom=scroll:GetTop(),scroll:GetBottom()
        local a,b=canvas:GetTop(),canvas:GetBottom()
        local visible=false
        if panel:IsShown() and top and bottom and a and b then
            local outerScale,innerScale=scroll:GetEffectiveScale(),canvas:GetEffectiveScale()
            visible=a*innerScale<=top*outerScale+1 and b*innerScale>=bottom*outerScale-1
        end
        canvas:SetShown(visible)
    end)
    panel.Buttons={}
    for slot=1,14 do
        local index=slot
        local button=CreateFrame("CheckButton",nil,canvas,"AppearanceCategorySlotButtonTemplate")
        button:SetFrameLevel(model:GetFrameLevel()+10)
        button:UnregisterAllEvents()
        button:SetScript("OnShow",function(self) self.AnimFrame:Hide() end)
        button:SetScript("OnEvent",nil)
        button:SetScript("OnHide",function() GameTooltip:Hide() end)
        local layout={{"TOPLEFT",12,-56},{"TOPLEFT",12,-109},{"TOPLEFT",12,-215},{"TOPRIGHT",-12,-128},{"TOPRIGHT",-12,-287},{"TOPLEFT",12,-374},{"TOPRIGHT",-12,-181},{"TOPLEFT",12,-162},{"TOPRIGHT",-12,-234},{"BOTTOM",-53,28},{"BOTTOM",0,28},{"BOTTOM",53,28},{"TOPLEFT",402,-350},{"TOPLEFT",455,-350}}
        button:SetPoint(layout[slot][1],canvas,layout[slot][1],layout[slot][2],layout[slot][3])
        button:SetScript("OnClick",function(_,mouse)
            if not F.Applies() then return end
            if mouse=="RightButton" then
                view.Area52FeaturedItems[index]=0;F.Refresh(view);view:UpdatePendingBuild();GameTooltip:Hide()
            else model.IsRanged=(index==12);F.OpenPicker(view,index) end
        end)
        button:SetScript("OnEnter",function(self) F.SlotTooltip(view,index,self) end)
        button:SetScript("OnLeave",function() GameTooltip:Hide() end)
        local label=button:CreateFontString(nil,"OVERLAY","GameFontHighlightSmall")
        label:SetPoint("TOP",button,"BOTTOM",0,-2);label:SetText(tostring(F.Slots[slot][2]))
        panel.Buttons[slot]=button
    end
    local full=Button(panel,"Choose full armor set",210,function() F.OpenPicker(view,0) end)
    full:SetPoint("BOTTOMLEFT",panel,"BOTTOMLEFT",110,8)
    local assigned=Button(panel,"Load archetype set",180,function()
        F.ImportAssigned(view,true);view:UpdatePendingBuild()
    end)
    assigned:SetPoint("LEFT",full,"RIGHT",12,0)
    local help=panel:CreateFontString(nil,"OVERLAY","GameFontHighlightSmall")
    help:SetPoint("TOP",panel,"TOP",0,-28);help:SetText("Hover for item details. Left-click to choose; right-click to clear.")
    local mounts=canvas:CreateFontString(nil,"OVERLAY","GameFontHighlightSmall")
    mounts:SetPoint("TOPLEFT",canvas,"TOPLEFT",402,-324);mounts:SetText("Mounts")
    local ordered={}
    for _,section in ipairs(content.sections) do
        if section~=ai and section~=panel then
            ordered[#ordered+1]=section
            if section==content.Rotation then ordered[#ordered+1]=ai;ordered[#ordered+1]=panel end
        end
    end
    content.sections=ordered
    view.Area52AISection=ai;view.Area52VanitySection=panel
end

function F.Before(view,deserialize)
    if GetRealmName()~="Area 52 - Free-Pick" then return end
    F.Ensure(view)
    local build=C_BuildEditor.GetPendingBuild()
    local reload=deserialize or not view.Area52FeaturedItems or view.Area52FeaturedID~=build.ID
    if reload then
        view.Area52FeaturedGeneration=(view.Area52FeaturedGeneration or 0)+1
        view.Area52FeaturedItems=F.Read(build.Description)
        view.Area52FeaturedInvalid=not view.Area52FeaturedItems
        view.Area52FeaturedItems=view.Area52FeaturedItems or F.Read("")
        view.Area52FeaturedID=build.ID
        view.Area52FeaturedImported=tonumber((build.Description or ""):match("###AREA52_FEATURED_IMPORTED\n(%d+)"))
        F.Refresh(view)
    end
    local archetype
    if reload then archetype=Area52DraftSelector.ReadID(build.Description)
    else archetype=view.Area52ArchetypeID or Area52DraftSelector.ReadID(build.Description) end
    if archetype and F.Applies() and view.Area52FeaturedImported~=archetype then
        view.Area52ArchetypeID=archetype
        view.Area52DeferredImport=true
    end
    local enabled=F.Applies()
    view.Area52AISection:SetShown(enabled);view.Area52VanitySection:SetShown(enabled)
    view.Area52VanitySection.Canvas:SetShown(enabled and view:IsShown())
    local content=view.MainFrame.ScrollFrame.Content
    view.Area52AISection:ClearAllPoints();view.Area52AISection:SetPoint("TOPLEFT",content.Rotation,"BOTTOMLEFT",0,-16)
    view.Area52VanitySection:ClearAllPoints();view.Area52VanitySection:SetPoint("TOPLEFT",view.Area52AISection,"BOTTOMLEFT",0,-16)
    content.Consumables:ClearAllPoints()
    content.Consumables:SetPoint("TOPLEFT",enabled and view.Area52VanitySection or content.Rotation,"BOTTOMLEFT",0,-16)
end

local oldDescription=Area52DraftSelector.Description
function Area52DraftSelector.Description(view,description)
    description=(description or ""):gsub("\n*###AREA52_FEATURED_VANITY[^#]*","")
    description=description:gsub("\n*###AREA52_FEATURED_IMPORTED[^#]*","")
    if F.Applies() then description=description.."\n\n###AREA52_FEATURED_VANITY\n1:"..table.concat(view.Area52FeaturedItems,",") end
    if F.Applies() and view.Area52FeaturedImported then description=description.."\n\n###AREA52_FEATURED_IMPORTED\n"..view.Area52FeaturedImported end
    return oldDescription(view,description)
end
local oldUpdate=EditableBuildViewMixin.UpdatePendingBuild
function EditableBuildViewMixin:UpdatePendingBuild(deserialize)
    F.Before(self,deserialize)
    oldUpdate(self,deserialize)
    if F.Applies() and self.Area52FeaturedInvalid then
        for _,button in ipairs({self.Controls.SaveBuild,self.Controls.PublishBuild}) do
            button:Disable();button.tooltipTitle="Invalid Featured Vanity data"
            button.tooltipText="The saved reward metadata is malformed. Reopen an intact saved build before editing."
        end
    end
end

local oldValidation=Area52CanPublishBuild
function Area52CanPublishBuild(view)
    if F.Applies() then
        if view.Area52FeaturedInvalid then return false,{"Invalid Featured Vanity metadata. Reopen an intact saved build."} end
        local items=view.Area52FeaturedItems
        for slot=1,14 do
            if items and items[slot]~=0 and not F.Accepts(slot,items[slot]) then
                return false,{"Invalid "..F.Slots[slot][1].." reward. Choose another item or clear this slot."}
            end
        end
        if items and items[10]>0 and GetItemInventoryType(items[10])==17 and items[11]>0 then
            return false,{"A two-handed Vanity weapon cannot be combined with an off-hand reward."}
        end
        if #(C_BuildEditor.GetPendingBuild().Description or "")>8192 then
            return false,{"Build description and Featured fields exceed 8,192 bytes. Shorten the text before saving."}
        end
    end
    return oldValidation(view)
end

Area52FeaturedEditor.MountKinds = {
    [1041]="ground",
    [1132]="ground",
    [1133]="ground",
    [1134]="ground",
    [2411]="ground",
    [2413]="ground",
    [2414]="ground",
    [2415]="ground",
    [5655]="ground",
    [5656]="ground",
    [5663]="ground",
    [5665]="ground",
    [5668]="ground",
    [5864]="ground",
    [5872]="ground",
    [5873]="ground",
    [5874]="ground",
    [5875]="ground",
    [8563]="ground",
    [8583]="ground",
    [8586]="ground",
    [8588]="ground",
    [8589]="ground",
    [8590]="ground",
    [8591]="ground",
    [8592]="ground",
    [8595]="ground",
    [8627]="ground",
    [8628]="ground",
    [8629]="ground",
    [8630]="ground",
    [8631]="ground",
    [8632]="ground",
    [8633]="ground",
    [10732]="flying",
    [10733]="flying",
    [10734]="flying",
    [12145]="ground",
    [12146]="ground",
    [12147]="ground",
    [12302]="ground",
    [12303]="ground",
    [12325]="ground",
    [12326]="ground",
    [12327]="ground",
    [12330]="ground",
    [12351]="ground",
    [12353]="ground",
    [12354]="ground",
    [13086]="ground",
    [13317]="ground",
    [13321]="ground",
    [13322]="ground",
    [13323]="ground",
    [13324]="ground",
    [13325]="ground",
    [13326]="ground",
    [13327]="ground",
    [13328]="ground",
    [13329]="ground",
    [13331]="ground",
    [13332]="ground",
    [13333]="ground",
    [13334]="ground",
    [13335]="ground",
    [14062]="ground",
    [15277]="ground",
    [15290]="ground",
    [15292]="ground",
    [15293]="ground",
    [16339]="ground",
    [18241]="ground",
    [18242]="ground",
    [18243]="ground",
    [18244]="ground",
    [18245]="ground",
    [18246]="ground",
    [18247]="ground",
    [18248]="ground",
    [18766]="ground",
    [18767]="ground",
    [18768]="ground",
    [18772]="ground",
    [18773]="ground",
    [18774]="ground",
    [18776]="ground",
    [18777]="ground",
    [18778]="ground",
    [18785]="ground",
    [18786]="ground",
    [18787]="ground",
    [18788]="ground",
    [18789]="ground",
    [18790]="ground",
    [18791]="ground",
    [18793]="ground",
    [18794]="ground",
    [18795]="ground",
    [18796]="ground",
    [18797]="ground",
    [18798]="ground",
    [18902]="ground",
    [19029]="ground",
    [19030]="ground",
    [19872]="ground",
    [19902]="ground",
    [21044]="ground",
    [21176]="ground",
    [21218]="ground",
    [21321]="ground",
    [21323]="ground",
    [21324]="ground",
    [23193]="ground",
    [23720]="ground",
    [25470]="ground",
    [25471]="ground",
    [25472]="ground",
    [25473]="ground",
    [25474]="ground",
    [25475]="ground",
    [25476]="ground",
    [25477]="ground",
    [25527]="ground",
    [25528]="ground",
    [25529]="ground",
    [25531]="ground",
    [25532]="ground",
    [25533]="ground",
    [25596]="flying",
    [28481]="ground",
    [28482]="ground",
    [28915]="ground",
    [28927]="ground",
    [28936]="ground",
    [29102]="ground",
    [29103]="ground",
    [29104]="ground",
    [29105]="ground",
    [29220]="ground",
    [29221]="ground",
    [29222]="ground",
    [29223]="ground",
    [29224]="ground",
    [29225]="ground",
    [29227]="ground",
    [29228]="ground",
    [29229]="ground",
    [29230]="ground",
    [29231]="ground",
    [29465]="ground",
    [29466]="ground",
    [29467]="ground",
    [29468]="ground",
    [29469]="ground",
    [29470]="ground",
    [29471]="ground",
    [29472]="ground",
    [29743]="ground",
    [29744]="ground",
    [29745]="ground",
    [29746]="ground",
    [29747]="ground",
    [30480]="ground",
    [30609]="both",
    [31829]="ground",
    [31830]="ground",
    [31831]="ground",
    [31832]="ground",
    [31833]="ground",
    [31834]="ground",
    [31835]="ground",
    [31836]="ground",
    [32314]="both",
    [32316]="both",
    [32317]="both",
    [32318]="both",
    [32319]="both",
    [32458]="both",
    [32768]="ground",
    [32857]="both",
    [32858]="both",
    [32859]="both",
    [32860]="both",
    [32861]="both",
    [32862]="both",
    [33176]="flying",
    [33182]="flying",
    [33183]="ground",
    [33184]="flying",
    [33189]="ground",
    [33224]="ground",
    [33225]="ground",
    [33809]="ground",
    [33976]="ground",
    [33977]="ground",
    [33999]="both",
    [34060]="both",
    [34061]="both",
    [34092]="both",
    [34129]="ground",
    [35225]="ground",
    [35226]="ground",
    [35513]="ground",
    [35906]="ground",
    [37011]="ground",
    [37012]="both",
    [37598]="ground",
    [37676]="both",
    [37719]="ground",
    [37827]="ground",
    [37828]="ground",
    [38265]="ground",
    [38576]="ground",
    [40775]="ground",
    [40777]="ground",
    [41508]="ground",
    [43516]="both",
    [43599]="ground",
    [43951]="flying",
    [43952]="flying",
    [43953]="flying",
    [43954]="flying",
    [43955]="flying",
    [43956]="ground",
    [43958]="ground",
    [43959]="ground",
    [43961]="ground",
    [43962]="ground",
    [43963]="ground",
    [43964]="ground",
    [43986]="flying",
    [44077]="ground",
    [44080]="ground",
    [44083]="ground",
    [44086]="ground",
    [44151]="ground",
    [44160]="flying",
    [44164]="flying",
    [44168]="flying",
    [44175]="flying",
    [44177]="both",
    [44178]="both",
    [44221]="flying",
    [44223]="ground",
    [44224]="ground",
    [44225]="ground",
    [44226]="ground",
    [44229]="flying",
    [44230]="ground",
    [44231]="ground",
    [44234]="ground",
    [44235]="ground",
    [44413]="ground",
    [44554]="both",
    [44555]="ground",
    [44556]="ground",
    [44558]="ground",
    [44689]="flying",
    [44690]="flying",
    [44707]="flying",
    [44842]="both",
    [44843]="both",
    [45125]="ground",
    [45586]="ground",
    [45589]="ground",
    [45590]="ground",
    [45591]="ground",
    [45592]="ground",
    [45593]="ground",
    [45595]="ground",
    [45596]="ground",
    [45597]="ground",
    [45693]="flying",
    [45725]="flying",
    [45801]="flying",
    [45802]="flying",
    [46099]="ground",
    [46100]="ground",
    [46102]="ground",
    [46109]="ground",
    [46171]="flying",
    [46308]="ground",
    [46708]="flying",
    [46743]="ground",
    [46744]="ground",
    [46745]="ground",
    [46746]="ground",
    [46747]="ground",
    [46748]="ground",
    [46749]="ground",
    [46750]="ground",
    [46751]="ground",
    [46752]="ground",
    [46755]="ground",
    [46756]="ground",
    [46757]="ground",
    [46758]="ground",
    [46759]="ground",
    [46760]="ground",
    [46761]="ground",
    [46762]="ground",
    [46763]="ground",
    [46764]="ground",
    [46778]="ground",
    [46813]="flying",
    [46814]="flying",
    [46815]="ground",
    [46816]="ground",
    [47100]="ground",
    [47101]="ground",
    [47179]="ground",
    [47180]="ground",
    [47840]="flying",
    [49044]="ground",
    [49046]="ground",
    [49093]="ground",
    [49096]="ground",
    [49098]="ground",
    [49282]="ground",
    [49283]="ground",
    [49284]="ground",
    [49285]="ground",
    [49286]="ground",
    [49288]="ground",
    [49289]="ground",
    [49290]="ground",
    [49636]="both",
    [50250]="both",
    [50420]="ground",
    [50435]="flying",
    [50818]="ground",
    [51954]="flying",
    [51955]="flying",
    [52200]="ground",
    [54068]="ground",
    [54069]="ground",
    [54797]="flying",
    [54811]="both",
    [54860]="ground",
    [54862]="both",
    [54863]="both",
    [54864]="both",
    [61355]="ground",
    [69228]="ground",
    [71665]="ground",
    [71718]="ground",
    [74269]="ground",
    [79177]="ground",
    [79239]="ground",
    [79243]="ground",
    [79245]="ground",
    [79247]="ground",
    [79248]="ground",
    [85262]="ground",
    [87801]="ground",
    [89362]="ground",
    [89390]="ground",
    [89391]="ground",
    [89783]="ground",
    [91010]="ground",
    [91013]="ground",
    [91014]="ground",
    [91015]="ground",
    [97321]="both",
    [97389]="ground",
    [97390]="ground",
    [97391]="ground",
    [97392]="ground",
    [97856]="ground",
    [97857]="ground",
    [97907]="ground",
    [97908]="ground",
    [97909]="ground",
    [97910]="ground",
    [98075]="ground",
    [98467]="ground",
    [98468]="ground",
    [98470]="ground",
    [98471]="ground",
    [98472]="ground",
    [98473]="both",
    [98475]="ground",
    [98476]="ground",
    [98477]="ground",
    [98479]="ground",
    [98480]="ground",
    [98490]="ground",
    [99477]="ground",
    [99478]="both",
    [99479]="ground",
    [99480]="ground",
    [99481]="both",
    [99482]="both",
    [99483]="ground",
    [99484]="both",
    [99485]="both",
    [99489]="ground",
    [99490]="ground",
    [99982]="both",
    [100140]="both",
    [101227]="both",
    [101255]="ground",
    [101256]="ground",
    [102107]="both",
    [102125]="ground",
    [102126]="ground",
    [102127]="ground",
    [102128]="ground",
    [110411]="ground",
    [112456]="ground",
    [116788]="ground",
    [116789]="ground",
    [120306]="ground",
    [120307]="ground",
    [120308]="ground",
    [120599]="flying",
    [121502]="ground",
    [121503]="ground",
    [121504]="ground",
    [121505]="ground",
    [121815]="ground",
    [131024]="ground",
    [131025]="ground",
    [131026]="both",
    [134947]="ground",
    [155653]="ground",
    [155654]="ground",
    [155655]="both",
    [157489]="ground",
    [160592]="flying",
    [170000]="ground",
    [170001]="ground",
    [170002]="both",
    [170003]="ground",
    [170004]="ground",
    [170005]="ground",
    [170006]="ground",
    [170007]="ground",
    [170008]="ground",
    [170009]="ground",
    [173178]="both",
    [177567]="ground",
    [177568]="ground",
    [181260]="flying",
    [181261]="flying",
    [187322]="both",
    [187323]="both",
    [187324]="both",
    [187325]="both",
    [188777]="flying",
    [190197]="flying",
    [190198]="flying",
    [190199]="flying",
    [190200]="flying",
    [190201]="flying",
    [190301]="ground",
    [191541]="ground",
    [191542]="ground",
    [191543]="ground",
    [192632]="ground",
    [192633]="ground",
    [192634]="ground",
    [192635]="ground",
    [192636]="ground",
    [193373]="ground",
    [193374]="ground",
    [193375]="ground",
    [193376]="ground",
    [193377]="ground",
    [193698]="ground",
    [196728]="both",
    [199156]="ground",
    [199157]="ground",
    [199158]="ground",
    [199159]="ground",
    [199160]="ground",
    [199161]="ground",
    [199658]="ground",
    [199659]="ground",
    [199660]="ground",
    [199661]="ground",
    [199662]="ground",
    [199666]="ground",
    [199667]="flying",
    [199738]="flying",
    [199739]="flying",
    [199740]="flying",
    [203079]="both",
    [203080]="both",
    [203081]="both",
    [203082]="both",
    [205628]="flying",
    [205629]="flying",
    [213209]="ground",
    [218185]="ground",
    [218186]="ground",
    [218187]="ground",
    [219575]="ground",
    [223915]="flying",
    [225859]="ground",
    [225860]="ground",
    [225861]="ground",
    [225862]="ground",
    [233000]="ground",
    [233001]="ground",
    [233002]="ground",
    [233442]="ground",
    [233443]="ground",
    [233444]="ground",
    [233445]="ground",
    [236292]="ground",
    [236293]="ground",
    [236294]="ground",
    [236295]="ground",
    [236296]="ground",
    [236310]="ground",
    [236415]="both",
    [240140]="ground",
    [240141]="ground",
    [240142]="ground",
    [240143]="ground",
    [240144]="ground",
    [240145]="ground",
    [240146]="ground",
    [240147]="ground",
    [240545]="ground",
    [246192]="both",
    [249038]="ground",
    [250451]="ground",
    [250452]="ground",
    [250453]="ground",
    [250454]="ground",
    [250455]="ground",
    [250456]="ground",
    [250457]="ground",
    [250458]="ground",
    [250459]="ground",
    [250460]="ground",
    [250461]="ground",
    [250462]="ground",
    [252676]="both",
    [252677]="both",
    [252678]="both",
    [252679]="both",
    [252680]="ground",
    [252681]="ground",
    [254027]="both",
    [254028]="both",
    [259633]="ground",
    [259654]="ground",
    [259965]="ground",
    [274111]="ground",
    [274437]="flying",
    [274438]="flying",
    [274439]="flying",
    [274440]="flying",
    [274441]="flying",
    [274442]="flying",
    [278707]="ground",
    [283039]="flying",
    [283040]="flying",
    [283041]="flying",
    [283042]="flying",
    [287390]="ground",
    [287849]="flying",
    [290291]="ground",
    [291911]="flying",
    [291912]="flying",
    [291913]="flying",
    [291914]="flying",
    [291915]="flying",
    [291938]="flying",
    [291939]="flying",
    [291940]="flying",
    [291941]="flying",
    [291942]="flying",
    [291943]="flying",
    [291944]="flying",
    [291945]="flying",
    [291946]="flying",
    [291947]="flying",
    [305685]="both",
    [311698]="flying",
    [311699]="flying",
    [311700]="flying",
    [311701]="flying",
    [311702]="flying",
    [311703]="flying",
    [311704]="flying",
    [311705]="flying",
    [311706]="flying",
    [311707]="flying",
    [311711]="flying",
    [311712]="flying",
    [311713]="flying",
    [311714]="flying",
    [311715]="flying",
    [311716]="flying",
    [313692]="ground",
    [323660]="ground",
    [323661]="ground",
    [323662]="ground",
    [333265]="ground",
    [333534]="flying",
    [333535]="flying",
    [333536]="flying",
    [333998]="ground",
    [333999]="ground",
    [334410]="both",
    [334411]="both",
    [334412]="both",
    [334413]="both",
    [334414]="both",
    [334415]="both",
    [334416]="both",
    [335191]="flying",
    [335192]="flying",
    [335193]="flying",
    [338626]="ground",
    [338696]="ground",
    [339548]="ground",
    [343204]="both",
    [343205]="both",
    [343206]="both",
    [343207]="both",
    [343208]="both",
    [343209]="both",
    [343210]="both",
    [343211]="both",
    [348726]="ground",
    [348727]="ground",
    [348728]="ground",
    [348729]="ground",
    [348730]="ground",
    [348731]="both",
    [350183]="flying",
    [350184]="flying",
    [350185]="flying",
    [350186]="flying",
    [365954]="ground",
    [366988]="ground",
    [366989]="ground",
    [366990]="ground",
    [366991]="ground",
    [369008]="flying",
    [369009]="flying",
    [369010]="flying",
    [369011]="flying",
    [393445]="both",
    [393446]="ground",
    [393447]="ground",
    [393448]="ground",
    [393449]="flying",
    [393450]="flying",
    [398247]="flying",
    [398248]="flying",
    [398249]="flying",
    [400013]="flying",
    [400014]="flying",
    [400015]="ground",
    [400016]="flying",
    [400017]="flying",
    [400018]="flying",
    [400019]="flying",
    [400020]="flying",
    [400021]="flying",
    [400022]="flying",
    [400023]="flying",
    [400024]="ground",
    [400025]="ground",
    [400026]="ground",
    [400027]="ground",
    [400028]="ground",
    [400029]="ground",
    [400030]="ground",
    [400031]="ground",
    [400032]="flying",
    [400033]="flying",
    [400034]="flying",
    [400035]="ground",
    [400036]="ground",
    [400037]="ground",
    [400038]="ground",
    [400039]="flying",
    [400040]="flying",
    [400041]="flying",
    [400042]="flying",
    [400043]="flying",
    [400044]="flying",
    [400045]="ground",
    [400046]="flying",
    [400047]="flying",
    [400048]="ground",
    [400049]="ground",
    [400050]="ground",
    [400051]="flying",
    [400052]="flying",
    [400053]="flying",
    [400065]="flying",
    [400066]="flying",
    [400067]="flying",
    [400068]="flying",
    [400069]="flying",
    [400082]="ground",
    [400083]="ground",
    [400124]="ground",
    [400125]="ground",
    [400126]="flying",
    [400127]="flying",
    [400128]="flying",
    [400129]="flying",
    [400130]="flying",
    [400131]="flying",
    [400132]="ground",
    [400133]="ground",
    [400134]="ground",
    [400135]="ground",
    [400136]="ground",
    [400137]="ground",
    [400138]="ground",
    [400139]="ground",
    [400140]="ground",
    [400141]="ground",
    [400142]="ground",
    [400143]="flying",
    [400144]="flying",
    [400145]="flying",
    [400146]="flying",
    [400147]="flying",
    [400148]="flying",
    [400149]="flying",
    [400150]="flying",
    [400151]="flying",
    [400152]="flying",
    [400153]="flying",
    [400166]="flying",
    [400167]="flying",
    [400209]="flying",
    [400210]="flying",
    [400251]="flying",
    [400252]="flying",
    [400253]="flying",
    [400254]="flying",
    [400255]="flying",
    [400256]="flying",
    [400257]="flying",
    [400258]="ground",
    [400259]="ground",
    [400260]="ground",
    [400262]="ground",
    [400263]="ground",
    [400264]="ground",
    [400265]="ground",
    [400266]="ground",
    [400267]="ground",
    [400268]="ground",
    [400269]="ground",
    [400270]="ground",
    [400271]="ground",
    [400272]="ground",
    [400273]="ground",
    [400274]="ground",
    [400275]="ground",
    [400563]="flying",
    [416601]="ground",
    [416602]="ground",
    [420852]="flying",
    [428234]="ground",
    [428235]="ground",
    [428236]="ground",
    [428545]="both",
    [428546]="both",
    [439040]="ground",
    [448724]="ground",
    [448725]="ground",
    [448726]="both",
    [448727]="ground",
    [448728]="ground",
    [498840]="ground",
    [499181]="ground",
    [499203]="both",
    [499304]="both",
    [499307]="ground",
    [499308]="both",
    [499309]="both",
    [499363]="both",
    [499364]="both",
    [499367]="both",
    [499378]="both",
    [499379]="both",
    [499386]="both",
    [499387]="both",
    [499389]="both",
    [499391]="both",
    [499424]="both",
    [499425]="both",
    [499426]="both",
    [499427]="ground",
    [499438]="both",
    [499475]="ground",
    [499476]="ground",
    [499477]="ground",
    [499478]="ground",
    [499479]="ground",
    [499480]="ground",
    [499481]="ground",
    [499482]="ground",
    [499483]="ground",
    [499484]="ground",
    [499485]="ground",
    [499486]="ground",
    [499487]="ground",
    [499488]="ground",
    [499489]="ground",
    [499490]="ground",
    [499539]="ground",
    [499578]="ground",
    [499579]="ground",
    [499580]="ground",
    [499581]="ground",
    [499582]="ground",
    [499583]="both",
    [499636]="both",
    [499638]="both",
    [499639]="ground",
    [499659]="ground",
    [499660]="ground",
    [499661]="ground",
    [499662]="ground",
    [499663]="ground",
    [499664]="both",
    [499674]="ground",
    [499675]="ground",
    [499676]="ground",
    [499677]="ground",
    [499678]="ground",
    [499679]="ground",
    [499869]="ground",
    [499870]="both",
    [499871]="both",
    [499872]="both",
    [499873]="ground",
    [499874]="ground",
    [499875]="ground",
    [499876]="ground",
    [499877]="ground",
    [499878]="ground",
    [499879]="both",
    [499880]="ground",
    [499881]="ground",
    [499882]="ground",
    [499883]="ground",
    [499884]="ground",
    [499885]="ground",
    [499886]="ground",
    [499887]="ground",
    [499891]="ground",
    [499892]="ground",
    [499933]="both",
    [499937]="ground",
    [499940]="ground",
    [499942]="both",
    [499943]="ground",
    [499944]="ground",
    [499945]="ground",
    [499984]="ground",
    [499987]="ground",
    [499990]="both",
    [499995]="ground",
    [499998]="ground",
    [500002]="both",
    [500005]="both",
    [500006]="flying",
    [500648]="ground",
    [500649]="ground",
    [500650]="ground",
    [500651]="ground",
    [500652]="ground",
    [501000]="ground",
    [509884]="ground",
    [509902]="ground",
    [510004]="both",
    [510006]="both",
    [520006]="flying",
    [532580]="ground",
    [532581]="ground",
    [532582]="ground",
    [546725]="flying",
    [546726]="flying",
    [546727]="flying",
    [546728]="flying",
    [558600]="both",
    [558601]="ground",
    [558602]="ground",
    [558603]="ground",
    [558604]="ground",
    [558605]="ground",
    [558606]="ground",
    [558607]="ground",
    [558608]="ground",
    [558609]="ground",
    [558610]="ground",
    [558611]="both",
    [558612]="both",
    [558613]="both",
    [558614]="both",
    [558615]="both",
    [558616]="both",
    [558617]="both",
    [558618]="both",
    [558619]="both",
    [558620]="ground",
    [558621]="ground",
    [558622]="ground",
    [558623]="ground",
    [558624]="both",
    [558625]="both",
    [558626]="both",
    [558627]="both",
    [558628]="both",
    [558629]="both",
    [558630]="both",
    [558631]="both",
    [558632]="both",
    [558633]="both",
    [558634]="both",
    [558635]="ground",
    [558636]="ground",
    [558637]="ground",
    [558638]="both",
    [558639]="both",
    [558640]="both",
    [558641]="ground",
    [558642]="ground",
    [558643]="ground",
    [558644]="ground",
    [558645]="ground",
    [558646]="ground",
    [558647]="ground",
    [558648]="ground",
    [558649]="ground",
    [558650]="ground",
    [558651]="ground",
    [558652]="both",
    [558653]="both",
    [558654]="both",
    [558655]="both",
    [558656]="both",
    [558657]="both",
    [558658]="both",
    [558659]="ground",
    [558660]="ground",
    [558661]="ground",
    [558662]="ground",
    [558663]="ground",
    [558664]="ground",
    [558665]="ground",
    [558666]="ground",
    [558667]="ground",
    [558668]="ground",
    [558669]="ground",
    [558670]="ground",
    [558671]="ground",
    [558672]="ground",
    [558673]="ground",
    [558674]="ground",
    [558675]="ground",
    [558676]="ground",
    [558677]="ground",
    [558678]="ground",
    [558749]="both",
    [558750]="both",
    [558751]="both",
    [558752]="both",
    [558797]="both",
    [558798]="both",
    [558819]="ground",
    [558822]="ground",
    [559202]="ground",
    [559203]="ground",
    [559205]="ground",
    [559206]="ground",
    [559366]="ground",
    [559367]="ground",
    [559368]="ground",
    [559369]="both",
    [559370]="both",
    [559371]="both",
    [559400]="ground",
    [559401]="ground",
    [559402]="ground",
    [559403]="ground",
    [559404]="ground",
    [559405]="ground",
    [559406]="ground",
    [559411]="ground",
    [559412]="ground",
    [559413]="ground",
    [559414]="ground",
    [559415]="ground",
    [559416]="ground",
    [559417]="ground",
    [559418]="ground",
    [559419]="ground",
    [559420]="ground",
    [559421]="both",
    [559422]="both",
    [559423]="both",
    [559424]="both",
    [559425]="ground",
    [559426]="ground",
    [559427]="both",
    [559428]="both",
    [559429]="both",
    [559430]="ground",
    [559431]="ground",
    [559432]="both",
    [559433]="both",
    [559434]="both",
    [559435]="both",
    [559436]="both",
    [559437]="both",
    [559438]="both",
    [559439]="both",
    [559440]="both",
    [559441]="both",
    [559442]="both",
    [559443]="both",
    [559444]="both",
    [559445]="both",
    [559446]="both",
    [559447]="both",
    [559448]="both",
    [559449]="ground",
    [559450]="ground",
    [559451]="ground",
    [559452]="ground",
    [559453]="both",
    [559454]="both",
    [559455]="both",
    [559456]="both",
    [559457]="both",
    [559519]="both",
    [559520]="both",
    [559542]="ground",
    [559546]="ground",
    [559547]="ground",
    [559548]="ground",
    [559549]="ground",
    [559550]="ground",
    [559551]="ground",
    [559579]="ground",
    [559580]="ground",
    [559581]="ground",
    [559582]="ground",
    [559583]="ground",
    [559584]="ground",
    [559585]="both",
    [559589]="both",
    [559590]="both",
    [559591]="both",
    [559592]="both",
    [559593]="both",
    [559594]="both",
    [559595]="both",
    [559598]="ground",
    [559599]="both",
    [559774]="ground",
    [559775]="ground",
    [559776]="ground",
    [559777]="ground",
    [559778]="ground",
    [559779]="ground",
    [559780]="ground",
    [559781]="ground",
    [559782]="ground",
    [559783]="ground",
    [559784]="ground",
    [559785]="ground",
    [559786]="ground",
    [559787]="ground",
    [559788]="ground",
    [559789]="ground",
    [559790]="ground",
    [559791]="ground",
    [559792]="ground",
    [559793]="ground",
    [559794]="ground",
    [559795]="ground",
    [559796]="ground",
    [559797]="ground",
    [559798]="ground",
    [559799]="ground",
    [559800]="ground",
    [559801]="ground",
    [559802]="ground",
    [559803]="ground",
    [559804]="ground",
    [559806]="ground",
    [559807]="ground",
    [559808]="ground",
    [559809]="ground",
    [559810]="ground",
    [559811]="ground",
    [559812]="ground",
    [559813]="ground",
    [559814]="ground",
    [559815]="ground",
    [559816]="ground",
    [559817]="ground",
    [559818]="ground",
    [559819]="ground",
    [559820]="ground",
    [559821]="ground",
    [559822]="ground",
    [559823]="ground",
    [559824]="ground",
    [559825]="ground",
    [559826]="ground",
    [559827]="ground",
    [559828]="ground",
    [559829]="ground",
    [559830]="ground",
    [559831]="ground",
    [559832]="ground",
    [559833]="ground",
    [559834]="ground",
    [559835]="ground",
    [559836]="ground",
    [559837]="ground",
    [559838]="ground",
    [559839]="ground",
    [559840]="ground",
    [559841]="ground",
    [559844]="ground",
    [559846]="ground",
    [559848]="both",
    [560100]="ground",
    [560200]="ground",
    [560201]="ground",
    [560202]="ground",
    [560203]="ground",
    [560204]="ground",
    [560205]="ground",
    [560206]="ground",
    [560207]="ground",
    [560208]="ground",
    [560209]="ground",
    [560210]="ground",
    [560211]="ground",
    [560212]="ground",
    [560213]="ground",
    [560214]="ground",
    [560215]="ground",
    [560216]="ground",
    [560217]="ground",
    [560218]="ground",
    [560219]="ground",
    [560220]="ground",
    [560221]="ground",
    [560222]="ground",
    [560223]="ground",
    [560224]="ground",
    [560225]="ground",
    [560226]="ground",
    [560227]="ground",
    [560228]="ground",
    [560229]="ground",
    [560230]="ground",
    [560231]="ground",
    [560232]="ground",
    [560233]="ground",
    [560234]="ground",
    [560235]="ground",
    [560236]="ground",
    [560237]="ground",
    [560238]="ground",
    [560239]="ground",
    [560240]="ground",
    [560241]="ground",
    [560242]="ground",
    [560243]="ground",
    [560244]="ground",
    [560245]="ground",
    [560246]="ground",
    [560247]="ground",
    [560248]="ground",
    [560249]="ground",
    [560250]="ground",
    [560251]="ground",
    [560252]="ground",
    [560253]="ground",
    [560254]="ground",
    [560255]="ground",
    [560256]="ground",
    [560257]="ground",
    [560258]="ground",
    [560259]="ground",
    [560260]="ground",
    [560261]="ground",
    [560262]="ground",
    [560263]="ground",
    [560264]="ground",
    [560265]="ground",
    [560266]="ground",
    [560267]="ground",
    [560268]="ground",
    [560269]="ground",
    [560270]="ground",
    [560271]="ground",
    [560272]="ground",
    [560273]="ground",
    [560274]="ground",
    [560275]="ground",
    [560276]="ground",
    [560277]="ground",
    [560278]="ground",
    [560279]="ground",
    [560280]="ground",
    [560281]="ground",
    [560282]="ground",
    [560283]="ground",
    [560284]="ground",
    [560285]="ground",
    [560286]="ground",
    [560287]="ground",
    [560288]="ground",
    [560289]="ground",
    [560290]="ground",
    [560291]="ground",
    [560292]="ground",
    [560293]="ground",
    [560294]="ground",
    [560295]="ground",
    [560296]="ground",
    [560297]="ground",
    [560298]="ground",
    [560299]="ground",
    [560300]="ground",
    [560301]="ground",
    [560302]="ground",
    [560303]="ground",
    [560304]="ground",
    [560305]="ground",
    [560306]="ground",
    [560307]="ground",
    [560308]="ground",
    [560309]="ground",
    [560310]="ground",
    [560311]="ground",
    [560312]="ground",
    [560313]="ground",
    [560314]="ground",
    [560315]="ground",
    [560316]="ground",
    [560317]="ground",
    [560318]="ground",
    [560319]="ground",
    [560320]="ground",
    [560321]="ground",
    [560322]="ground",
    [560323]="ground",
    [560324]="ground",
    [560325]="ground",
    [560326]="ground",
    [560327]="ground",
    [560328]="ground",
    [560329]="ground",
    [560330]="ground",
    [560331]="ground",
    [560332]="ground",
    [560333]="ground",
    [560334]="ground",
    [597601]="ground",
    [603200]="ground",
    [612826]="ground",
    [612827]="ground",
    [612828]="ground",
    [612829]="ground",
    [612830]="both",
    [612831]="both",
    [612832]="ground",
    [612833]="ground",
    [612849]="both",
    [612892]="flying",
    [623923]="flying",
    [699636]="ground",
    [699637]="ground",
    [699638]="ground",
    [699639]="both",
    [699640]="flying",
    [775001]="ground",
    [815142]="ground",
    [815143]="both",
    [815144]="ground",
    [815145]="both",
    [820031]="flying",
    [829139]="flying",
    [829140]="flying",
    [829141]="flying",
    [829142]="flying",
    [829143]="flying",
    [900185]="ground",
    [900186]="ground",
    [900917]="ground",
    [933183]="ground",
    [933184]="ground",
    [978310]="both",
    [978311]="both",
    [978312]="both",
    [978313]="both",
    [978314]="both",
    [978315]="both",
    [978316]="ground",
    [978317]="both",
    [987412]="ground",
    [1001619]="both",
    [1001620]="both",
    [1001621]="both",
    [1001622]="both",
    [1001627]="both",
    [1001628]="both",
    [1001629]="both",
    [1001630]="both",
    [1001680]="ground",
    [1001681]="ground",
    [1001682]="ground",
    [1001683]="ground",
    [1001684]="ground",
    [1001690]="ground",
    [1001846]="ground",
    [1001847]="ground",
    [1001848]="ground",
    [1001849]="ground",
    [1001850]="ground",
    [1002011]="ground",
    [1002012]="ground",
    [1002013]="ground",
    [1002014]="ground",
    [1100449]="ground",
    [1175625]="both",
    [1175634]="ground",
    [1175635]="ground",
    [1179128]="ground",
    [1179129]="ground",
    [1179130]="ground",
    [1179131]="ground",
    [1179132]="flying",
    [1179133]="flying",
    [1179134]="flying",
    [1179135]="flying",
    [1179136]="ground",
    [1179137]="ground",
    [1179138]="ground",
    [1179139]="ground",
    [1179140]="ground",
    [1179141]="ground",
    [1179142]="ground",
    [1179143]="ground",
    [1179144]="ground",
    [1179145]="ground",
    [1179146]="ground",
    [1179147]="ground",
    [1179148]="ground",
    [1179149]="ground",
    [1179150]="ground",
    [1179151]="ground",
    [1179152]="ground",
    [1179153]="flying",
    [1179154]="flying",
    [1179155]="flying",
    [1179156]="flying",
    [1179157]="flying",
    [1179158]="flying",
    [1179159]="flying",
    [1179160]="flying",
    [1179161]="flying",
    [1179162]="ground",
    [1179163]="flying",
    [1179164]="flying",
    [1179165]="ground",
    [1179166]="ground",
    [1179167]="ground",
    [1179168]="ground",
    [1179169]="ground",
    [1179170]="ground",
    [1179171]="ground",
    [1179172]="ground",
    [1179173]="ground",
    [1179174]="ground",
    [1179175]="ground",
    [1179176]="ground",
    [1179177]="ground",
    [1179178]="ground",
    [1179179]="ground",
    [1179180]="ground",
    [1179181]="ground",
    [1179182]="ground",
    [1179183]="ground",
    [1179184]="ground",
    [1179185]="ground",
    [1179186]="ground",
    [1179187]="ground",
    [1179188]="ground",
    [1179189]="ground",
    [1179190]="ground",
    [1179191]="ground",
    [1179192]="ground",
    [1179193]="ground",
    [1179194]="ground",
    [1179195]="ground",
    [1179196]="ground",
    [1179197]="ground",
    [1179198]="ground",
    [1179199]="flying",
    [1179200]="flying",
    [1179201]="flying",
    [1179202]="flying",
    [1179203]="flying",
    [1179204]="ground",
    [1179205]="ground",
    [1179206]="ground",
    [1179207]="ground",
    [1179208]="ground",
    [1179209]="both",
    [1179210]="both",
    [1179211]="ground",
    [1179212]="both",
    [1179213]="ground",
    [1179217]="flying",
    [1179218]="ground",
    [1179219]="ground",
    [1179220]="ground",
    [1179221]="ground",
    [1179222]="ground",
    [1179223]="ground",
    [1179224]="ground",
    [1179225]="ground",
    [1179226]="ground",
    [1179227]="ground",
    [1179228]="flying",
    [1179229]="flying",
    [1233809]="ground",
    [1235071]="ground",
    [1235072]="ground",
    [1235073]="ground",
    [1235074]="ground",
    [1235075]="ground",
    [1235076]="ground",
    [1235077]="ground",
    [1235078]="ground",
    [1235079]="ground",
    [1235080]="ground",
    [1235081]="ground",
    [1235082]="ground",
    [1235083]="ground",
    [1235084]="ground",
    [1235085]="ground",
    [1235086]="ground",
    [1235087]="ground",
    [1235088]="ground",
    [1235089]="ground",
    [1235090]="ground",
    [1235091]="ground",
    [1235092]="ground",
    [1235093]="ground",
    [1235094]="ground",
    [1235095]="ground",
    [1235096]="ground",
    [1235097]="ground",
    [1235098]="ground",
    [1235099]="ground",
    [1235100]="ground",
    [1235101]="ground",
    [1235102]="ground",
    [1235103]="ground",
    [1235104]="ground",
    [1235105]="ground",
    [1235106]="ground",
    [1235107]="ground",
    [1235108]="ground",
    [1235109]="ground",
    [1235110]="ground",
    [1235111]="ground",
    [1235112]="ground",
    [1235113]="ground",
    [1235114]="ground",
    [1284998]="ground",
    [1284999]="ground",
    [1285000]="ground",
    [1285001]="ground",
    [1285002]="ground",
    [1285003]="ground",
    [1285004]="ground",
    [1333809]="ground",
    [1433809]="ground",
    [1500000]="ground",
    [1533809]="ground",
    [1633809]="ground",
    [1733809]="ground",
    [1777035]="ground",
    [1777038]="ground",
    [1777039]="ground",
    [1777040]="ground",
    [1777041]="ground",
    [1777060]="both",
    [1777068]="ground",
    [1777069]="ground",
    [1777071]="ground",
    [1777077]="both",
    [1777078]="ground",
    [1777317]="ground",
    [1777318]="ground",
    [1777319]="ground",
    [1777320]="ground",
    [1777321]="ground",
    [1777322]="ground",
    [1777323]="ground",
    [1777324]="ground",
    [1777325]="ground",
    [1777326]="ground",
    [1777327]="ground",
    [1777328]="ground",
    [1777329]="ground",
    [1777330]="ground",
    [1777331]="ground",
    [1777332]="ground",
    [1777333]="ground",
    [1777334]="ground",
    [1777335]="ground",
    [1777336]="ground",
    [1777338]="both",
    [1777339]="ground",
    [1777340]="ground",
    [1777341]="ground",
    [1777342]="ground",
    [1777360]="both",
    [1777361]="both",
    [1777362]="both",
    [1777363]="ground",
    [1777364]="ground",
    [1777365]="ground",
    [1777366]="ground",
    [1777367]="ground",
    [1777368]="ground",
    [1777369]="ground",
    [1777370]="ground",
    [1777371]="ground",
    [1777372]="ground",
    [1777373]="ground",
    [1777374]="ground",
    [1777375]="ground",
    [1777376]="ground",
    [1777377]="ground",
    [1777413]="both",
    [1777414]="both",
    [1777415]="both",
    [1777416]="both",
    [1777417]="ground",
    [1777429]="ground",
    [1777430]="ground",
    [1777431]="ground",
    [1777432]="ground",
    [1777433]="ground",
    [1777434]="ground",
    [1777435]="ground",
    [1777436]="ground",
    [1777437]="ground",
    [1777438]="ground",
    [1777439]="ground",
    [1777440]="ground",
    [1777441]="ground",
    [1804703]="ground",
    [1833809]="ground",
    [1877071]="ground",
    [8199945]="both",
    [8199946]="ground",
    [8199947]="ground",
    [8199948]="ground",
    [8199949]="ground",
    [8199950]="ground",
    [8199951]="ground",
    [8199952]="ground",
    [8199953]="ground",
    [8199954]="ground",
    [8199955]="ground",
    [8199956]="ground",
    [8199957]="ground",
    [8199958]="ground",
    [8199959]="ground",
    [8199960]="ground",
    [8199961]="ground",
    [8199962]="ground",
    [8199963]="ground",
    [8199964]="ground",
    [8199965]="ground",
    [8199966]="ground",
    [8199967]="ground",
    [8199968]="ground",
    [8199969]="ground",
    [8199970]="ground",
    [8199971]="ground",
    [8199972]="ground",
    [8199973]="ground",
    [8199974]="ground",
    [8199975]="ground",
    [8199976]="ground",
    [8199977]="ground",
    [8199978]="ground",
    [8199979]="both",
    [8199980]="both",
    [8199981]="both",
    [8199982]="both",
    [8199983]="both",
    [8199984]="ground",
    [8199985]="ground",
    [8199986]="ground",
    [8199987]="ground",
    [8199988]="ground",
    [8199989]="ground",
    [8199990]="ground",
    [8199991]="ground",
    [8199992]="ground",
    [8199993]="ground",
    [8199994]="ground",
    [8199995]="ground",
    [8199996]="ground",
    [8199997]="ground",
    [8199998]="ground",
    [8199999]="ground",
    [8200000]="both",
    [8200001]="both",
    [8200002]="both",
    [8200003]="ground",
    [8200004]="both",
    [8200005]="both",
    [8200006]="both",
    [8200007]="ground",
    [8200008]="both",
    [8200018]="ground",
    [8200019]="ground",
    [8200026]="ground",
    [8200027]="ground",
    [8200028]="both",
    [8200029]="ground",
    [8200030]="ground",
    [8200031]="ground",
    [8200032]="ground",
    [8200038]="both",
    [8200039]="ground",
    [8200040]="ground",
    [8200041]="ground",
    [8200043]="both",
    [8200044]="both",
    [8200045]="both",
    [8200046]="both",
    [8200047]="both",
    [8200048]="both",
    [8200049]="both",
    [8200050]="ground",
    [8200051]="both",
    [8200052]="both",
    [8200053]="both",
    [8200054]="ground",
    [8200055]="both",
    [8200056]="both",
    [8200057]="both",
    [8200058]="ground",
    [8200059]="ground",
    [8200060]="ground",
    [8200061]="ground",
    [8200062]="ground",
    [8200067]="ground",
    [8200068]="ground",
    [8200069]="ground",
    [8200070]="ground",
    [8200071]="ground",
    [8200072]="ground",
    [8200073]="ground",
    [8200074]="ground",
    [8200075]="ground",
    [8200076]="ground",
    [8200077]="ground",
    [8200080]="both",
    [8200081]="both",
    [8200082]="both",
    [8200083]="ground",
    [8200084]="ground",
    [8200085]="ground",
    [8200086]="ground",
    [8200087]="ground",
    [8200094]="ground",
    [8200095]="ground",
    [8200096]="both",
    [8200097]="both",
    [8200098]="both",
    [8200099]="ground",
    [8200100]="both",
    [8200112]="both",
    [8200113]="both",
    [8200114]="both",
    [8200115]="ground",
    [8200116]="both",
    [8200117]="ground",
    [8200126]="ground",
    [8200127]="ground",
    [8200128]="ground",
    [8200129]="ground",
    [8200130]="ground",
    [8200177]="ground",
    [8200178]="ground",
    [8200179]="ground",
    [8200180]="ground",
    [8200181]="ground",
    [8200182]="ground",
    [8200183]="both",
    [8200185]="ground",
    [8200186]="ground",
    [8200188]="ground",
    [8200204]="both",
    [8200205]="both",
    [8200211]="ground",
    [8200212]="ground",
    [8200213]="ground",
    [8200214]="ground",
    [8200215]="flying",
    [8388604]="ground",
}

Area52FeaturedEditor.PreviewCatalog = {
    {
        {id=74491,name="Creature name unavailable"},
        {id=80854,name="Creature name unavailable"},
        {id=24747,name="Fjord Hawk"},
        {id=6554,name="Gorishi Stinger"},
        {id=416,name="Imp"},
        {id=16941,name="Phasing Voidcaller"},
        {id=1042,name="Red Whelp"},
        {id=35189,name="Skoll"},
        {id=1863,name="Succubus"},
        {id=18683,name="Voidhunter Yar"},
        {id=1860,name="Voidwalker"},
        {id=510,name="Water Elemental"},
    },
    {
        {id=15313,name="Druid of the Eclipse form"},
        {id=15314,name="Druid of the Eclipse form"},
        {id=41264,name="Incarnation 15605"},
        {id=41265,name="Incarnation 15606"},
        {id=41266,name="Incarnation 15607"},
        {id=41267,name="Incarnation 15608"},
        {id=41268,name="Incarnation 15609"},
        {id=41269,name="Incarnation 15610"},
        {id=41270,name="Incarnation 15611"},
        {id=424731,name="Incarnation 179673"},
        {id=424732,name="Incarnation 179674"},
        {id=424733,name="Incarnation 179675"},
        {id=424734,name="Incarnation 179676"},
        {id=424735,name="Incarnation 179677"},
        {id=424737,name="Incarnation 179678"},
        {id=308385,name="Incarnation 261377"},
        {id=308386,name="Incarnation 261378"},
        {id=308387,name="Incarnation 261379"},
        {id=308388,name="Incarnation 261380"},
        {id=308389,name="Incarnation 261381"},
        {id=308390,name="Incarnation 261382"},
        {id=499780,name="Incarnation 45401"},
        {id=499777,name="Incarnation 45403"},
        {id=499796,name="Incarnation 47124"},
        {id=499720,name="Incarnation 47250"},
        {id=499719,name="Incarnation 47251"},
        {id=90248,name="Incarnation 47522"},
        {id=1815,name="Incarnation 47524"},
        {id=499721,name="Incarnation 47533"},
        {id=499718,name="Incarnation 47536"},
        {id=599787,name="Incarnation 47537"},
        {id=499787,name="Incarnation 47538"},
        {id=499788,name="Incarnation 47539"},
        {id=499786,name="Incarnation 47540"},
        {id=499785,name="Incarnation 47541"},
        {id=499795,name="Incarnation 47542"},
        {id=499798,name="Incarnation 47543"},
        {id=499799,name="Incarnation 47544"},
        {id=499797,name="Incarnation 47545"},
        {id=499794,name="Incarnation 47547"},
        {id=499800,name="Incarnation 47548"},
        {id=499793,name="Incarnation 47549"},
        {id=499791,name="Incarnation 47550"},
        {id=499792,name="Incarnation 47551"},
        {id=499790,name="Incarnation 47552"},
        {id=499789,name="Incarnation 47553"},
        {id=499784,name="Incarnation 47554"},
        {id=499783,name="Incarnation 47555"},
        {id=499782,name="Incarnation 47556"},
        {id=499779,name="Incarnation 47558"},
        {id=499802,name="Incarnation 47560"},
        {id=499801,name="Incarnation 47561"},
        {id=499803,name="Incarnation 47562"},
        {id=80655,name="Incarnation 47563"},
        {id=173527,name="Incarnation 47569"},
        {id=404520,name="Incarnation 47571"},
        {id=404521,name="Incarnation 47572"},
        {id=404522,name="Incarnation 47573"},
        {id=404523,name="Incarnation 47574"},
        {id=404524,name="Incarnation 47575"},
        {id=404525,name="Incarnation 47576"},
        {id=304276,name="Incarnation 47578"},
        {id=81100,name="Incarnation 47580"},
        {id=81101,name="Incarnation 47581"},
        {id=81102,name="Incarnation 47582"},
        {id=81104,name="Incarnation 47584"},
        {id=81105,name="Incarnation 47585"},
        {id=81106,name="Incarnation 47586"},
        {id=81107,name="Incarnation 47587"},
        {id=81108,name="Incarnation 47588"},
        {id=81109,name="Incarnation 47589"},
        {id=81110,name="Incarnation 47590"},
        {id=81111,name="Incarnation 47591"},
        {id=499733,name="Incarnation 47592"},
        {id=499732,name="Incarnation 47593"},
        {id=499731,name="Incarnation 47595"},
        {id=499734,name="Incarnation 47596"},
        {id=499735,name="Incarnation 47597"},
        {id=499736,name="Incarnation 47598"},
        {id=499737,name="Incarnation 47599"},
        {id=499742,name="Incarnation 47600"},
        {id=499743,name="Incarnation 47601"},
        {id=499744,name="Incarnation 47602"},
        {id=499745,name="Incarnation 47603"},
        {id=499738,name="Incarnation 47604"},
        {id=499739,name="Incarnation 47605"},
        {id=499740,name="Incarnation 47606"},
        {id=499741,name="Incarnation 47607"},
        {id=499746,name="Incarnation 47608"},
        {id=499747,name="Incarnation 47609"},
        {id=499748,name="Incarnation 47610"},
        {id=499749,name="Incarnation 47611"},
        {id=499750,name="Incarnation 47612"},
        {id=499751,name="Incarnation 47613"},
        {id=499752,name="Incarnation 47614"},
        {id=141758,name="Incarnation 47615"},
        {id=80963,name="Incarnation 47640"},
        {id=80650,name="Incarnation 47642"},
        {id=499396,name="Incarnation 47647"},
        {id=371442,name="Incarnation 47648"},
        {id=371443,name="Incarnation 47649"},
        {id=371444,name="Incarnation 47650"},
        {id=1777239,name="Incarnation 47651"},
        {id=499638,name="Incarnation 47653"},
        {id=499639,name="Incarnation 47654"},
        {id=499640,name="Incarnation 47655"},
        {id=371436,name="Incarnation 47656"},
        {id=371437,name="Incarnation 47657"},
        {id=371438,name="Incarnation 47658"},
        {id=371439,name="Incarnation 47659"},
        {id=371440,name="Incarnation 47660"},
        {id=371441,name="Incarnation 47661"},
        {id=80882,name="Incarnation 51206"},
        {id=421465,name="Incarnation 51207"},
        {id=421466,name="Incarnation 51208"},
        {id=421467,name="Incarnation 51209"},
        {id=421468,name="Incarnation 51210"},
        {id=322518,name="Incarnation 51211"},
        {id=322519,name="Incarnation 51212"},
        {id=322520,name="Incarnation 51213"},
        {id=322521,name="Incarnation 51214"},
        {id=322522,name="Incarnation 51215"},
        {id=322523,name="Incarnation 51216"},
        {id=144889,name="Incarnation 51217"},
        {id=81103,name="Incarnation 51429"},
        {id=67268,name="Incarnation 6004"},
        {id=67269,name="Incarnation 6005"},
        {id=67270,name="Incarnation 6006"},
        {id=67271,name="Incarnation 6007"},
        {id=67272,name="Incarnation 6008"},
        {id=67273,name="Incarnation 6009"},
        {id=699796,name="Incarnation 61375"},
        {id=699797,name="Incarnation 61376"},
        {id=699798,name="Incarnation 61377"},
        {id=699799,name="Incarnation 61378"},
        {id=421493,name="Incarnation 645"},
        {id=421494,name="Incarnation 646"},
        {id=421495,name="Incarnation 647"},
        {id=421496,name="Incarnation 648"},
        {id=421497,name="Incarnation 649"},
        {id=421498,name="Incarnation 650"},
        {id=421499,name="Incarnation 651"},
        {id=421491,name="Incarnation 652"},
        {id=99033,name="Incarnation: Druid of the Flames"},
        {id=200157,name="Incarnation: Emperor Korthek"},
        {id=5531,name="Incarnation: Fallen to Nightmare (Gray & Red)"},
        {id=499778,name="Incarnation: Guardian of the Glade (Maroon)"},
        {id=80881,name="Incarnation: Metamorphosis of Fel"},
        {id=499804,name="Incarnation: Might of the Grizzlemaw (White)"},
        {id=1816,name="Incarnation: Plague Bear (Brown)"},
        {id=499637,name="Incarnation: Tree of Wrath (Brown)"},
    },
}
Area52FeaturedEditor.IncarnationChoices = {
    {
        {id=822,name="Incarnation: Plague Bear (Brown)"},
        {id=45401,name="Incarnation 45401"},
        {id=45402,name="Incarnation: Guardian of the Glade (Maroon)"},
        {id=45403,name="Incarnation 45403"},
        {id=47124,name="Incarnation 47124"},
        {id=47250,name="Incarnation 47250"},
        {id=47251,name="Incarnation 47251"},
        {id=47522,name="Incarnation 47522"},
        {id=47524,name="Incarnation 47524"},
        {id=47533,name="Incarnation 47533"},
        {id=47536,name="Incarnation 47536"},
        {id=47537,name="Incarnation 47537"},
        {id=47538,name="Incarnation 47538"},
        {id=47539,name="Incarnation 47539"},
        {id=47540,name="Incarnation 47540"},
        {id=47541,name="Incarnation 47541"},
        {id=47542,name="Incarnation 47542"},
        {id=47543,name="Incarnation 47543"},
        {id=47544,name="Incarnation 47544"},
        {id=47545,name="Incarnation 47545"},
        {id=47547,name="Incarnation 47547"},
        {id=47548,name="Incarnation 47548"},
        {id=47549,name="Incarnation 47549"},
        {id=47550,name="Incarnation 47550"},
        {id=47551,name="Incarnation 47551"},
        {id=47552,name="Incarnation 47552"},
        {id=47553,name="Incarnation 47553"},
        {id=47554,name="Incarnation 47554"},
        {id=47555,name="Incarnation 47555"},
        {id=47556,name="Incarnation 47556"},
        {id=47557,name="Incarnation: Fallen to Nightmare (Gray & Red)"},
        {id=47558,name="Incarnation 47558"},
        {id=47559,name="Incarnation: Might of the Grizzlemaw (White)"},
        {id=47560,name="Incarnation 47560"},
        {id=47561,name="Incarnation 47561"},
        {id=47562,name="Incarnation 47562"},
        {id=61375,name="Incarnation 61375"},
        {id=61376,name="Incarnation 61376"},
        {id=61377,name="Incarnation 61377"},
        {id=61378,name="Incarnation 61378"},
        {id=261377,name="Incarnation 261377"},
        {id=261378,name="Incarnation 261378"},
        {id=261379,name="Incarnation 261379"},
        {id=261380,name="Incarnation 261380"},
        {id=261381,name="Incarnation 261381"},
        {id=261382,name="Incarnation 261382"},
    },
    {
        {id=645,name="Incarnation 645"},
        {id=646,name="Incarnation 646"},
        {id=647,name="Incarnation 647"},
        {id=648,name="Incarnation 648"},
        {id=649,name="Incarnation 649"},
        {id=650,name="Incarnation 650"},
        {id=651,name="Incarnation 651"},
        {id=652,name="Incarnation 652"},
        {id=6004,name="Incarnation 6004"},
        {id=6005,name="Incarnation 6005"},
        {id=6006,name="Incarnation 6006"},
        {id=6007,name="Incarnation 6007"},
        {id=6008,name="Incarnation 6008"},
        {id=6009,name="Incarnation 6009"},
        {id=47563,name="Incarnation 47563"},
        {id=47565,name="Incarnation 47565"},
        {id=47566,name="Incarnation 47566"},
        {id=47567,name="Incarnation 47567"},
        {id=47568,name="Incarnation 47568"},
        {id=47569,name="Incarnation 47569"},
        {id=47571,name="Incarnation 47571"},
        {id=47572,name="Incarnation 47572"},
        {id=47573,name="Incarnation 47573"},
        {id=47574,name="Incarnation 47574"},
        {id=47575,name="Incarnation 47575"},
        {id=47576,name="Incarnation 47576"},
        {id=47577,name="Incarnation: Emperor Korthek"},
        {id=47578,name="Incarnation 47578"},
        {id=47579,name="Incarnation: Druid of the Flames"},
        {id=47580,name="Incarnation 47580"},
        {id=47581,name="Incarnation 47581"},
        {id=47582,name="Incarnation 47582"},
        {id=47584,name="Incarnation 47584"},
        {id=47585,name="Incarnation 47585"},
        {id=47586,name="Incarnation 47586"},
        {id=47587,name="Incarnation 47587"},
        {id=47588,name="Incarnation 47588"},
        {id=47589,name="Incarnation 47589"},
        {id=47590,name="Incarnation 47590"},
        {id=47591,name="Incarnation 47591"},
        {id=47592,name="Incarnation 47592"},
        {id=47593,name="Incarnation 47593"},
        {id=47595,name="Incarnation 47595"},
        {id=47596,name="Incarnation 47596"},
        {id=47597,name="Incarnation 47597"},
        {id=47598,name="Incarnation 47598"},
        {id=47599,name="Incarnation 47599"},
        {id=47600,name="Incarnation 47600"},
        {id=47601,name="Incarnation 47601"},
        {id=47602,name="Incarnation 47602"},
        {id=47603,name="Incarnation 47603"},
        {id=47604,name="Incarnation 47604"},
        {id=47605,name="Incarnation 47605"},
        {id=47606,name="Incarnation 47606"},
        {id=47607,name="Incarnation 47607"},
        {id=47608,name="Incarnation 47608"},
        {id=47609,name="Incarnation 47609"},
        {id=47610,name="Incarnation 47610"},
        {id=47611,name="Incarnation 47611"},
        {id=47612,name="Incarnation 47612"},
        {id=47613,name="Incarnation 47613"},
        {id=47614,name="Incarnation 47614"},
        {id=47615,name="Incarnation 47615"},
        {id=51429,name="Incarnation 51429"},
    },
    {
        {id=15605,name="Incarnation 15605"},
        {id=15606,name="Incarnation 15606"},
        {id=15607,name="Incarnation 15607"},
        {id=15608,name="Incarnation 15608"},
        {id=15609,name="Incarnation 15609"},
        {id=15610,name="Incarnation 15610"},
        {id=15611,name="Incarnation 15611"},
        {id=47640,name="Incarnation 47640"},
        {id=47642,name="Incarnation 47642"},
        {id=47647,name="Incarnation 47647"},
        {id=47648,name="Incarnation 47648"},
        {id=47649,name="Incarnation 47649"},
        {id=47650,name="Incarnation 47650"},
        {id=47651,name="Incarnation 47651"},
        {id=47652,name="Incarnation: Tree of Wrath (Brown)"},
        {id=47653,name="Incarnation 47653"},
        {id=47654,name="Incarnation 47654"},
        {id=47655,name="Incarnation 47655"},
        {id=47656,name="Incarnation 47656"},
        {id=47657,name="Incarnation 47657"},
        {id=47658,name="Incarnation 47658"},
        {id=47659,name="Incarnation 47659"},
        {id=47660,name="Incarnation 47660"},
        {id=47661,name="Incarnation 47661"},
        {id=179673,name="Incarnation 179673"},
        {id=179674,name="Incarnation 179674"},
        {id=179675,name="Incarnation 179675"},
        {id=179676,name="Incarnation 179676"},
        {id=179677,name="Incarnation 179677"},
        {id=179678,name="Incarnation 179678"},
    },
    {
        {id=51205,name="Incarnation: Metamorphosis of Fel"},
        {id=51206,name="Incarnation 51206"},
        {id=51207,name="Incarnation 51207"},
        {id=51208,name="Incarnation 51208"},
        {id=51209,name="Incarnation 51209"},
        {id=51210,name="Incarnation 51210"},
        {id=51211,name="Incarnation 51211"},
        {id=51212,name="Incarnation 51212"},
        {id=51213,name="Incarnation 51213"},
        {id=51214,name="Incarnation 51214"},
        {id=51215,name="Incarnation 51215"},
        {id=51216,name="Incarnation 51216"},
        {id=51217,name="Incarnation 51217"},
    },
}
Area52FeaturedEditor.IncarnationDefaults = {
    [33]={"Incarnation: Fallen to Nightmare (Gray & Red)","None","None","None"},
    [143]={"Incarnation: Guardian of the Glade (Maroon)","None","None","None"},
    [26]={"None","None","None","Incarnation: Metamorphosis of Fel"},
    [75]={"None","Incarnation: Emperor Korthek","None","None"},
    [131]={"None","Incarnation: Druid of the Flames","None","None"},
    [32]={"Incarnation: Plague Bear (Brown)","None","None","None"},
    [34]={"Incarnation: Might of the Grizzlemaw (White)","None","None","None"},
    [14]={"None","None","Incarnation: Tree of Wrath (Brown)","None"},
}

local F=Area52FeaturedEditor
F.PreviewLabels={"Combat pet / pet appearance reward","Incarnation preview (matches reward after launcher update)","Character cast visual","Pet cast visual","Bear incarnation reward","Cat incarnation reward","Tree incarnation reward","Metamorphosis incarnation reward"}
F.RewardPets={
    {id=1042,name="Red Whelp â€” combat pet"},
    {id=24747,name="Fjord Hawk â€” combat pet"},
    {id=35189,name="Skoll â€” combat pet"},
    {id=6554,name="Gorishi Stinger â€” tameable combat pet"},
    {id=74491,name="Vanguard Sentinel â€” Voidwalker appearance"},
    {id=16941,name="Phasing Voidcaller â€” Voidwalker appearance"},
    {id=18683,name="Beast of Nightmare â€” Voidwalker appearance"},
    {id=80854,name="Shadowseer â€” Succubus appearance"},
}

function F.ReadPreview(description)
    local marker="###AREA52_FEATURED_PREVIEW\n"
    local start=(description or ""):find(marker,1,true)
    if not start then return {4294967295,4294967295,4294967295,4294967295},false end
    if start>1 and description:sub(start-1,start-1)~="\n" or description:find(marker,start+#marker,true) then return nil end
    local raw=description:sub(start+#marker):match("^[^\n]*")
    if not raw or not raw:match("^1:[%d,]+$") then return nil end
    local values={}
    for token in (raw:sub(3)..","):gmatch("([^,]*),") do
        local value=tonumber(token)
        if not value or value<0 or value>4294967295 or #values>=4 then return nil end
        values[#values+1]=value
    end
    if #values~=4 then return nil end
    return values,true
end

function F.PreviewChoices(view,index)
    local result={{id=4294967295,name="Archetype default"},{id=0,name="None"}}
    if index>4 then
        local defaults=(F.IncarnationDefaults or {})[view.Area52ArchetypeID]
        result[1].name="Archetype default: "..(defaults and defaults[index-4] or "None")
        for _,choice in ipairs((F.IncarnationChoices or {})[index-4] or {}) do result[#result+1]=choice end
    elseif index==1 then
        for _,choice in ipairs(F.RewardPets) do result[#result+1]=choice end
    elseif index<3 then
        for _,choice in ipairs((F.PreviewCatalog or {})[index] or {}) do result[#result+1]=choice end
    else
        local seen={}
        for _,spell in ipairs(C_BuildEditor.GetPendingBuild().Spells or {}) do
            local id=spell.Spell
            if type(id)=="number" and id>0 and not seen[id] then
                local name=GetSpellInfo(id)
                if name then result[#result+1]={id=id,name=name};seen[id]=true end
            end
        end
        table.sort(result,function(a,b)if a.id==4294967295 then return b.id~=4294967295 elseif b.id==4294967295 then return false elseif a.id==0 then return b.id~=0 elseif b.id==0 then return false end;return a.name==b.name and a.id<b.id or a.name<b.name end)
    end
    return result
end

function F.RefreshPreviewControls(view)
    local panel=view.Area52PreviewSection
    if not panel then return end
    for index=1,8 do
        local id=view.Area52PreviewEffects[index]
        local name=id==0 and "None" or tostring(id)
        for _,choice in ipairs(F.PreviewChoices(view,index)) do if choice.id==id then name=choice.name;break end end
        if index>4 and id~=0 and id~=4294967295 then
            local _,_,_,displayName=C_Appearance.GetAppearanceDisplayInfo(id)
            if displayName and displayName~="" then name=displayName end
        end
        panel.Buttons[index]:SetText(F.PreviewLabels[index]..": "..name)
    end
end

function F.OpenPreviewChoices(view,index)
    if index==2 or not F.Applies() or view.Area52PreviewInvalid then return end
    if index>4 then return F.OpenIncarnationPicker(view,index) end
    local context={view=view,id=C_BuildEditor.GetPendingBuild().ID,archetype=view.Area52ArchetypeID,generation=view.Area52FeaturedGeneration}
    local choices=F.PreviewChoices(view,index)
    local popup=F.PreviewPopup
    if not popup then
        popup=CreateFrame("Frame",nil,UIParent)
        popup:SetSize(420,390);popup:SetPoint("CENTER");popup:SetFrameStrata("DIALOG")
        popup:SetBackdrop({bgFile="Interface\\Buttons\\WHITE8X8",edgeFile="Interface\\Tooltips\\UI-Tooltip-Border",edgeSize=16})
        popup:SetBackdropColor(0.04,0.06,0.09,1)
        popup.Buttons={}
        popup.Title=popup:CreateFontString(nil,"OVERLAY","GameFontNormal")
        popup.Title:SetPoint("TOP",0,-16)
        for row=1,10 do
            local button=CreateFrame("Button",nil,popup,"UIPanelButtonTemplate")
            button:SetSize(380,26);button:SetPoint("TOP",0,-42-(row-1)*28)
            popup.Buttons[row]=button
        end
        popup.Previous=CreateFrame("Button",nil,popup,"UIPanelButtonTemplate")
        popup.Previous:SetSize(100,26);popup.Previous:SetPoint("BOTTOMLEFT",20,18);popup.Previous:SetText("Previous")
        popup.Next=CreateFrame("Button",nil,popup,"UIPanelButtonTemplate")
        popup.Next:SetSize(100,26);popup.Next:SetPoint("BOTTOM",0,18);popup.Next:SetText("Next")
        popup.Cancel=CreateFrame("Button",nil,popup,"UIPanelButtonTemplate")
        popup.Cancel:SetSize(100,26);popup.Cancel:SetPoint("BOTTOMRIGHT",-20,18);popup.Cancel:SetText("Cancel")
        popup.Cancel:SetScript("OnClick",function()popup:Hide()end)
        F.PreviewPopup=popup
    end
    local page=1
    local function refresh()
        popup.Title:SetText(F.PreviewLabels[index].." â€” "..page.."/"..math.max(1,math.ceil(#choices/10)))
        for row,button in ipairs(popup.Buttons) do
            local choice=choices[(page-1)*10+row]
            button:SetShown(choice~=nil)
            if choice then
                button:SetText(choice.name)
                button:SetScript("OnClick",function()
                    popup:Hide()
                    if not F.ValidContext(context) or view.Area52PreviewInvalid then return end
                    view.Area52PreviewEffects[index]=choice.id
                    F.RefreshPreviewControls(view)
                    view:UpdatePendingBuild()
                end)
            end
        end
        popup.Previous:SetScript("OnClick",function()page=math.max(1,page-1);refresh()end)
        popup.Next:SetScript("OnClick",function()page=math.min(math.max(1,math.ceil(#choices/10)),page+1);refresh()end)
    end
    refresh();popup:Show()
end

local previousBefore=F.Before
function F.Before(view,deserialize)
    previousBefore(view,deserialize)
    if GetRealmName()~="Area 52 - Free-Pick" then return end
    local build=C_BuildEditor.GetPendingBuild()
    if deserialize or view.Area52PreviewBuild~=build.ID or not view.Area52PreviewEffects then
        view.Area52PreviewEffects=F.ReadPreview(build.Description)
        view.Area52PreviewInvalid=not view.Area52PreviewEffects
        view.Area52PreviewEffects=view.Area52PreviewEffects or {0,0,0,0}
        view.Area52PreviewEffects[2]=4294967295
        local rewardDescription=(build.Description or ""):gsub("###AREA52_FEATURED_PREVIEW","###IGNORED_PREVIEW"):gsub("###AREA52_FEATURED_INCARNATIONS","###AREA52_FEATURED_PREVIEW")
        local rewards=F.ReadPreview(rewardDescription)
        if not rewards then view.Area52PreviewInvalid=true end
        for index=1,4 do view.Area52PreviewEffects[index+4]=rewards and rewards[index] or 4294967295 end
        view.Area52PreviewBuild=build.ID
    end
    if not view.Area52PreviewSection then
        BuildViewSectionMixin.CategoryIcons.AREA52_FEATURED_PREVIEW="Interface\\Icons\\inv_misc_book_09"
        local content=view.MainFrame.ScrollFrame.Content
        local panel=CreateFrame("Frame",view:GetName().."Area52Preview",content,"EditableBuildViewSectionTemplate")
        panel:SetSize(764,340);panel:SetCategory("AREA52_FEATURED_PREVIEW");panel.Area52FeaturedOnly=true
        panel.Serialize=function()return nil end;panel.Deserialize=function()end
        panel.Buttons={}
        for index=1,8 do
            local selected=index
            local button=CreateFrame("Button",nil,panel,"UIPanelButtonTemplate")
            button:SetSize(660,28);button:SetPoint("TOP",0,-38-(index-1)*32)
            button:SetScript("OnClick",function()F.OpenPreviewChoices(view,selected)end)
            if index==2 then button:Disable() end
            if index>4 then
                button:SetScript("OnEnter",function(self)
                    GameTooltip:SetOwner(self,"ANCHOR_RIGHT")
                    GameTooltip:SetText("Choose and preview the incarnation in Wardrobe. Save changes the Draft reward; the matching creation preview arrives with the next launcher catalog update.",nil,nil,nil,nil,true)
                    GameTooltip:Show()
                end)
                button:SetScript("OnLeave",function()GameTooltip:Hide()end)
            end
            panel.Buttons[index]=button
        end
        view.Area52PreviewSection=panel
        local ordered={}
        for _,section in ipairs(content.sections) do
            if section~=panel then
                ordered[#ordered+1]=section
                if section==view.Area52VanitySection then ordered[#ordered+1]=panel end
            end
        end
        content.sections=ordered
        view:HookScript("OnHide",function()if F.PreviewPopup then F.PreviewPopup:Hide()end end)
    end
    local panel=view.Area52PreviewSection
    local enabled=F.Applies()
    panel:SetShown(enabled)
    panel:ClearAllPoints();panel:SetPoint("TOPLEFT",view.Area52VanitySection,"BOTTOMLEFT",0,-16)
    local consumables=view.MainFrame.ScrollFrame.Content.Consumables
    consumables:ClearAllPoints();consumables:SetPoint("TOPLEFT",enabled and panel or view.MainFrame.ScrollFrame.Content.Rotation,"BOTTOMLEFT",0,-16)
    F.RefreshPreviewControls(view)
end

BUILDCREATOR_SECTION_AREA52_FEATURED_PREVIEW="Character Creation and Incarnation Rewards"
local previousDescription=Area52DraftSelector.Description
function Area52DraftSelector.Description(view,description)
    description=(description or ""):gsub("\n*###AREA52_FEATURED_PREVIEW[^#]*","")
    description=description:gsub("\n*###AREA52_FEATURED_INCARNATIONS[^#]*","")
    if F.Applies() and view.Area52PreviewEffects then
        local effects=view.Area52PreviewEffects
        description=description.."\n\n###AREA52_FEATURED_PREVIEW\n1:"..table.concat({effects[1],4294967295,effects[3],effects[4]},",")
        description=description.."\n\n###AREA52_FEATURED_INCARNATIONS\n1:"..table.concat({effects[5] or 4294967295,effects[6] or 4294967295,effects[7] or 4294967295,effects[8] or 4294967295},",")
    end
    return previousDescription(view,description)
end
local previousValidate=Area52CanPublishBuild
function Area52CanPublishBuild(view)
    if F.Applies() and view.Area52ImportPending then return false,{"Archetype items are still loading. Wait before saving."} end
    if F.Applies() and view.Area52PreviewInvalid then return false,{"Invalid Character Creation preview settings. Reopen an intact saved build."} end
    return previousValidate(view)
end
local previousReason=Area52BuildPublishReason
function Area52BuildPublishReason(reason)
    if reason=="PUBLISH_BUILD_INVALID_FEATURED_INCARNATIONS" then return "Choose a supported incarnation reward for each form." end
    if reason=="PUBLISH_BUILD_INVALID_FEATURED_PREVIEW" then return "The Character Creation preview contains an invalid pet, incarnation or spell visual. Choose a supported option and save again." end
    return previousReason(reason)
end

function F.OpenIncarnationPicker(view,index)
    F.OpenPicker(view,0)
    if not F.picker then return end
    F.picker.incarnationIndex=index
    AppearanceUI.OpenCategory("APPEARANCE_TYPE_INCARNATION",({17,18,23,31})[index-4])
    AppearanceWardrobeFrame.Collection.SearchBox:SetText("")
    AppearanceWardrobeFrame.Collection:RefreshCategory()
    F.PickerBar.Text:SetText("Assign to Featured Draft: "..F.PreviewLabels[index])
end

local previousPickAppearance=F.PickAppearance
function F.PickAppearance(appearance)
    local context=F.picker
    if not context or not context.incarnationIndex then return previousPickAppearance(appearance) end
    if not F.ValidContext(context) then F.ClosePicker(false);return true end
    for _,choice in ipairs((F.IncarnationChoices or {})[context.incarnationIndex-4] or {}) do
        if choice.id==appearance then
            local view=context.view
            view.Area52PreviewEffects[context.incarnationIndex]=appearance
            F.ClosePicker(true)
            F.RefreshPreviewControls(view)
            view:UpdatePendingBuild()
            return true
        end
    end
    F.Message("Choose a supported incarnation for this form category.")
    return true
end

local sessionUpdate=EditableBuildViewMixin.UpdatePendingBuild
function EditableBuildViewMixin:UpdatePendingBuild(deserialize)
    if self.Area52Updating then
        self.Area52UpdateQueued=true
        return
    end
    self.Area52Updating=true
    self.Area52DeferredImport=nil
    local ok,message=pcall(sessionUpdate,self,deserialize)
    self.Area52Updating=nil
    if not ok then
        self.Area52UpdateQueued=nil
        self.Area52DeferredImport=nil
        error(message)
    end
    local import=self.Area52DeferredImport
    local queued=self.Area52UpdateQueued
    self.Area52DeferredImport=nil
    self.Area52UpdateQueued=nil
    if import then Area52FeaturedEditor.ImportAssigned(self,false)
    elseif queued then sessionUpdate(self,false) end
end
