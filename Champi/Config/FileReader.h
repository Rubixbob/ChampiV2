#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <filesystem>
#include <algorithm>
#include <cstdlib>

#include "../Entities/Data.h"
#include "../Entities/GearPiece.h"
#include "../Entities/Item.h"
#include "../Entities/Materia.h"
#include "../Entities/Food.h"

using namespace std;

class FileReader
{
public:
    FileReader();
    virtual ~FileReader();
    static FileReader& Instance() { return _instance; }

    bool isReading = false;
    bool isWriting = false;
    string currentFile = "";
	float progress = 0.0f;
    uintmax_t filesSizeOffset = 0;
	uintmax_t filesSizeTotal = 0;

    map<string, int> itemCol;

    set<int> classJobCategoryList;

    void readAllFiles();

    void readBaseParamLight();
    void readFoodLight();
    void readGearPieceLight();
    void readJobLight();
    void readMateriaLight();
    void readRequiredItemLight();
    void readSpecialShopLight();
private:
    static FileReader _instance;

    enum FileType {
        BaseParam,
        ClassJob,
        ClassJobCategory,
        EquipSlotCategory,
        Item,
        ItemAction,
		ItemFood,
        ItemLevel,
        Materia,
        MateriaGrade,
        SpecialShop,
        TomestonesItem
    };

    vector<string> _fileNames = {
        "Config/BaseParam.csv",
        "Config/ClassJob.csv",
        "Config/ClassJobCategory.csv",
        "Config/EquipSlotCategory.csv",
        "Config/Item.csv",
        "Config/ItemAction.csv",
        "Config/ItemFood.csv",
        "Config/ItemLevel.csv",
        "Config/Materia.csv",
        "Config/MateriaGrade.csv",
        "Config/SpecialShop.csv",
        "Config/TomestonesItem.csv"
    };

	string _baseParamLightFileName = "Config/BaseParamLight.txt";
	string _foodLightFileName = "Config/FoodLight.txt";
	string _gearPieceLightFileName = "Config/GearPieceLight.txt";
	string _jobLightFileName = "Config/JobLight.txt";
	string _materiaLightFileName = "Config/MateriaLight.txt";
	string _requiredItemLightFileName = "Config/RequiredItemLight.txt";
	string _specialShopLightFileName = "Config/SpecialShopLight.txt";

    set<int> _combatReleventBaseParam = {
        1,
		2,
		3,
		4,
        5,
        6,
        12,
        13,
        19,
        22,
        27,
        44,
        45,
        46
    };

    set<int> _materiaReleventBaseParam = {
        6,
        19,
        22,
        27,
        44,
        45,
        46
    };

    map<int, size_t> _itemToMateriaIdx;
    map<int, size_t> _itemFoodIdToIdx;
    map<int, size_t> _itemActionIdToItemFoodIdx;
    map<int, vector<int>> _materiaOvermeldPercent;
    map<string, size_t> _classJobNameToIdx;
    map<int, int> _tomestoneToItem;
    map<int, map<int, map<int, int>>> _itemCost; // For each item, for each shop, count per item
    set<int> _requiredItems; // List of items that are required in itemCost

    bool readFile(FileType fileType);
	void handleLine(FileType fileType, const vector<string>& attributes, map<string, int>& col);
	void handleItemLine(const vector<string>& attributes, map<string, int>& col);
	void handleItemActionLine(const vector<string>& attributes, map<string, int>& col);
	void handleItemFoodLine(const vector<string>& attributes, map<string, int>& col);
	void handleMateriaLine(const vector<string>& attributes, map<string, int>& col);
	void handleMateriaGradeLine(const vector<string>& attributes, map<string, int>& col);
	void handleClassJobLine(const vector<string>& attributes, map<string, int>& col);
	void handleClassJobCategoryLine(const vector<string>& attributes, map<string, int>& col);
	void handleBaseParamLine(const vector<string>& attributes, map<string, int>& col);
	void handleSpecialShopLine(const vector<string>& attributes, map<string, int>& col);
	void handleTomestonesItemLine(const vector<string>& attributes, map<string, int>& col);
	vector<string> readLine(ifstream& itemFile);
    map<string, int> GenerateColumnMap(vector<string> headers);
    void writeBaseParamLight();
    void writeFoodLight();
    void writeGearPieceLight();
    void writeJobLight();
    void writeMateriaLight();
    void writeRequiredItemLight();
    void writeSpecialShopLight();
};

