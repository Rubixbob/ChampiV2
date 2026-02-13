#include "SettingsTab.h"

SettingsTab SettingsTab::_instance = SettingsTab();

SettingsTab::SettingsTab()
{
    //ctor
}

SettingsTab::~SettingsTab()
{
    //dtor
}

void SettingsTab::draw()
{
    //drawLightFileButton();


    if (ImGui::BeginTable("SettingsTable", 3, ImGuiTableFlags_SizingFixedFit)) {
        int col;

        ImGui::TableNextRow();
        col = 0;

        ImGui::TableSetColumnIndex(col++);
        ImGui::Text("Max Parallel Workers");
        ImGui::TableSetColumnIndex(col++);
        ImGui::PushItemWidth(200);
        ImGui::SliderInt("##MaxParallelWorkers", &Settings::Instance().maxParallelWorkers, 1, thread::hardware_concurrency(), "%d", ImGuiSliderFlags_AlwaysClamp);
		if (ImGui::IsItemEdited()) Settings::Instance().writeSettingsFile();
        ImGui::TableSetColumnIndex(col++);
        ImGui::TextDisabled("(?)");
        if (ImGui::BeginItemTooltip())
        {
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted("Maximum number of parallel workers used for solving.\n"
                "A higher value makes solving faster but increases CPU usage.\n"
                "Ctrl + Click to manually input a value.");
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }

        ImGui::TableNextRow();
        col = 0;

        ImGui::TableSetColumnIndex(col++);
        ImGui::Text("Min Materia Ratio");
        ImGui::TableSetColumnIndex(col++);
        ImGui::PushItemWidth(200);
        ImGui::SliderFloat("##MinMateriaRatio", &Settings::Instance().minMateriaRatio, 0, 1, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        if (ImGui::IsItemEdited()) Settings::Instance().writeSettingsFile();
        ImGui::TableSetColumnIndex(col++);
        ImGui::TextDisabled("(?)");
        if (ImGui::BeginItemTooltip())
        {
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted("Minimum ratio of materia value allowed when the full value can't be applied.\n"
                "Setting this to 1 may result in some interesting sets to be ignored,\n"
                "while setting this too low will increase solving time.\n"
                "Ctrl + Click to manually input a value.");
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }

        ImGui::EndTable();
    }

    //drawItemTable();
}

void SettingsTab::drawLightFileButton() {
    if (FileReader::Instance().isReading) {
        ImGui::ProgressBar(FileReader::Instance().progress, ImVec2(200.0f, 0.0f));
        ImGui::SameLine();
        ImGui::Text("Reading");
        ImGui::SameLine();
        ImGui::Text(FileReader::Instance().currentFile.c_str());
    }
    else if (FileReader::Instance().isWriting) {
        ImGui::Text("Writing");
        ImGui::SameLine();
        ImGui::Text(FileReader::Instance().currentFile.c_str());
    }
    else {
        if (ImGui::Button("Generate Light Files")) {
            _readAllFilesThread = jthread(&FileReader::readAllFiles, &FileReader::Instance());
        }
    }
}

void SettingsTab::drawItemTable() {
    // Filters
	ImGui::Separator();

	ImGui::Text("LevelEquip");
	ImGui::SameLine();
    ImGui::PushItemWidth(100);
	ImGui::InputInt2("##LevelEquip", _levelEquip);
    if (ImGui::IsItemEdited()) {
        applyFilters();
    }

    ImGui::SameLine();
    ImGui::Text("ClassJobCategory");
    ImGui::SameLine();
    ImGui::PushItemWidth(100);
    if (ImGui::BeginCombo("##ClassJobCategory", to_string(_classJobCategory).c_str())) {
		for (auto entry : FileReader::Instance().classJobCategoryList) {
            const bool isSelected = (_classJobCategory == entry);
            if (ImGui::Selectable(to_string(entry).c_str(), isSelected)) {
                _classJobCategory = entry;
                applyFilters();
            }
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();
	ImGui::Text("ClassJobUse");
    ImGui::SameLine();
    ImGui::PushItemWidth(100);
    if (ImGui::BeginCombo("##ClassJobUse", to_string(_classJobUse).c_str())) {
        for (auto entry : FileReader::Instance().classJobUseList) {
            const bool isSelected = (_classJobUse == entry);
            if (ImGui::Selectable(to_string(entry).c_str(), isSelected)) {
                _classJobUse = entry;
                applyFilters();
            }
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();
	ImGui::Text("LevelItem");
    ImGui::SameLine();
    ImGui::PushItemWidth(100);
	ImGui::InputInt2("##LevelItem", _levelItem);
    if (ImGui::IsItemEdited()) {
        applyFilters();
    }

    ImGui::SameLine();
	ImGui::Text("EquipSlotCategory");
    ImGui::SameLine();
    ImGui::PushItemWidth(100);
    if (ImGui::BeginCombo("##EquipSlotCategory", to_string(_equipSlotCategory).c_str())) {
        for (auto entry : FileReader::Instance().equipSlotCategoryList) {
            const bool isSelected = (_equipSlotCategory == entry);
            if (ImGui::Selectable(to_string(entry).c_str(), isSelected)) {
                _equipSlotCategory = entry;
                applyFilters();
            }
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();
	ImGui::Text("BaseParamModifier");
    ImGui::SameLine();
    ImGui::PushItemWidth(100);
    if (ImGui::BeginCombo("##BaseParamModifier", to_string(_baseParamModifier).c_str())) {
        for (auto entry : FileReader::Instance().baseParamModifierList) {
            const bool isSelected = (_baseParamModifier == entry);
            if (ImGui::Selectable(to_string(entry).c_str(), isSelected)) {
                _baseParamModifier = entry;
                applyFilters();
            }
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();
	if (ImGui::Button("Apply Filters")) {
        applyFilters();
    }
	ImGui::Separator();

    if (!_isApplyingFilters && _itemsToDisplay.size() > 0 && ImGui::BeginTable("ItemTable", _columnHeaders.size(), ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
        for (const auto& header : _columnHeaders) {
            ImGui::TableSetupColumn(header.c_str());
		}
        ImGui::TableHeadersRow();

        for (const auto& item : _itemsToDisplay) {
            ImGui::TableNextRow();
            for (int column = 0; column < _columnHeaders.size(); column++) {
                ImGui::TableSetColumnIndex(column);
                ImGui::TextUnformatted(item[FileReader::Instance().itemCol[_columnHeaders[column]]].c_str());
            }
        }
        ImGui::EndTable();
    }
}

void SettingsTab::applyFilters() {
    auto source = _applyFiltersThread.get_stop_source();
    if (source.stop_possible() && !source.stop_requested()) {
        _applyFiltersThread.request_stop();
        _applyFiltersThread.join();
    }

    _isApplyingFilters = true;

    _itemsToDisplay.clear();
    _applyFiltersThread = jthread([this](stop_token stopToken) { applyFiltersInner(stopToken); });
}

void SettingsTab::applyFiltersInner(stop_token stopToken) {
    for (const auto& item : FileReader::Instance().gearList) {
        if (stopToken.stop_requested()) {
            break;
		}
        int itemLevelEquip = stoi(item[FileReader::Instance().itemCol["LevelEquip"]]);
        int itemClassJobCategory = stoi(item[FileReader::Instance().itemCol["ClassJobCategory"]]);
        int itemClassJobUse = stoi(item[FileReader::Instance().itemCol["ClassJobUse"]]);
        int itemLevelItem = stoi(item[FileReader::Instance().itemCol["LevelItem"]]);
        int itemEquipSlotCategory = stoi(item[FileReader::Instance().itemCol["EquipSlotCategory"]]);
        int itemBaseParamModifier = stoi(item[FileReader::Instance().itemCol["BaseParamModifier"]]);
        if (itemLevelEquip >= _levelEquip[0] && itemLevelEquip <= _levelEquip[1]
            && (_classJobCategory == -1 || itemClassJobCategory == _classJobCategory)
            && (_classJobUse == -1 || itemClassJobUse == _classJobUse)
            && itemLevelItem >= _levelItem[0] && itemLevelItem <= _levelItem[1]
            && (_equipSlotCategory == -1 || itemEquipSlotCategory == _equipSlotCategory)
            && (_baseParamModifier == -1 || itemBaseParamModifier == _baseParamModifier)) {
            _itemsToDisplay.push_back(item);
        }
    }

    _isApplyingFilters = false;
}