#include "SolveTab.h"

SolveTab SolveTab::_instance = SolveTab();

SolveTab::SolveTab() {
    //ctor
}

SolveTab::~SolveTab() {
    //dtor
}

void SolveTab::draw() {
    ImGui::BeginGroup();
    drawJobDropdown();

    if (_selectedJob) {
        ImGui::SameLine();
        drawSolveButton();

        drawOvermeldCheckbox();
    }
    ImGui::EndGroup();

    if (_selectedJob) {
        drawLimitBaseParamGroups();

        if (ImGui::BeginTabBar("SolveTabBar")) {
            if (ImGui::BeginHandTabItem("Gear")) {
                drawGearTab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginHandTabItem("Food")) {
                drawFoodTab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginHandTabItem("Results", nullptr, _selectResultsTab ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None)) {
                _selectResultsTab = false;
                drawResultsTab();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
}

void SolveTab::drawGearTab() {
    if (_gearItemLevelFilter[0] != -1 && _gearItemLevelFilter[1] != -1) {
        ImGui::Text("iLevel");
        ImGui::SameLine();
        ImGui::PushItemWidth(100);
        ImGui::InputInt2("##GearItemLevel", _gearItemLevelFilter);
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            selectGearItemLvl();
        }
    }

    ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp;
    ImVec2 outer_size = ImVec2(0.0f, min(ImGui::GetFrameHeight() + _gearPiecesToDisplayCount * (ImGui::GetTextLineHeight() + ImGui::GetStyle().CellPadding.y * 2.0f) + (_gearPiecesToDisplaySlotCount - 1) * 8 - 2, ImGui::GetContentRegionAvail().y));
    if (_gearPiecesToDisplayCount > 0 && ImGui::BeginTable("ItemsTable", static_cast<int>(_gearColumnHeaders.size()), flags, outer_size)) {
        ImGui::TableSetupScrollFreeze(0, 1);
        for (const auto& header : _gearColumnHeaders) {
            ImGui::TableSetupColumn(header.c_str());
        }
        ImGui::TableHeadersRow();

        for (int slot = 1; slot < 13; slot++) {
            const auto& gearPieces = _gearPiecesToDisplay[slot];
            if (gearPieces.size() == 0) continue;
            for (int idx = 0; idx < gearPieces.size(); idx++) {
                const auto item = gearPieces[idx];
                map<int, int> baseParamToIdx;
                for (int i = 0; i < 6; i++) {
                    if (item->baseParam[i] != 0) {
                        baseParamToIdx[item->baseParam[i]] = i;
                    }
                }
                int col = 0;
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(col++);
                ImGui::TextUnformatted(GearPiece::equipSlotName.at(item->equipSlotCategory).c_str());
                ImGui::TableSetColumnIndex(col++);
                const bool isSelected = _selectedGearPieces[slot].contains(idx);
                if (ImGui::HandSelectable(item->name.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns)) {
                    if (isSelected) {
                        _selectedGearPieces[slot].erase(idx);
                    } else {
                        _selectedGearPieces[slot].insert(idx);
                    }
                    updateBaseParamRanges();
                }
                if (!isSelected) ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.4f)));

                if (item->requiredItems.size() > 0 && ImGui::BeginItemTooltip()) {
                    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                    string tooltipText = "";
                    for (auto& [shopId, costMap] : item->requiredItems) {
                        if (shopId != item->requiredItems.begin()->first) tooltipText += "\n";

                        for (auto [requiredItem, requiredItemCount] : costMap) {
                            tooltipText += to_string(requiredItemCount) + "x " + requiredItem->name + "\n";
                        }
                    }
                    ImGui::TextUnformatted(tooltipText.c_str());
                    ImGui::PopTextWrapPos();
                    ImGui::EndTooltip();
                }
                
                for (const auto& baseParam : _gearBaseParamToDisplay) {
                    ImGui::TableSetColumnIndex(col++);
                    if (baseParam == 12) {
                        ImGui::TextUnformatted(to_string(item->damagePhys).c_str());
                    } else if (baseParam == 13) {
                        ImGui::TextUnformatted(to_string(item->damageMag).c_str());
                    } else {
                        int baseParamValue = baseParamToIdx.contains(baseParam) ? item->baseParamValue[baseParamToIdx[baseParam]] : 0;
                        ImGui::TextUnformatted(to_string(baseParamValue).c_str());
                    }
                }
                ImGui::TableSetColumnIndex(col++);
                ImGui::TextUnformatted(to_string(item->levelItem).c_str());
                ImGui::TableSetColumnIndex(col++);
                ImGui::TextUnformatted(to_string(item->isAdvancedMeldingPermitted && _allowOvermelds ? 5 : item->materiaSlotCount).c_str());
            }
            if (slot < 12) {
                ImGui::TableNextRow();
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4(0.23f, 0.23f, 0.25f, 1.0f)));
                ImGui::TableNextRow();
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4(0.23f, 0.23f, 0.25f, 1.0f)));
            }
        }
        ImGui::EndTable();
    }
}

void SolveTab::drawFoodTab() {
    if (_foodItemLevelFilter[0] != -1 && _foodItemLevelFilter[1] != -1) {
        ImGui::Text("iLevel");
        ImGui::SameLine();
        ImGui::PushItemWidth(100);
        ImGui::InputInt2("##FoodItemLevel", _foodItemLevelFilter);
        if (ImGui::IsItemEdited()) {
            selectFoodItemLvl();
        }
    }

    ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp;
    ImVec2 outer_size = ImVec2(0.0f, min(ImGui::GetFrameHeight() + _foodToDisplay.size() * (ImGui::GetTextLineHeight() + ImGui::GetStyle().CellPadding.y * 2.0f) - 2, ImGui::GetContentRegionAvail().y));
    if (_foodToDisplay.size() > 0 && ImGui::BeginTable("FoodTable", static_cast<int>(_foodColumnHeaders.size()), flags, outer_size)) {
        ImGui::TableSetupScrollFreeze(0, 1);
        for (const auto& header : _foodColumnHeaders) {
            ImGui::TableSetupColumn(header.c_str());
        }
        ImGui::TableHeadersRow();

        for (int idx = 0; idx < _foodToDisplay.size(); idx++) {
            const auto food = _foodToDisplay[idx];
            map<int, int> baseParamToIdx;
            for (int i = 0; i < 6; i++) {
                if (food->baseParam[i] != 0) {
                    baseParamToIdx[food->baseParam[i]] = i;
                }
            }
            int col = 0;
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(col++);
            const bool isSelected = _selectedFoodIdx.contains(idx);
            if (ImGui::HandSelectable(food->name.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns)) {
                if (isSelected) {
                    _selectedFoodIdx.erase(idx);
                } else {
                    _selectedFoodIdx.insert(idx);
                }
                updateBaseParamRanges();
            }
            if (!isSelected) ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.4f)));
            
            for (const auto& baseParam : _foodBaseParamToDisplay) {
                ImGui::TableSetColumnIndex(col++);
                int baseParamValue = baseParamToIdx.contains(baseParam) ? food->valueHQ[baseParamToIdx[baseParam]] : 0;
                int baseParamMax = baseParamToIdx.contains(baseParam) ? food->maxHQ[baseParamToIdx[baseParam]] : 0;
                string textToDisplay = baseParamToIdx.contains(baseParam) ?
                    "+" + to_string(baseParamValue) + "% (Max " + to_string(baseParamMax) + ")" :
                    "";
                ImGui::TextUnformatted(textToDisplay.c_str());
            }
            ImGui::TableSetColumnIndex(col++);
            ImGui::TextUnformatted(to_string(food->levelItem).c_str());
        }
        ImGui::EndTable();
    }
}

void SolveTab::drawResultsTab() {
    if (SetBuilder::Instance().isSolving) {
        ImGui::ProgressBar((float)SetBuilder::Instance().solvingProgress, ImVec2(200.0f, 0.0f));

        ImGui::SameLine();
        auto cursorX = ImGui::GetCursorPosX();
        drawTime("Elapsed: ", SetBuilder::Instance().elapsed);

        ImGui::SameLine();
        // Offset to leave enough space for the text to not constantly move
        auto nextCursorX = cursorX + ImGui::CalcTextSize("Elapsed: 9999m99s999").x + ImGui::GetStyle().ItemSpacing.x;
        if (nextCursorX > ImGui::GetCursorPosX()) ImGui::SetCursorPosX(nextCursorX);
        cursorX = ImGui::GetCursorPosX();
        drawTime("Remaining: ", SetBuilder::Instance().estimatedRemaining);

        ImGui::SameLine();
        // Offset to leave enough space for the text to not constantly move
        nextCursorX = cursorX + ImGui::CalcTextSize("Remaining: 9999m99s999").x + ImGui::GetStyle().ItemSpacing.x;
        if (nextCursorX > ImGui::GetCursorPosX()) ImGui::SetCursorPosX(nextCursorX);
        ImGui::TextUnformatted(("Active threads: " + to_string(SetBuilder::Instance().activeThreads) + " / " + to_string(SetBuilder::Instance().maxHardwareThreads)).c_str());
    } else if (SetBuilder::Instance().solvingTime > 0) {
        drawTime("Solving done in: ", SetBuilder::Instance().solvingTime);
    }

    if (SetBuilder::Instance().results.size() > 0 && !SetBuilder::Instance().isSolving) {
        ImGui::SameLine();
        const char* exportAll = "XivGear Export All";
        float off = (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(exportAll).x - ImGui::GetStyle().FramePadding.x * 2);
        if (off > 0.0f) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
        drawClipboardButton(exportAll, [this] { return resultsToJson(); });
    }

    ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp;
    ImVec2 outer_size = ImVec2(0.0f, min(ImGui::GetFrameHeight() + SetBuilder::Instance().results.size() * (ImGui::GetTextLineHeight() + ImGui::GetStyle().CellPadding.y * 2.0f) - 2, ImGui::GetContentRegionAvail().y));
    if (SetBuilder::Instance().results.size() > 0 && ImGui::BeginTable("ResultsTable", static_cast<int>(_resultsColumnHeaders.size()), flags, outer_size)) {
        ImGui::TableSetupScrollFreeze(0, 1);
        for (const auto& header : _resultsColumnHeaders) {
            ImGui::TableSetupColumn(header.c_str());
        }
        ImGui::TableHeadersRow();

        ImGui::BeginDisabled(SetBuilder::Instance().isSolving);
        for (auto& result : SetBuilder::Instance().results) {
            int col = 0;
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(col++);
            if (ImGui::HandSelectable(result.second.gcdStr().c_str(), false, ImGuiSelectableFlags_SpanAllColumns) && !SetBuilder::Instance().isSolving) {
                selectResult(result.second);
            }
            if (_selectedResult.gcdInt == result.second.gcdInt) {
                drawSelectedResultModal();
            }
            ImGui::TableSetColumnIndex(col++);
            ImGui::TextUnformatted(to_string(result.second.damageMod).c_str());
            for (const auto& baseParam : _resultsBaseParamToDisplay) {
                ImGui::TableSetColumnIndex(col++);
                if (baseParam == 12) {
                    ImGui::TextUnformatted(to_string(result.second.damagePhys).c_str());
                } else if (baseParam == 13) {
                    ImGui::TextUnformatted(to_string(result.second.damageMag).c_str());
                } else if (baseParam <= 5) {
                    ImGui::TextUnformatted(to_string(result.second.mainBaseParamValue).c_str());
                } else {
                    ImGui::TextUnformatted(to_string(result.second.fedMeldedBaseParamValue[baseParam]).c_str());
                }
            }
            ImGui::TableSetColumnIndex(col++);
            string foodName = "";
            if (result.second.selectedFood != nullptr) {
                foodName = result.second.selectedFood->name;
            }
            ImGui::TextUnformatted(foodName.c_str());
        }
        ImGui::EndDisabled();
        ImGui::EndTable();
    }
}

void SolveTab::drawTime(string preText, int64_t timeValue) {
    stringstream timeText;
    timeText << preText
        << (int64_t)(timeValue / 1000 / 60) << "m"
        << setw(2) << (int64_t)(timeValue / 1000) % 60 << "s"
        << setw(3) << timeValue % 1000;
    ImGui::TextUnformatted(timeText.str().c_str());
}

void SolveTab::selectResult(const GearSet& result) {
    _selectedResult = result;
    ImGui::OpenPopup("SelectedResultPopup");
    _selectedResultSlots = 0;
    for (auto perm : _selectedResult.meldPerms) {
        int slot = 0;
        while (slot < 5) {
            if (perm->materia[slot] == nullptr) {
                break;
            }
            slot++;
        }
        if (slot > _selectedResultSlots) {
            _selectedResultSlots = slot;
        }
    }
}

void SolveTab::drawSelectedResultModal() {
    ImGui::PushStyleVarX(ImGuiStyleVar_WindowPadding, 16.0f);
    ImGui::PushStyleVarX(ImGuiStyleVar_CellPadding, 12.0f);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.059f, 0.059f, 0.059f, 1.0f));

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("SelectedResultPopup", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize)) {
        if (ImGui::BeginTable("SelectedResultTable", _selectedResultSlots + 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg)) {
            for (auto perm : _selectedResult.meldPerms) {
                ImGui::TableNextRow();
                int col = 0;
                ImGui::TableSetColumnIndex(col++);
                ImGui::TextUnformatted(GearPiece::equipSlotName.at(perm->gearPiece->equipSlotCategory).c_str());
                ImGui::TableSetColumnIndex(col++);
                ImGui::Text(perm->gearPiece->name.c_str());
                for (int slot = 0; slot < 5; slot++) {
                    auto materia = perm->materia[slot];
                    if (materia == nullptr) break;
                    ImGui::TableSetColumnIndex(col++);
                    stringstream materiaText;
                    materiaText << BaseParam::abbr.at(materia->baseParam) << " " << materia->grade;
                    ImGui::Text(materiaText.str().c_str());
                }
            }
            ImGui::EndTable();
        }

        ImGui::Separator();
        float off = (ImGui::GetContentRegionAvail().x - 320 - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
        if (off > 0.0f) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
        drawClipboardButton("XivGear Export", [this] { return _selectedResult.toJson(); }, ImVec2(160, 0));

        ImGui::SameLine();
        if (ImGui::HandButton("Close", ImVec2(160, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::SetItemDefaultFocus();

        ImGui::EndPopup();
    }

    ImGui::PopStyleColor(1);
    ImGui::PopStyleVar(2);
}

void SolveTab::drawJobDropdown() {
    ImGui::Text("Job");
    ImGui::SameLine();
    ImGui::PushItemWidth(100);
    if (ImGui::BeginHandCombo("##Job", _selectedJobName.c_str())) {
        for (auto& job : Data::Instance().jobList) {
            const bool isSelected = (_selectedJobName == job.name);
            if (ImGui::HandSelectable(job.name.c_str(), isSelected)) {
                _selectedJobName = job.name;
                selectJob();
            }
            if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
}

void SolveTab::drawSolveButton() {
    vector<string> solveButtonErrorMessages;
    if (_selectedJob == nullptr) solveButtonErrorMessages.push_back("No job selected");
    for (auto it : _selectedGearPieces) {
        if (it.first == 6) continue;
        if (it.first == 2 && _selectedJob != nullptr && _selectedJob->name == "PLD" && it.second.size() > 0) continue;
        if (it.first == 12 && (it.second.size() > 1 || it.second.size() == 1 && !_gearPiecesToDisplay[it.first][*it.second.begin()]->isUnique)) continue;
        if (it.first != 12 && it.second.size() > 0) continue;
        solveButtonErrorMessages.push_back("Not enough " + GearPiece::equipSlotName.at(it.first) + " pieces selected");
    }
    if (_selectedFoodIdx.size() == 0) solveButtonErrorMessages.push_back("No food selected");
    if (SetBuilder::Instance().isSolving) solveButtonErrorMessages.push_back("Solve is already ongoing");
    ImGui::BeginDisabled(solveButtonErrorMessages.size() > 0);
    if (ImGui::HandButton("Solve")) {
        filterSelection();
        if (solveButtonErrorMessages.size() == 0) {
            _selectResultsTab = true;
            _resultGearItemLevelFilter[0] = _gearItemLevelFilter[0];
            _resultGearItemLevelFilter[1] = _gearItemLevelFilter[1];
            _resultFoodItemLevelFilter[0] = _foodItemLevelFilter[0];
            _resultFoodItemLevelFilter[1] = _foodItemLevelFilter[1];
			SetBuilder::Instance().setBaseParamRanges(_limitBaseParam, _baseParamRangeSelected);
            SetBuilder::Instance().startSolve(_selectedJob, 100, _gearPiecesFiltered, _foodFiltered, _releventMateriaBaseParam);
            // TODO: pass selected level
        }
    }
    if (solveButtonErrorMessages.size() > 0) {
        stringstream tooltipText;
        for (int i = 0; i < solveButtonErrorMessages.size(); i++) {
            tooltipText << solveButtonErrorMessages[i];
            if (i < solveButtonErrorMessages.size() - 1) {
                tooltipText << endl;
            }
        }
        ImGui::SetItemTooltip(tooltipText.str().c_str());
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::BeginDisabled(!SetBuilder::Instance().isSolving);
    if (ImGui::HandButton("Cancel")) {
        SetBuilder::Instance().cancelSolve();
    }
    ImGui::EndDisabled();
}

void SolveTab::drawOvermeldCheckbox() {
    ImGui::Checkbox("Allow Overmelds", &_allowOvermelds);
    if (ImGui::IsItemEdited()) {
        updateGearPiecesToDisplayMeldPerms();
    }
}

void SolveTab::drawLimitBaseParamGroups() {
    for (auto& [baseParam, isLimited] : _limitBaseParam) {
        ImGui::SameLine();
        ImGui::BeginGroup();
        auto baseParamAbbr = BaseParam::abbr.at(baseParam);
        ImGui::Checkbox(("Limit " + baseParamAbbr).c_str(), &isLimited);
        if (ImGui::IsItemEdited() && !isLimited) {
            setSelectedRangeToAllowed(baseParam);
        }
        ImGui::BeginDisabled(!isLimited);
        ImGui::PushItemWidth(150);
        string modText;
        string modFormat;
        switch (baseParam) {
            case BaseParam::Tenacity:
                modText = "Dmg Mod";
                modFormat = "%.3f";
                break;
            case BaseParam::Piety:
                modText = "MP Tick";
                modFormat = "%.0f";
                break;
            case BaseParam::SkillSpeed:
            case BaseParam::SpellSpeed:
                modText = "GCD";
                modFormat = "%.2f";
                break;
            default:
                break;
        }
        auto& modRangeSelected = _baseParamModRangeSelected[baseParam];
        auto& modRangeAllowed = _baseParamModRangeAllowed[baseParam];
        auto& valueRangeSelected = _baseParamRangeSelected[baseParam];
        auto& valueRangeAllowed = _baseParamRangeAllowed[baseParam];
        ImGui::SliderFloat(("##" + modText + "Min").c_str(), &modRangeSelected.first, modRangeAllowed.first, modRangeSelected.second, modFormat.c_str(), ImGuiSliderFlags_AlwaysClamp); // TODO: combined slider
        if (ImGui::IsItemEdited()) {
            int newValue;
            switch (baseParam) {
                case BaseParam::Tenacity:
                    newValue = Damage::Instance().tenModIntToMin((int)(round(modRangeSelected.first * 1000)));
                    break;
                case BaseParam::Piety:
                    newValue = Damage::Instance().pieModToMin((int)modRangeSelected.first);
                    break;
                case BaseParam::SkillSpeed:
                case BaseParam::SpellSpeed:
                    newValue = Damage::Instance().gcdToMinSpeed((int)(round(modRangeSelected.first * 100)));
                    break;
                default:
                    break;
            }
            valueRangeSelected.first = min(max(newValue, valueRangeAllowed.first), valueRangeAllowed.second);
        }
        ImGui::SameLine();
        ImGui::SliderFloat((modText + "##" + modText + "Max").c_str(), &modRangeSelected.second, modRangeSelected.first, modRangeAllowed.second, modFormat.c_str(), ImGuiSliderFlags_AlwaysClamp); // TODO: combined slider
        if (ImGui::IsItemEdited()) {
            int newValue;
            switch (baseParam) {
                case BaseParam::Tenacity:
                    newValue = Damage::Instance().tenModIntToMax((int)(round(modRangeSelected.second * 1000)));
                    break;
                case BaseParam::Piety:
                    newValue = Damage::Instance().pieModToMax((int)modRangeSelected.second);
                    break;
                case BaseParam::SkillSpeed:
                case BaseParam::SpellSpeed:
                    newValue = Damage::Instance().gcdToMaxSpeed((int)(round(modRangeSelected.second * 100)));
                    break;
                default:
                    break;
            }
            valueRangeSelected.second = min(max(newValue, valueRangeAllowed.first), valueRangeAllowed.second);
        }
        ImGui::PushItemWidth(150);
        ImGui::InputInt(("##" + baseParamAbbr + "Min").c_str(), &valueRangeSelected.first);
        if (ImGui::IsItemEdited()) {
            valueRangeSelected.first = min(valueRangeSelected.first, valueRangeSelected.second);
            valueRangeSelected.first = min(max(valueRangeSelected.first, valueRangeAllowed.first), valueRangeAllowed.second);
            modRangeSelected.first = getModToDisplay(baseParam, valueRangeSelected.first);
        }
        ImGui::SameLine();
        ImGui::InputInt((baseParamAbbr + "##" + baseParamAbbr + "Max").c_str(), &valueRangeSelected.second);
        if (ImGui::IsItemEdited()) {
            valueRangeSelected.second = max(valueRangeSelected.first, valueRangeSelected.second);
            valueRangeSelected.second = min(max(valueRangeSelected.second, valueRangeAllowed.first), valueRangeAllowed.second);
            modRangeSelected.second = getModToDisplay(baseParam, valueRangeSelected.second);
        }
        ImGui::EndDisabled();
        ImGui::EndGroup();
    }
}

void SolveTab::drawClipboardButton(const char* label, function<string()> textProvider, const ImVec2& size) {
    if (ImGui::HandButton(label, size)) {
        ImGui::LogToClipboard();
        ImGui::LogText(textProvider().c_str());
        ImGui::LogFinish();
        _clipboardExportUntil = ImGui::GetTime() + 1.0;
    }
    if (ImGui::BeginItemTooltip()) {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        const bool copied = ImGui::GetTime() < _clipboardExportUntil;
        ImGui::TextUnformatted(copied ? "Copied to Clipboard!": "Copy to Clipboard");
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void SolveTab::selectJob() {
    _selectedJob = &(*find_if(Data::Instance().jobList.begin(), Data::Instance().jobList.end(), [this](const Job& j) { return j.name == _selectedJobName; }));

    setReleventStats();
    setColumnHeaders();

    initGearList(false);
    initFoodList(false);

	Damage::Instance().init(_selectedJob, 100); // TODO: pass max values from gear/food selection
    initBaseParamLimits();
}

void SolveTab::initBaseParamLimits() {
    _limitBaseParam.clear();
    _baseParamModRangeSelected.clear();
    _baseParamModRangeAllowed.clear();
    _baseParamRangeSelected.clear();
    _baseParamRangeAllowed.clear();

    for (auto& baseParam : _releventMateriaBaseParam) {
        if (baseParam != BaseParam::Tenacity
         && baseParam != BaseParam::Piety
         && baseParam != BaseParam::SkillSpeed
         && baseParam != BaseParam::SpellSpeed) continue;

        _limitBaseParam[baseParam] = false;
        updateBaseParamRange(baseParam);
        // Init selection to max allowed range
        _baseParamRangeSelected[baseParam].first = _baseParamRangeAllowed[baseParam].first;
        _baseParamRangeSelected[baseParam].second = _baseParamRangeAllowed[baseParam].second;
        _baseParamModRangeSelected[baseParam].first = _baseParamModRangeAllowed[baseParam].first;
        _baseParamModRangeSelected[baseParam].second = _baseParamModRangeAllowed[baseParam].second;
    }
}

void SolveTab::updateBaseParamRanges() {
    for (auto& [baseParam, _] : _limitBaseParam) {
        updateBaseParamRange(baseParam);
    }
}

void SolveTab::updateBaseParamRange(int baseParam) {
    auto& modRangeSelected = _baseParamModRangeSelected[baseParam];
    auto& modRangeAllowed = _baseParamModRangeAllowed[baseParam];
    auto& valueRangeSelected = _baseParamRangeSelected[baseParam];
    auto& valueRangeAllowed = _baseParamRangeAllowed[baseParam];

    valueRangeAllowed.first = Damage::Instance().getStartingValue(baseParam);
    valueRangeAllowed.second = Damage::Instance().getStartingValue(baseParam);

    // Add range from gear
    for (int slot = 1; slot < 13; slot++) {
        if (!_selectedGearPieces.contains(slot)) continue;

        int slotMinValue = INT32_MAX;
        int slotSecondMinValue = INT32_MAX;
        int slotMaxValue = 0;
        int slotSecondMaxValue = 0;
        for (auto idx : _selectedGearPieces[slot]) {
			auto piece = _gearPiecesToDisplay[slot][idx];
            int pieceMinValue = piece->minBaseParamValue[baseParam];
            int pieceMaxValue = piece->maxBaseParamValue[baseParam];
			if (pieceMinValue <= slotMinValue) {
                slotSecondMinValue = piece->isUnique ? slotMinValue : pieceMinValue;
                slotMinValue = pieceMinValue;
            }
            if (pieceMaxValue >= slotMaxValue) {
                slotSecondMaxValue = piece->isUnique ? slotMaxValue : pieceMaxValue;
                slotMaxValue = pieceMaxValue;
			}
        }

        valueRangeAllowed.first += slotMinValue < INT32_MAX ? slotMinValue : 0;
        valueRangeAllowed.second += slotMaxValue;
        if (slot == 12) {
            valueRangeAllowed.first += slotSecondMinValue < INT32_MAX ? slotSecondMinValue : 0;
            valueRangeAllowed.second += slotSecondMaxValue;
        }
    }

    // Add range from food
    pair<int, int> foodRange = make_pair(_selectedFoodIdx.size() > 0 ? INT32_MAX : 0, 0);
    for (auto idx : _selectedFoodIdx) {
        auto food = _foodToDisplay[idx];
        pair<int, int> foodValue = make_pair(0, 0);
        for (int i = 0; i < 3; i++) {
            if (food->baseParam[i] != baseParam) continue;
            foodValue.first = min((int)(valueRangeAllowed.first * 0.01f * food->valueHQ[i]), food->maxHQ[i]);
            foodValue.second = min((int)(valueRangeAllowed.second * 0.01f * food->valueHQ[i]), food->maxHQ[i]);
        }
        if (foodValue.first < foodRange.first) {
            foodRange.first = foodValue.first;
		}
        if (foodValue.second > foodRange.second) {
            foodRange.second = foodValue.second;
        }
	}
    valueRangeAllowed.first += foodRange.first;
    valueRangeAllowed.second += foodRange.second;

    // Set allowed mod range
    switch (baseParam) {
        case BaseParam::Tenacity:
            modRangeAllowed.first = Damage::Instance().tenMod(valueRangeAllowed.first);
            modRangeAllowed.second = Damage::Instance().tenMod(valueRangeAllowed.second);
            break;
        case BaseParam::Piety:
            modRangeAllowed.first = (float)Damage::Instance().pieMod(valueRangeAllowed.first);
            modRangeAllowed.second = (float)Damage::Instance().pieMod(valueRangeAllowed.second);
            break;
        case BaseParam::SkillSpeed:
        case BaseParam::SpellSpeed:
            modRangeAllowed.first = Damage::Instance().gcd(valueRangeAllowed.first) / 100.0f;
            modRangeAllowed.second = Damage::Instance().gcd(valueRangeAllowed.second) / 100.0f;
            break;
        default:
            break;
    }

    if (_limitBaseParam[baseParam]) {
        // Clamp selection to allowed range
        valueRangeSelected.first = max(valueRangeAllowed.first, valueRangeSelected.first);
        valueRangeSelected.second = max(valueRangeSelected.first, valueRangeSelected.second);
        valueRangeSelected.second = min(valueRangeAllowed.second, valueRangeSelected.second);
        valueRangeSelected.first = min(valueRangeSelected.first, valueRangeSelected.second);

        modRangeSelected.first = getModToDisplay(baseParam, valueRangeSelected.first);
        modRangeSelected.second = getModToDisplay(baseParam, valueRangeSelected.second);
    } else {
        // Set selection to allowed range
        setSelectedRangeToAllowed(baseParam);
    }
}

void SolveTab::setSelectedRangeToAllowed(int baseParam) {
    auto& modRangeSelected = _baseParamModRangeSelected[baseParam];
    auto& modRangeAllowed = _baseParamModRangeAllowed[baseParam];
    auto& valueRangeSelected = _baseParamRangeSelected[baseParam];
    auto& valueRangeAllowed = _baseParamRangeAllowed[baseParam];

    valueRangeSelected.first = valueRangeAllowed.first;
    valueRangeSelected.second = valueRangeAllowed.second;

    modRangeSelected.first = getModToDisplay(baseParam, valueRangeSelected.first);
    modRangeSelected.second = getModToDisplay(baseParam, valueRangeSelected.second);
}

float SolveTab::getModToDisplay(const int& baseParam, const int& value) {
    switch (baseParam) {
        case BaseParam::Tenacity:
            return Damage::Instance().tenMod(value);
        case BaseParam::Piety:
            return (float)Damage::Instance().pieMod(value);
        case BaseParam::SkillSpeed:
        case BaseParam::SpellSpeed:
            return Damage::Instance().gcd(value) / 100.0f;
        default:
            return .0f;
    }
}

void SolveTab::initGearList(bool updateRanges) {
    _selectedJobGearPieces.clear();

    int maxItemLvl = 0;
    for (auto& [id, gearPiece] : Data::Instance().gearPieceList) {
        if (!_selectedJob->categories.contains(gearPiece.classJobCategory)) continue;

        if (gearPiece.levelItem > maxItemLvl) {
            maxItemLvl = gearPiece.levelItem;
        }
        _selectedJobGearPieces.push_back(&gearPiece);
    }

    if (_gearItemLevelFilter[0] == -1 || _gearItemLevelFilter[1] == -1) {
        _gearItemLevelFilter[0] = maxItemLvl - 5;
        _gearItemLevelFilter[1] = maxItemLvl;
    }

    selectGearItemLvl(updateRanges);
}

void SolveTab::initFoodList(bool updateRanges) {
    _selectedJobFood.clear();
    int maxItemLvl = 0;
    for (auto& food : Data::Instance().foodList) {
        bool ignore = false;
        for (int i = 0; i < 3; i++) {
            if (food.baseParam[i] == 3) continue; // Vitality is not displayed
            if (find(_foodBaseParamToDisplay.begin(), _foodBaseParamToDisplay.end(), food.baseParam[i]) == _foodBaseParamToDisplay.end()) {
                ignore = true;
            }
        }
        if (ignore) continue;

        if (food.levelItem > maxItemLvl) {
            maxItemLvl = food.levelItem;
        }
        _selectedJobFood.push_back(&food);
    }

    if (_foodItemLevelFilter[0] == -1 || _foodItemLevelFilter[1] == -1) {
        _foodItemLevelFilter[0] = maxItemLvl;
        _foodItemLevelFilter[1] = maxItemLvl;
    }

    selectFoodItemLvl(updateRanges);
}

void SolveTab::selectGearItemLvl(bool updateRanges) {
    _gearPiecesToDisplay.clear();
    _gearPiecesToDisplayCount = 0;
    _gearPiecesToDisplaySlotCount = 0;
    _selectedGearPieces.clear();
    for (int slot = 1; slot < 13; slot++) {
        _gearPiecesToDisplay[slot];
    }

    for (const auto gearPiece : _selectedJobGearPieces) {
        if (gearPiece->levelItem < _gearItemLevelFilter[0] || gearPiece->levelItem > _gearItemLevelFilter[1]) continue;

        auto slot = gearPiece->equipSlotCategory == 13 ? 1 : gearPiece->equipSlotCategory;
        if (_gearPiecesToDisplay[slot].size() == 0) _gearPiecesToDisplaySlotCount++;
        _gearPiecesToDisplay[slot].push_back(gearPiece);
        _gearPiecesToDisplayCount++;
        _selectedGearPieces[slot].insert(_gearPiecesToDisplay[slot].size() - 1);
    }

    for (auto& it : _gearPiecesToDisplay) {
        sort(it.second.begin(), it.second.end(), [](const GearPiece* p1, const GearPiece* p2) {
            if (p1->levelEquip != p2->levelEquip) return p2->levelEquip > p1->levelEquip;
            if (p1->levelItem != p2->levelItem) return p2->levelItem > p1->levelItem;
            return p2->id > p1->id;
            });
    }

    updateGearPiecesToDisplayMeldPerms();

    if (updateRanges) {
        updateBaseParamRanges();
    }
}

void SolveTab::updateGearPiecesToDisplayMeldPerms() {
    for (auto& it : _gearPiecesToDisplay) {
        for (auto& gearPiece : it.second) {
            gearPiece->setMeldPerms(_releventMateriaBaseParam, Data::Instance().materiaList ,_allowOvermelds);
        }
    }
}

void SolveTab::selectFoodItemLvl(bool updateRanges) {
    _foodToDisplay.clear();
    _selectedFoodIdx.clear();

    for (const auto food : _selectedJobFood) {
        if (food->levelItem < _foodItemLevelFilter[0] || food->levelItem > _foodItemLevelFilter[1]) continue;

        _foodToDisplay.push_back(food);
        _selectedFoodIdx.insert(_foodToDisplay.size() - 1);
    }

    if (updateRanges) {
        updateBaseParamRanges();
    }
}

void SolveTab::setReleventStats() {
    _releventMateriaBaseParam.clear();

    _releventMateriaBaseParam.push_back(BaseParam::DirectHit);
    _releventMateriaBaseParam.push_back(BaseParam::CriticalHit);
    _releventMateriaBaseParam.push_back(BaseParam::Determination);
    _releventMateriaBaseParam.push_back(_selectedJob->getSpeedBaseParam());
    if (_selectedJob->role == 1) _releventMateriaBaseParam.push_back(BaseParam::Tenacity);
    if (_selectedJob->role == 4) _releventMateriaBaseParam.push_back(BaseParam::Piety);
}

void SolveTab::setColumnHeaders() {
    _gearColumnHeaders.clear();
    _foodColumnHeaders.clear();
    _gearBaseParamToDisplay.clear();
    _foodBaseParamToDisplay.clear();

    _gearBaseParamToDisplay.push_back(_selectedJob->getWeaponDamageBaseParam());
    _gearBaseParamToDisplay.push_back(_selectedJob->primaryStat);
    for (const auto& baseParam : _releventMateriaBaseParam) {
        _gearBaseParamToDisplay.push_back(baseParam);
        _foodBaseParamToDisplay.push_back(baseParam);
    }

    _gearColumnHeaders.push_back("Equip slot");
    _gearColumnHeaders.push_back("Name");
    for (const auto& baseParam : _gearBaseParamToDisplay) {
        _gearColumnHeaders.push_back(BaseParam::abbr.at(baseParam));
    }
    _gearColumnHeaders.push_back("iLvl");
    _gearColumnHeaders.push_back("Meld slots");

    _foodColumnHeaders.push_back("Name");
    for (const auto& baseParam : _foodBaseParamToDisplay) {
        _foodColumnHeaders.push_back(BaseParam::abbr.at(baseParam));
    }
    _foodColumnHeaders.push_back("iLvl");
}

void SolveTab::filterSelection() {
    _resultsColumnHeaders.clear();
    _gearPiecesFiltered.clear();
    _foodFiltered.clear();

    _resultsBaseParamToDisplay = _gearBaseParamToDisplay;

    _resultsColumnHeaders.push_back("GCD");
    _resultsColumnHeaders.push_back("Damage Mod");
    for (const auto& baseParam : _resultsBaseParamToDisplay) {
        _resultsColumnHeaders.push_back(BaseParam::abbr.at(baseParam));
    }
    _resultsColumnHeaders.push_back("Food");

    for (auto& it : _selectedGearPieces) {
        int slot = it.first;
        for (auto idx : it.second) {
            _gearPiecesFiltered[slot].push_back(_gearPiecesToDisplay[slot][idx]);
        }
    }

    for (auto idx : _selectedFoodIdx) {
        _foodFiltered.push_back(_foodToDisplay[idx]);
    }
}

string SolveTab::resultsToJson() {
    if (SetBuilder::Instance().results.size() == 0) return "";

    const Job* job = SetBuilder::Instance().results.begin()->second.selectedJob;

    stringstream resultJson;
    resultJson << "{"
               << "\"name\":\"" << job->name << "\","
               << "\"sets\":[";

    for (auto it = SetBuilder::Instance().results.begin(); it != SetBuilder::Instance().results.end(); it++) {
        if (it != SetBuilder::Instance().results.begin()) resultJson << ",";
        resultJson << it->second.toJson();
    }

    resultJson << "],"
               << "\"level\":" << 100 << ","
               << "\"job\":\"" << job->name << "\","
               << "\"partyBonus\":" << 5 << ","
               << "\"itemDisplaySettings\":{"
               <<     "\"minILvl\":" << _resultGearItemLevelFilter[0] << ","
               <<     "\"maxILvl\":" << _resultGearItemLevelFilter[1] << ","
               <<     "\"minILvlFood\":" << _resultFoodItemLevelFilter[0] << ","
               <<     "\"maxILvlFood\":" << _resultFoodItemLevelFilter[1] << "}"
               << "}";

    return resultJson.str();
}