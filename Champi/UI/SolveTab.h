#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>
#include <functional>

#include "../ImGui/imgui.h"

#include "CustomElements.h"

#include "../Entities/Data.h"
#include "../Entities/GearPiece.h"
#include "../Entities/Job.h"
#include "../Solver/SetBuilder.h"

class SolveTab
{
public:
    SolveTab();
    virtual ~SolveTab();
    static SolveTab& Instance() { return _instance; }

    void draw();
private:
    static SolveTab _instance;
	
    vector<string> _gearColumnHeaders;
    vector<string> _foodColumnHeaders;
    vector<string> _resultsColumnHeaders;
    vector<int> _gearBaseParamToDisplay;
    vector<int> _foodBaseParamToDisplay;
    vector<int> _releventMateriaBaseParam;
    vector<int> _resultsBaseParamToDisplay;

    string _selectedJobName = "";
    Job* _selectedJob = nullptr;
    int _gearItemLevelFilter[2] = { -1, -1 };
    int _foodItemLevelFilter[2] = { -1, -1 };
    bool _selectResultsTab = false;

    map<int, bool> _limitBaseParam;
    map<int, pair<float, float>> _baseParamModRangeSelected;
    map<int, pair<float, float>> _baseParamModRangeAllowed;
    map<int, pair<int, int>> _baseParamRangeSelected;
    map<int, pair<int, int>> _baseParamRangeAllowed;

    vector<GearPiece*> _selectedJobGearPieces;
    map<int, vector<GearPiece*>> _gearPiecesToDisplay;
    int _gearPiecesToDisplayCount;
    int _gearPiecesToDisplaySlotCount;
    map<int, set<size_t>> _selectedGearPieces;
    map<int, vector<GearPiece*>> _gearPiecesFiltered;
    vector<Food*> _selectedJobFood;
    vector<Food*> _foodToDisplay;
    set<size_t> _selectedFoodIdx;
    vector<Food*> _foodFiltered;
	GearSet _selectedResult;
    int _selectedResultSlots;

    int _resultGearItemLevelFilter[2] = { -1, -1 };
    int _resultFoodItemLevelFilter[2] = { -1, -1 };
    double _clipboardExportUntil = 0.0;

    void drawGearTab();
    void drawFoodTab();
    void drawResultsTab();
    void drawTime(string preText, int64_t timeValue);
    void selectResult(const GearSet& result);
    void drawSelectedResultModal();
    void drawSolveButton();
    void drawLimitBaseParamGroups();
    void drawClipboardButton(const char* label, function<string()> textProvider, const ImVec2& size = ImVec2(0, 0));

    void selectJob();
    void initBaseParamLimits();
    void updateBaseParamRanges();
    void updateBaseParamRange(int baseParam);
    void setSelectedRangeToAllowed(int baseParam);
    float getModToDisplay(const int& baseParam, const int& value);
    void initGearList(bool updateRanges = true);
    void initFoodList(bool updateRanges = true);
    void selectGearItemLvl(bool updateRanges = true);
    void selectFoodItemLvl(bool updateRanges = true);
    void setReleventStats();
    void setColumnHeaders();

    void filterSelection();

    string resultsToJson();
};