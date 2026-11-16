AppearancePlayerModelMixin = CreateFromMixins("ModelMixin", "NineSlicePanelMixin")

local VisualSlots = {
    INVSLOT_HEAD,
    INVSLOT_SHOULDER,
    INVSLOT_BACK,
    INVSLOT_CHEST,
    INVSLOT_BODY,
    INVSLOT_TABARD,
    INVSLOT_WRIST,
    INVSLOT_HAND,
    INVSLOT_WAIST,
    INVSLOT_LEGS,
    INVSLOT_FEET,
    INVSLOT_MAINHAND,
    INVSLOT_OFFHAND,
}

local InventoryLayout = {
    { "TOPLEFT", 12, -56, categoryID = 1 },
    { categoryID = 2 },
    { categoryID = 3 },
    { categoryID = 4 },
    { categoryID = 6 },
    { categoryID = 5 },
    { categoryID = 7 },
    { "TOPRIGHT", -12, -128, categoryID = 8 },
    { categoryID = 9 },
    { categoryID = 10 },
    { categoryID = 11 },
    { "BOTTOM", 53, 28, categoryID = 12 },
    { "BOTTOM", -53, 28, categoryID = 13, illusionCategoryID = 15 },
    { "BOTTOM", 0, 28, categoryID = 14, illusionCategoryID = 16 },
}

local SlotLayout = {
    ["APPEARANCE_TYPE_ITEM"] = InventoryLayout,
    ["APPEARANCE_TYPE_ITEM_SET"] = InventoryLayout,
    ["APPEARANCE_TYPE_OUTFIT"] = InventoryLayout,
    ["APPEARANCE_TYPE_ILLUSION"] = InventoryLayout,
    ["APPEARANCE_TYPE_INCARNATION"] = {
        [1] = { "TOPLEFT", 12, -44 },
        [9] = { "TOPRIGHT", -12, -44 },
    },
    ["APPEARANCE_TYPE_SPELL_VISUAL"] = "GRID",
    ["APPEARANCE_TYPE_COSMETIC_PET"] = "GRID",
    ["APPEARANCE_TYPE_COSMETIC"] = "GRID",
    ["APPEARANCE_TYPE_MOUNT"] = "GRID",
    ["APPEARANCE_TYPE_COMPANION"] = "GRID",
    ["APPEARANCE_TYPE_TOY"] = "GRID",
}

local function ResetSlotButton(pool, button)
    button:Hide()
    button:SetScale(1)
    button:ClearAllPoints()
    button:SetFrameLevel(button:GetParent():GetFrameLevel()+1)
end
function AppearancePlayerModelMixin:OnLoad()
    self:SetPosition(0, 0.12, 0)
    ModelMixin.OnLoad(self)
    self:SetEnableDragRotation(true)
    self:SetEnableScrollZoom(true)
    self:SetEnableDragMove(true)
    self:SetDragScale(0.001)
    self:SetMaxPosOffset(0.2, 0.18)
    self:SetMinMaxDistance(-1, 1)
    
    self.ErrorFrame:SetFrameLevel(self:GetFrameLevel()+10)
    
    self:SetUnit("player")
    self:SetCamera(1)
    self.Background:SetAtlas(AppearanceUtil.GetBackgroundAtlas())
    
    self.SlotButtons = CreateFramePool("CheckButton", self, "AppearanceCategorySlotButtonTemplate", ResetSlotButton)
    
    AppearanceUI:RegisterCallback("OnAppearanceTypeSelected", self.SetAppearanceType, self)
    AppearanceUI:RegisterCallback("OnCategorySelected", self.SetCategory, self)
end

function AppearancePlayerModelMixin:OnShow()
    self:RegisterEvent("APPLY_PENDING_APPEARANCE_RESULT")
    self:RegisterEvent("PENDING_APPEARANCE_CHANGED")
    self:RegisterEvent("VIEWABLE_APPEARANCES_RESET")
    self:RegisterEvent("APPEARANCE_OUTFIT_SAVED_RESULT")
    self:Refresh()
end

function AppearancePlayerModelMixin:OnHide()
    self:UnregisterEvent("APPLY_PENDING_APPEARANCE_RESULT")
    self:UnregisterEvent("PENDING_APPEARANCE_CHANGED")
    self:UnregisterEvent("VIEWABLE_APPEARANCES_RESET")
    self:UnregisterEvent("APPEARANCE_OUTFIT_SAVED_RESULT")
end

function AppearancePlayerModelMixin:InternalUndress()
    SetModelApplyComponents(false)
    self:SetUnit("player")
    SetModelApplyComponents(true)
    self:Undress()
end

function AppearancePlayerModelMixin:APPLY_PENDING_APPEARANCE_RESULT(applied, result)
    self:Refresh()
    if applied then
        self.ErrorFrame:Clear()
        self.ErrorFrame:AddMessage(_G[result] or result, GREEN_FONT_COLOR:GetRGB())
    elseif result == "APPLY_APPEARANCES_COSMETIC_PET_APPEARANCE_CONFIRMATION" then
        self.ErrorFrame:Clear()
        StaticPopup_Show("CONFIRM_APPLY_COSMETIC_PET")
    else
        self.ErrorFrame:AddMessage(_G[result] or result, RED_FONT_COLOR:GetRGB())
    end
end

function AppearancePlayerModelMixin:APPEARANCE_OUTFIT_SAVED_RESULT(applied, result)
    self:Refresh()
    if applied then
        self.ErrorFrame:Clear()
        self.ErrorFrame:AddMessage(_G[result] or result, GREEN_FONT_COLOR:GetRGB())
    else
        self.ErrorFrame:AddMessage(_G[result] or result, RED_FONT_COLOR:GetRGB())
    end
end

function AppearancePlayerModelMixin:PENDING_APPEARANCE_CHANGED()
    self:Refresh()
end

function AppearancePlayerModelMixin:VIEWABLE_APPEARANCES_RESET()
    self:Refresh()
end

function AppearancePlayerModelMixin:Dress(skipCategoryVisual)
    self:InternalUndress()
    for _, slotID in ipairs(VisualSlots) do
        if slotID == INVSLOT_MAINHAND or slotID == INVSLOT_OFFHAND then
            if self.IsRanged then
                if slotID == INVSLOT_MAINHAND then
                    local displayID = self:GetSlotDisplayID(INVSLOT_RANGED)
                    if displayID then
                        self:TryOn(displayID)
                    end
                end
            else
                local displayID, illusionID = self:GetSlotDisplayID(slotID), self:GetSlotIllusionDisplayID(slotID)
                if displayID then
                    self:TryOn(format("item:%d:%d", displayID, illusionID or 0), slotID)
                end
            end
        else
            local displayID = self:GetSlotDisplayID(slotID)
            if displayID then
                self:TryOn(displayID)
            end
        end
    end
    
    if not skipCategoryVisual and self.categoryID > INVSLOT_LAST_EQUIPPED then
        local displayType, displayID, _, _, _, spellVisualID = AppearanceUtil.GetCategoryDisplayInfo(self.categoryID)
        if displayType == "APPEARANCE_DISPLAY_TYPE_ANIMATED_SPELL_VISUAL" or displayType == "APPEARANCE_DISPLAY_TYPE_STATIC_SPELL_VISUAL" then
            self:SetSpellVisual(displayID, spellVisualID or 0)
        end
    end
end

function AppearancePlayerModelMixin:UndressSlot(undressSlotID)
    self:InternalUndress()
    for _, slotID in ipairs(VisualSlots) do
        if slotID ~= undressSlotID then
            if slotID == INVSLOT_MAINHAND or slotID == INVSLOT_OFFHAND then
                if self.IsRanged then
                    if slotID == INVSLOT_MAINHAND then
                        local displayID = self:GetSlotDisplayID(INVSLOT_RANGED)
                        if displayID then
                            self:TryOn(displayID)
                        end
                    end
                else
                    local displayID, illusionID = self:GetSlotDisplayID(slotID), self:GetSlotIllusionDisplayID(slotID)
                    if displayID then
                        self:TryOn(format("item:%d:%d", displayID, illusionID or 0), slotID)
                    end
                end
            else
                local displayID = self:GetSlotDisplayID(slotID)
                if displayID then
                    self:TryOn(displayID)
                end
            end
        end
    end
end

function AppearancePlayerModelMixin:GetSlotDisplayID(slotID)
    return AppearanceUtil.GetSlotDisplayID(slotID)
end

function AppearancePlayerModelMixin:GetSlotIllusionDisplayID(slotID)
    return AppearanceUtil.GetSlotIllusionDisplayID(slotID)
end

function AppearancePlayerModelMixin:GetCategoryDisplayID(categoryID)
    return AppearanceUtil.GetCategoryDisplayID(categoryID)
end

function AppearancePlayerModelMixin:ShowRanged()
    self.IsRanged = true
    local displayID = self:GetSlotDisplayID(INVSLOT_RANGED)
    if displayID then
        self:TryOn(displayID)
    end
end


function AppearancePlayerModelMixin:ShowMelee()
    self.IsRanged = false
    local displayID, illusionID = self:GetSlotDisplayID(INVSLOT_MAINHAND), self:GetSlotIllusionDisplayID(INVSLOT_MAINHAND)
    if displayID then
        self:TryOn(format("item:%d:%d", displayID, illusionID or 0), INVSLOT_MAINHAND)
    end
    displayID, illusionID = self:GetSlotDisplayID(INVSLOT_OFFHAND), self:GetSlotIllusionDisplayID(INVSLOT_OFFHAND)
    if displayID then
        self:TryOn(format("item:%d:%d", displayID, illusionID or 0), INVSLOT_OFFHAND)
    end
end

function AppearancePlayerModelMixin:SetAppearanceType(appearanceType)
    self.appearanceType = appearanceType
    self:UpdateSlotLayout()
end 

function AppearancePlayerModelMixin:SetCategory(categoryID)
    self.categoryID = categoryID
    local slotID = AppearanceUtil.GetCategorySlotID(categoryID)
    if slotID == INVSLOT_RANGED then
        self:ShowRanged()
    elseif slotID == INVSLOT_MAINHAND or slotID == INVSLOT_OFFHAND then
        self:ShowMelee()
    end
    self:UpdateSelectedCategory()
end 

function AppearancePlayerModelMixin:UpdateSlotLayout()
    self.SlotButtons:ReleaseAll()
    local layout = SlotLayout[self.appearanceType]
    local categories
    if self.appearanceType == "APPEARANCE_TYPE_OUTFIT" or self.appearanceType == "APPEARANCE_TYPE_ITEM_SET" or self.appearanceType == "APPEARANCE_TYPE_ILLUSION" then
        categories = C_AppearanceCollection.GetCategoriesForType("APPEARANCE_TYPE_ITEM")
    else
        categories = C_AppearanceCollection.GetCategoriesForType(self.appearanceType)
    end
    if layout == "GRID" then
        local x, y = 20, 38
        for i, categoryID in ipairs(categories) do
            local button = self.SlotButtons:Acquire()
            button:SetPoint("BOTTOMLEFT", x, y)
            button:SetCategory(categoryID)
            button:Show()
            x = x + 72
            if i % 5 == 0 then
                x = 20
                y = y + 58
            end
        end
    else
        local lastButton
        for i, categoryID in ipairs(categories) do
            local button = self.SlotButtons:Acquire()
            local slotLayout = layout[i]
            local point, x, y
            if slotLayout then
                point, x, y = unpack(slotLayout)
            end
            if point then
                button:SetPoint(point, x, y)
            else
                button:SetPoint("TOPLEFT", lastButton, "BOTTOMLEFT", 0, -10)
            end
            lastButton = button
            button:SetCategory(slotLayout and slotLayout.categoryID or categoryID)
            button:Show()
            
            if slotLayout and slotLayout.illusionCategoryID then
                button = self.SlotButtons:Acquire()
                button:SetPoint("CENTER", lastButton, "BOTTOM", 0, -8)
                button:SetCategory(slotLayout.illusionCategoryID)
                button:SetScale(0.5)
                button:SetFrameLevel(lastButton:GetFrameLevel()+5)
                button:Show()
            end
        end
    end
end 

local incarnationPreview = {
    [506] = {19, 100536},
    [507] = {19, 101602},
    [508] = {19, 101603},
    [509] = {19, 101605},
    [510] = {19, 101606},
    [511] = {19, 101607},
    [645] = {18, 328642},
    [646] = {18, 328643},
    [647] = {18, 328644},
    [648] = {18, 328645},
    [649] = {18, 328646},
    [650] = {18, 328647},
    [651] = {18, 328648},
    [652] = {18, 328649},
    [822] = {17, 1083},
    [1456] = {25, 77022},
    [1457] = {25, 77024},
    [1469] = {25, 420559},
    [1472] = {25, 420560},
    [1473] = {25, 420561},
    [1474] = {25, 420562},
    [1475] = {25, 420563},
    [1476] = {25, 420564},
    [1781] = {25, 93168},
    [1782] = {25, 93169},
    [1783] = {25, 93170},
    [1784] = {28, 68316},
    [1786] = {28, 65054},
    [1787] = {28, 65055},
    [1788] = {28, 168316},
    [1790] = {28, 168317},
    [1791] = {28, 168318},
    [1792] = {28, 945402},
    [1794] = {28, 945403},
    [1795] = {25, 500006},
    [1796] = {25, 500011},
    [1797] = {25, 500016},
    [1798] = {25, 500021},
    [1799] = {25, 500022},
    [1800] = {25, 500029},
    [1815] = {29, 77317},
    [1816] = {29, 77318},
    [1817] = {29, 77319},
    [2197] = {29, 48792},
    [2199] = {29, 48793},
    [2200] = {29, 48794},
    [2231] = {29, 48772},
    [2362] = {29, 49230},
    [2363] = {29, 49231},
    [2364] = {29, 49232},
    [2365] = {29, 49233},
    [2366] = {29, 49300},
    [5034] = {68, 80434},
    [5048] = {68, 80435},
    [5740] = {68, 80563},
    [5745] = {68, 80564},
    [6004] = {18, 104981},
    [6005] = {18, 104982},
    [6006] = {18, 104983},
    [6007] = {18, 104984},
    [6008] = {18, 104985},
    [6009] = {18, 104986},
    [10413] = {27, 99003},
    [11704] = {66, -1165961},
    [15595] = {22, 110039},
    [15596] = {22, 110040},
    [15597] = {22, 110041},
    [15598] = {22, 110042},
    [15599] = {22, 110043},
    [15605] = {23, 39801},
    [15606] = {23, 39802},
    [15607] = {23, 39803},
    [15608] = {23, 40908},
    [15609] = {23, 40970},
    [15610] = {23, 40971},
    [15611] = {23, 44650},
    [15641] = {66, -1119029},
    [45401] = {17, 48856},
    [45402] = {17, 48858},
    [45403] = {17, 48859},
    [47124] = {17, 48840},
    [47250] = {17, 48896},
    [47251] = {17, 48897},
    [47522] = {17, 19133},
    [47524] = {17, 1082},
    [47533] = {17, 48895},
    [47536] = {17, 48898},
    [47537] = {17, 99849},
    [47538] = {17, 48849},
    [47539] = {17, 48848},
    [47540] = {17, 48850},
    [47541] = {17, 48851},
    [47542] = {17, 48841},
    [47543] = {17, 48838},
    [47544] = {17, 48837},
    [47545] = {17, 48839},
    [47547] = {17, 48842},
    [47548] = {17, 48836},
    [47549] = {17, 48843},
    [47550] = {17, 48845},
    [47551] = {17, 48844},
    [47552] = {17, 48846},
    [47553] = {17, 48847},
    [47554] = {17, 48852},
    [47555] = {17, 48853},
    [47556] = {17, 48854},
    [47557] = {17, 48855},
    [47558] = {17, 48857},
    [47559] = {17, 48832},
    [47560] = {17, 48834},
    [47561] = {17, 48835},
    [47562] = {17, 48833},
    [47563] = {18, 80020},
    [47565] = {18, 48864},
    [47566] = {18, 48864},
    [47567] = {18, 48864},
    [47568] = {18, 48864},
    [47569] = {18, 74804},
    [47571] = {18, 74806},
    [47572] = {18, 74807},
    [47573] = {18, 74808},
    [47574] = {18, 74809},
    [47575] = {18, 74810},
    [47576] = {18, 74811},
    [47577] = {18, 200004},
    [47578] = {18, 148884},
    [47579] = {18, 38150},
    [47580] = {18, 85194},
    [47581] = {18, 85195},
    [47582] = {18, 85196},
    [47584] = {18, 86100},
    [47585] = {18, 86524},
    [47586] = {18, 86525},
    [47587] = {18, 86526},
    [47588] = {18, 69832},
    [47589] = {18, 69833},
    [47590] = {18, 69834},
    [47591] = {18, 69835},
    [47592] = {18, 48883},
    [47593] = {18, 48884},
    [47595] = {18, 48885},
    [47596] = {18, 48882},
    [47597] = {18, 48881},
    [47598] = {18, 48880},
    [47599] = {18, 48879},
    [47600] = {18, 48874},
    [47601] = {18, 48873},
    [47602] = {18, 48872},
    [47603] = {18, 48871},
    [47604] = {18, 48878},
    [47605] = {18, 48877},
    [47606] = {18, 48876},
    [47607] = {18, 48875},
    [47608] = {18, 48870},
    [47609] = {18, 48869},
    [47610] = {18, 48868},
    [47611] = {18, 48867},
    [47612] = {18, 48866},
    [47613] = {18, 48865},
    [47614] = {18, 48864},
    [47615] = {18, 80719},
    [47616] = {19, 148885},
    [47617] = {19, 48922},
    [47618] = {19, 48949},
    [47619] = {20, 48887},
    [47620] = {20, 48953},
    [47621] = {21, 80084},
    [47622] = {21, 49139},
    [47623] = {21, 48911},
    [47624] = {21, 48912},
    [47625] = {21, 48913},
    [47626] = {21, 48914},
    [47627] = {21, 91215},
    [47628] = {22, 80069},
    [47629] = {22, 96451},
    [47630] = {22, 196451},
    [47631] = {22, 196452},
    [47632] = {22, 196453},
    [47633] = {22, 196454},
    [47634] = {22, 196455},
    [47635] = {22, 196456},
    [47636] = {22, -499397},
    [47637] = {22, 48920},
    [47638] = {22, 48919},
    [47639] = {22, 48941},
    [47640] = {23, 950002},
    [47642] = {23, 950003},
    [47647] = {23, -499396},
    [47648] = {23, 48971},
    [47649] = {23, 48972},
    [47650] = {23, 48973},
    [47651] = {23, 48974},
    [47652] = {23, 48978},
    [47653] = {23, 48977},
    [47654] = {23, 48976},
    [47655] = {23, 48975},
    [47656] = {23, 48925},
    [47657] = {23, 48926},
    [47658] = {23, 48927},
    [47659] = {23, 48928},
    [47660] = {23, 48929},
    [47661] = {23, 48930},
    [47662] = {24, 48987},
    [47663] = {24, 48986},
    [47665] = {24, 48984},
    [47667] = {24, 48982},
    [47669] = {24, 48980},
    [47671] = {25, 48811},
    [47672] = {25, 48811},
    [47673] = {25, 48812},
    [47674] = {25, 64632},
    [47675] = {26, 48734},
    [47677] = {26, 17746},
    [47679] = {26, 17081},
    [47681] = {27, 48817},
    [47683] = {27, 48818},
    [47685] = {27, 99003},
    [47687] = {28, 48805},
    [47689] = {28, 78135},
    [47690] = {29, 48791},
    [47691] = {29, 148772},
    [47692] = {29, 49177},
    [47693] = {29, 18193},
    [51205] = {31, 80370},
    [51206] = {31, 80378},
    [51207] = {31, 64056},
    [51208] = {31, 64560},
    [51209] = {31, 65367},
    [51210] = {31, 66014},
    [51211] = {31, 405062},
    [51212] = {31, 405063},
    [51213] = {31, 405064},
    [51214] = {31, 405065},
    [51215] = {31, 405066},
    [51216] = {31, 405067},
    [51217] = {31, 839260},
    [51218] = {22, 430904},
    [51219] = {22, 430905},
    [51220] = {22, 430906},
    [51221] = {22, 430907},
    [51222] = {22, 430908},
    [51223] = {22, 430909},
    [51224] = {22, 430910},
    [51225] = {24, 48979},
    [51226] = {24, 48981},
    [51227] = {24, 48983},
    [51228] = {24, 48985},
    [51229] = {26, 19681},
    [51230] = {27, 48815},
    [51231] = {28, 48806},
    [51429] = {18, 85197},
    [55023] = {65, 40150},
    [61373] = {26, 10011},
    [61374] = {26, 79785},
    [61375] = {17, 84867},
    [61376] = {17, 84868},
    [61377] = {17, 84869},
    [61378] = {17, 84870},
    [101266] = {27, 20387},
    [147691] = {29, 148771},
    [179673] = {23, 95612},
    [179674] = {23, 95613},
    [179675] = {23, 96774},
    [179676] = {23, 96775},
    [179677] = {23, 96777},
    [179678] = {23, 196779},
    [179679] = {19, 104465},
    [179680] = {19, 105031},
    [179681] = {19, 106361},
    [179682] = {19, 108451},
    [211705] = {67, -500981},
    [261377] = {17, 212874},
    [261378] = {17, 212875},
    [261379] = {17, 212876},
    [261380] = {17, 212877},
    [261381] = {17, 212878},
    [261382] = {17, 212879},
}

function AppearancePlayerModelMixin:PreviewIncarnation(id)
    local entry = incarnationPreview[id]
    if self.appearanceType ~= "APPEARANCE_TYPE_INCARNATION" or not entry or entry[1] ~= self.categoryID then
        return false
    end
    local request = {}
    self.incarnationPreviewRequest = request
    if entry[2] < 0 then
        self:SetUnit("player")
        local _, _, _, _, _, visual = C_Appearance.GetAppearanceDisplayInfo(id)
        self:Dress(true)
        self:SetSpellVisual(-entry[2], visual or 0)
    else
        self:SetDisplayInfo(entry[2], function()
            if self.incarnationPreviewRequest ~= request then return end
            for _, item in ipairs(C_Appearance.GetCreatureDisplayItems(id) or {}) do
                local pending = Item:CreateFromID(item)
                RunNextFrame(function()
                    if self.incarnationPreviewRequest ~= request then return end
                    pending:CancelableContinueOnLoad(function(itemID)
                        if self.incarnationPreviewRequest == request then
                            self:TryOn(itemID)
                        end
                    end)
                end)
            end
        end)
    end
    return true
end

function AppearancePlayerModelMixin:UpdateModel()
    self.incarnationPreviewRequest = nil
    if self.cancelToken then
        self.cancelToken()
    end

    if self.appearanceType == "APPEARANCE_TYPE_ITEM" then
        self:SetUnit("player")
        self:Dress()
    elseif self.appearanceType == "APPEARANCE_TYPE_ITEM_SET" then
        self:SetUnit("player")
        self:Dress()
    elseif self.appearanceType == "APPEARANCE_TYPE_OUTFIT" then
        self:SetUnit("player")
        self:Dress()
    elseif self.appearanceType == "APPEARANCE_TYPE_ILLUSION" then
        self:SetUnit("player")
        self:Dress()
    elseif self.appearanceType == "APPEARANCE_TYPE_INCARNATION" then
        if not self:PreviewIncarnation(AppearanceUtil.GetCategoryAppearanceID(self.categoryID)) then
            self:SetCreatureDisplay(self:GetCategoryDisplayID(self.categoryID))
        end
    elseif self.appearanceType == "APPEARANCE_TYPE_SPELL_VISUAL" then
        self:SetUnit("player")
        self:Dress()
    elseif self.appearanceType == "APPEARANCE_TYPE_COSMETIC_PET" then
        self:SetCreatureDisplay(self:GetCategoryDisplayID(self.categoryID))
    elseif self.appearanceType == "APPEARANCE_TYPE_COSMETIC" then
        self:SetUnit("player")
        self:Dress()
    end
end

function AppearancePlayerModelMixin:SetCreatureDisplay(creatureID)
    self:SetDisplayInfo(creatureID, function()
        local appearanceID = AppearanceUtil.GetCategoryAppearanceID(self.categoryID)
        local displayItems = C_Appearance.GetCreatureDisplayItems(appearanceID)
        if displayItems and #displayItems > 0 then
            for _, itemID in ipairs(displayItems) do
                self:TryOn(itemID)
            end
        end
    end)
end

function AppearancePlayerModelMixin:UpdateSelectedCategory()
    self:UpdateModel()
    for button in self.SlotButtons:EnumerateActive() do
        button:SetChecked(button.categoryID == self.categoryID)
    end
end

function AppearancePlayerModelMixin:Refresh()
    local hasPending = C_Appearance.HasPendingAppearances()

    local canApply, reason = C_Appearance.CanApplyPendingAppearances()
    local hasInvalid = hasPending and not canApply

    if hasInvalid then
        self.ApplyButton:SetPoint("TOP", -126, -14)
        self.CancelButton:SetPoint("TOP", 126, -14)
    else
        self.ApplyButton:SetPoint("TOP", -64, -14)
        self.CancelButton:SetPoint("TOP", 64, -14)
    end

    self.ApplyButton:SetShown(hasPending)
    self.ApplyButton:SetEnabled(canApply)
    if self.appearanceType == "APPEARANCE_TYPE_COSMETIC_PET" then
        self.ApplyButton:SetText(SUMMON_PET)
    else
        self.ApplyButton:SetText(APPLY)
    end

    self.ApplyError:SetShown(hasInvalid)
    self.ApplyError:SetText(_G[reason] or reason)

    self.ClearInvalidButton:SetShown(hasInvalid)
    self.ClearInvalidButton:SetEnabled(not canApply)
    
    self.CancelButton:SetShown(hasPending)
    self.CancelButton:Enable()
    
    local hasOutfit = C_AppearanceOutfit.GetCurrentOutfitName() ~= nil
    self.SaveOutfitButton:SetShown(not hasPending and not hasOutfit)
    
    self:UpdateModel()
end 