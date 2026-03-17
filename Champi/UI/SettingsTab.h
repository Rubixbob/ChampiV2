#pragma once

#include <iostream>
#include <thread>

#include "../ImGui/imgui.h"
#include "../Config/FileReader.h"
#include "../Solver/Settings.h"

class SettingsTab
{
public:
    SettingsTab();
    virtual ~SettingsTab();
    static SettingsTab& Instance() { return _instance; }

    void draw();
private:
    static SettingsTab _instance;
	jthread _readAllFilesThread;

	void drawLightFileButton();
};