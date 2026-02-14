#include "SolveTab.h"

SolveTab SolveTab::_instance = SolveTab();

SolveTab::SolveTab() {
    //ctor
}

SolveTab::~SolveTab() {
    //dtor
}

void SolveTab::draw() {
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

    ImGui::SameLine();
    drawSolveButton();

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

void SolveTab::drawGearTab() {
    if (_gearItemLevelFilter[0] != -1 && _gearItemLevelFilter[1] != -1) {
        ImGui::Text("iLevel");
        ImGui::SameLine();
        ImGui::PushItemWidth(100);
        ImGui::InputInt2("##GearItemLevel", _gearItemLevelFilter);
        if (ImGui::IsItemEdited()) {
            selectGearItemLvl();
        }
    }

    ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp;
    ImVec2 outer_size = ImVec2(0.0f, min(ImGui::GetFrameHeight() + _gearPiecesToDisplayCount * (ImGui::GetTextLineHeight() + ImGui::GetStyle().CellPadding.y * 2.0f) + (_gearPiecesToDisplaySlotCount - 1) * 8 - 2, ImGui::GetContentRegionAvail().y));
    if (_gearPiecesToDisplayCount > 0 && ImGui::BeginTable("ItemsTable", _gearColumnHeaders.size(), flags, outer_size)) {
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
                    }
                    else {
                        _selectedGearPieces[slot].insert(idx);
                    }
                }
                if (!isSelected) ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.4f)));
                
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
                ImGui::TextUnformatted(to_string(item->isAdvancedMeldingPermitted ? 5 : item->materiaSlotCount).c_str());
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
    if (_foodToDisplay.size() > 0 && ImGui::BeginTable("FoodTable", _foodColumnHeaders.size(), flags, outer_size)) {
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
                }
                else {
                    _selectedFoodIdx.insert(idx);
                }
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
        auto estimatedRemaining = SetBuilder::Instance().estimatedRemaining;
        stringstream remainingText;
        remainingText << "Remaining: "
            << (int)(estimatedRemaining / 1000 / 60) << "m"
            << setw(2) << (int)(estimatedRemaining / 1000) % 60 << "s"
            << setw(3) << estimatedRemaining % 1000;
        ImGui::TextUnformatted(remainingText.str().c_str());
    } else if (SetBuilder::Instance().solvingTime > 0) {
        auto solvingTime = SetBuilder::Instance().solvingTime;
        stringstream solvingText;
        solvingText << "Solving done in: "
            << (int)(solvingTime / 1000 / 60) << "m"
            << setw(2) << (int)(solvingTime / 1000) % 60 << "s"
            << setw(3) << solvingTime % 1000;
        ImGui::TextUnformatted(solvingText.str().c_str());
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
    if (SetBuilder::Instance().results.size() > 0 && ImGui::BeginTable("ResultsTable", _resultsColumnHeaders.size(), flags, outer_size)) {
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

void SolveTab::drawSolveButton() {
    vector<string> solveButtonErrorMessages;
    if (_selectedJob == nullptr) solveButtonErrorMessages.push_back("No job selected");
    for (auto it : _selectedGearPieces) {
        if (it.first == 6) continue;
        if (it.first == 2 && _selectedJob != nullptr && _selectedJob->name == "PLD" && it.second.size() > 0) continue;
        if (it.first == 12 && (it.second.size() > 1 || it.second.size() == 1 && !_gearPiecesToDisplay[it.first][*it.second.begin()]->isUnique)) continue;
        if (it.first != 12 && it.second.size() > 0) continue;
        solveButtonErrorMessages.push_back("Not enough gear pieces selected");
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

    if (SetBuilder::Instance().isSolving) {
        ImGui::SameLine();
        if (ImGui::HandButton("Cancel")) {
            SetBuilder::Instance().cancelSolve();
        }
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

    initGearList();
    initFoodList();
}

void SolveTab::initGearList() {
    _selectedJobGearPieces.clear();

    int maxItemLvl = 0;
    for (auto& gearPiece : Data::Instance().gearPieceList) {
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

    selectGearItemLvl();
}

void SolveTab::initFoodList() {
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

    selectFoodItemLvl();
}

void SolveTab::selectGearItemLvl() {
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
}

void SolveTab::selectFoodItemLvl() {
    _foodToDisplay.clear();
    _selectedFoodIdx.clear();

    for (const auto food : _selectedJobFood) {
        if (food->levelItem < _foodItemLevelFilter[0] || food->levelItem > _foodItemLevelFilter[1]) continue;

        _foodToDisplay.push_back(food);
        _selectedFoodIdx.insert(_foodToDisplay.size() - 1);
    }
}

void SolveTab::setReleventStats() {
    _releventMateriaBaseParam.clear();

    _releventMateriaBaseParam.push_back(22); // DH
    _releventMateriaBaseParam.push_back(27); // CRIT
    _releventMateriaBaseParam.push_back(44); // DET
    _releventMateriaBaseParam.push_back(_selectedJob->primaryStat == 4 || _selectedJob->primaryStat == 5 ? 46 : 45); // SS
    if (_selectedJob->role == 1) _releventMateriaBaseParam.push_back(19); // TNC
    if (_selectedJob->role == 4) _releventMateriaBaseParam.push_back(6); // PIE
}

void SolveTab::setColumnHeaders() {
    _gearColumnHeaders.clear();
    _foodColumnHeaders.clear();
    _gearBaseParamToDisplay.clear();
    _foodBaseParamToDisplay.clear();

    _gearBaseParamToDisplay.push_back(_selectedJob->primaryStat == 4 || _selectedJob->primaryStat == 5 ? 13 : 12); // WD
    _gearBaseParamToDisplay.push_back(_selectedJob->primaryStat); // Main Stat
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