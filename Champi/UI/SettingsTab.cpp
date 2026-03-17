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
        ImGui::SliderInt("##MaxParallelWorkers", &Settings::Instance().maxParallelWorkers, 2, thread::hardware_concurrency(), "%d", ImGuiSliderFlags_AlwaysClamp);
		if (ImGui::IsItemEdited()) Settings::Instance().writeSettingsFile();
        ImGui::TableSetColumnIndex(col++);
        ImGui::TextDisabled("(?)");
        if (ImGui::BeginItemTooltip()) {
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
        if (ImGui::BeginItemTooltip()) {
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
}

void SettingsTab::drawLightFileButton() {
    if (FileReader::Instance().isReading) {
        ImGui::ProgressBar(FileReader::Instance().progress, ImVec2(200.0f, 0.0f));
        ImGui::SameLine();
        ImGui::Text("Reading");
        ImGui::SameLine();
        ImGui::Text(FileReader::Instance().currentFile.c_str());
    } else if (FileReader::Instance().isWriting) {
        ImGui::Text("Writing");
        ImGui::SameLine();
        ImGui::Text(FileReader::Instance().currentFile.c_str());
    } else {
        if (ImGui::Button("Generate Light Files")) {
            _readAllFilesThread = jthread(&FileReader::readAllFiles, &FileReader::Instance());
        }
    }
}