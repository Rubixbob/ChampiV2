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
    const vector<string> _columnHeaders = {
        "#",
        "Name",
		//"DamagePhys",
		//"DamageMag",
		//"Delayms",
		//"BaseParamValue[0]",
		//"BaseParamValue[1]",
		//"BaseParamValue[2]",
		//"BaseParamValue[3]",
		//"BaseParamValue[4]",
		//"BaseParamValue[5]",
		//"BaseParamValueSpecial[0]",
		//"BaseParamValueSpecial[1]",
		//"BaseParamValueSpecial[2]",
		//"BaseParamValueSpecial[3]",
		//"BaseParamValueSpecial[4]",
		//"BaseParamValueSpecial[5]",
        "LevelEquip",
        "ClassJobCategory",
		//"BaseParam[0]",
		//"BaseParam[1]",
		//"BaseParam[2]",
		//"BaseParam[3]",
		//"BaseParam[4]",
		//"BaseParam[5]",
		//"ItemSpecialBonus",
		//"BaseParamSpecial[0]",
		//"BaseParamSpecial[1]",
		//"BaseParamSpecial[2]",
		//"BaseParamSpecial[3]",
		//"BaseParamSpecial[4]",
		//"BaseParamSpecial[5]",
		//"MateriaSlotCount",
		//"IsAdvancedMeldingPermitted",
		//"Icon",
        "LevelItem",
        "EquipSlotCategory",
        //"BaseParamModifier",
		//"IsUnique"
    };

	bool _isApplyingFilters = false;

    vector<vector<string>> _itemsToDisplay;

	jthread _applyFiltersThread;

    int _levelEquip[2] = { 1, 100 };
	int _classJobCategory = -1;
	int _classJobUse = -1;
	int _levelItem[2] = { 1, 790 };
	int _equipSlotCategory = -1;
	int _baseParamModifier = -1;

	void drawLightFileButton();
	void drawItemTable();
    void applyFilters();
    void applyFiltersInner(stop_token stopToken);
};