#pragma once

#include <iostream>
#include "../ImGui/imgui.h"
#include "SolveTab.h"
#include "SettingsTab.h"

class MainWindow
{
public:
    MainWindow();
    virtual ~MainWindow();
    static MainWindow& Instance() { return _instance; }

	void draw();
private:
    static MainWindow _instance;
};