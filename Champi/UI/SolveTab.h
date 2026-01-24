#pragma once

#include <iostream>
#include <thread>

#include "../ImGui/imgui.h"
#include "../SolverOld/BiSFinder.h"

class SolveTab
{
public:
    SolveTab();
    virtual ~SolveTab();
    static SolveTab& Instance() { return m_instance; }

    void draw();
private:
    static SolveTab m_instance;
};