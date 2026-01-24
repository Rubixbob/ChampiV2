#pragma once

#include <iostream>
#include "../ImGui/imgui.h"
#include "SolveTab.h"

class MainWindow
{
public:
    MainWindow();
    virtual ~MainWindow();
    static MainWindow& Instance() { return m_instance; }

	void draw();
private:
    static MainWindow m_instance;
};