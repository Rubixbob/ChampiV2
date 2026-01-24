#include "SolveTab.h"

SolveTab SolveTab::m_instance = SolveTab();

SolveTab::SolveTab()
{
    //ctor
}

SolveTab::~SolveTab()
{
    //dtor
}

void SolveTab::draw()
{
    ImGui::Text("This is the Solve tab!\nCurrently Empty");
	if (ImGui::Button("Solve"))
    {
        // Placeholder for solve action
        std::cout << "Solve button clicked!" << std::endl;
        BiSFinder::Instance().init();
        //BiSFinder::Instance().findBiS();
        new jthread(&BiSFinder::findBiS, BiSFinder::Instance());
    }
}