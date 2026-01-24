#include "MainWindow.h"

MainWindow MainWindow::m_instance = MainWindow();

MainWindow::MainWindow()
{
    //ctor
}

MainWindow::~MainWindow()
{
    //dtor
}

void MainWindow::draw()
{
    static bool use_work_area = true;
    static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

    // We demonstrate using the full viewport area or the work area (without menu-bars, task-bars etc.)
    // Based on your use case you may want one or the other.
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
    ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

    ImGui::Begin("Main window", nullptr, flags);

    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
    {
        if (ImGui::BeginTabItem("Solve"))
        {
			SolveTab::Instance().draw();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Sets"))
        {
            ImGui::Text("This is the Sets tab!\nCurrently Empty");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Options"))
        {
            ImGui::Text("This is the Options tab!\nCurrently Empty");
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}