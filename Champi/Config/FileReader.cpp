#include "FileReader.h"

FileReader FileReader::_instance = FileReader();

FileReader::FileReader()
{
    //ctor
}

FileReader::~FileReader()
{
    //dtor
}

void FileReader::readAllFiles() {
    if (isReading) return;

    isReading = true;
    progress = 0.0f;

	// Reset data
    gearList.clear(); // TODO: delete when gearPieceList works
    Data::Instance().baseParamList.clear();
    Data::Instance().foodList.clear();
    Data::Instance().gearPieceList.clear();
    Data::Instance().jobList.clear();
    Data::Instance().materiaList.clear();

	_itemToMateriaIdx.clear();
	_itemFoodIdToIdx.clear();
	_itemActionIdToItemFoodIdx.clear();
    _materiaOvermeldPercent.clear();
    _classJobNameToIdx.clear();

    vector<FileType> filesToRead = {
        FileType::MateriaGrade,
        FileType::Materia,
        FileType::ItemFood,
        FileType::ItemAction,
        FileType::Item,
        FileType::ClassJob,
        FileType::ClassJobCategory,
        FileType::BaseParam
    };

    filesSizeOffset = 0;
    filesSizeTotal = 0;
    for (auto fileType : filesToRead) {
		filesSizeTotal += filesystem::file_size(_fileNames[fileType]);
    }

    currentFile = "";
    for (auto fileType : filesToRead) {
		currentFile = _fileNames[fileType];
        if (!readFile(fileType)) break;
        filesSizeOffset += filesystem::file_size(currentFile);
    }
    isReading = false;

    isWriting = true;
    currentFile = _baseParamLightFileName;
    writeBaseParamLight();
    currentFile = _foodLightFileName;
    writeFoodLight();
    currentFile = _gearPieceLightFileName;
	writeGearPieceLight();
    currentFile = _jobLightFileName;
    writeJobLight();
    currentFile = _materiaLightFileName;
    writeMateriaLight();
    currentFile = "";
    isWriting = false;

	// Cleanup temporary data
    _itemToMateriaIdx.clear();
    _itemFoodIdToIdx.clear();
    _itemActionIdToItemFoodIdx.clear();
    _materiaOvermeldPercent.clear();
    _classJobNameToIdx.clear();
}

bool FileReader::readFile(FileType fileType) {
    string fileName = _fileNames[fileType];

    ifstream itemFile(fileName, ios::in);
    if (!itemFile) {
        cout << "Couldn't open " << fileName << endl;
        isReading = false;
        return false;
    }

    try {
        auto headers = readLine(itemFile);
        while (headers[0] != "#") {
            headers = readLine(itemFile);
        }
        auto col = GenerateColumnMap(headers);
        if (fileType == FileType::Item) {
            itemCol = col;

            classJobCategoryList.clear();
            classJobUseList.clear();
            equipSlotCategoryList.clear();
            baseParamModifierList.clear();
            classJobCategoryList.insert(-1);
            classJobUseList.insert(-1);
            equipSlotCategoryList.insert(-1);
            baseParamModifierList.insert(-1);
        }

        bool firstDataLineRead = false;

        while (!itemFile.eof()) {
            auto attributes = readLine(itemFile);
            if (!firstDataLineRead) {
                while (attributes[0] != "0") {
                    attributes = readLine(itemFile);
                }
                firstDataLineRead = true;
            }
            if (attributes.size() > 0) {
                handleLine(fileType, attributes, col);
            }
            progress = (float)(filesSizeOffset + itemFile.tellg()) / (float)filesSizeTotal;
        }
    }
    catch (...) {
        // To ensure state correctness
    }

    itemFile.close();
    return true;
}

void FileReader::handleLine(FileType fileType, const vector<string>& attributes, map<string, int>& col) {
    switch (fileType) {
        case BaseParam:
            handleBaseParamLine(attributes, col);
            break;
        case ClassJob:
            handleClassJobLine(attributes, col);
            break;
        case ClassJobCategory:
            handleClassJobCategoryLine(attributes, col);
            break;
        case EquipSlotCategory:
            break;
        case Item:
            handleItemLine(attributes, col);
            break;
        case ItemAction:
			handleItemActionLine(attributes, col);
            break;
        case ItemFood:
			handleItemFoodLine(attributes, col);
            break;
        case ItemLevel:
            break;
        case Materia:
            handleMateriaLine(attributes, col);
            break;
        case MateriaGrade:
            handleMateriaGradeLine(attributes, col);
            break;
        default:
            break;
    }
}

void FileReader::handleItemLine(const vector<string>& attributes, map<string, int>& col) {
    auto id = stoi(attributes[col["#"]]);
    auto equipSlotCategory = stoi(attributes[col["EquipSlotCategory"]]);
    auto name = attributes[col["Name"]];
	auto levelItem = stoi(attributes[col["LevelItem"]]);
	auto itemActionId = stoi(attributes[col["ItemAction"]]);
    if (equipSlotCategory > 0 && equipSlotCategory <= 13 && levelItem >= 150) {
        gearList.push_back(attributes);

        Data::Instance().gearPieceList.emplace_back();
		auto& gearPiece = Data::Instance().gearPieceList.back();
        gearPiece.id = id;
		gearPiece.name = name;
        gearPiece.equipSlotCategory = equipSlotCategory;
		gearPiece.damagePhys = stoi(attributes[col["DamagePhys"]]);
		gearPiece.damageMag = stoi(attributes[col["DamageMag"]]);
		gearPiece.delayms = stoi(attributes[col["Delayms"]]);

		map<int, int> baseParamToIdx;
		for (int i = 0; i < 6; i++) {
			gearPiece.baseParamValue[i] = stoi(attributes[col["BaseParamValue[" + to_string(i) + "]"]]);
			auto baseParam = stoi(attributes[col["BaseParam[" + to_string(i) + "]"]]);
            gearPiece.baseParam[i] = baseParam;
			baseParamToIdx[baseParam] = i;
		}
		// Bake in HQ bonus stats
		bool hasHQBonus = stoi(attributes[col["ItemSpecialBonus"]]) == 1;
        if (hasHQBonus) {
            for (int i = 0; i < 6; i++) {
                int baseParamSpecial = stoi(attributes[col["BaseParamSpecial[" + to_string(i) + "]"]]);
                if (baseParamSpecial == 0) continue;
                int baseParamValueSpecial = stoi(attributes[col["BaseParamValueSpecial[" + to_string(i) + "]"]]);
                switch (baseParamSpecial) {
                    case 12: // Physical Damage
                        gearPiece.damagePhys += baseParamValueSpecial;
                        break;
                    case 13: // Magic Damage
                        gearPiece.damageMag += baseParamValueSpecial;
                        break;
                    case 17: // Block Rate
                    case 18: // Block Strength
                    case 21: // Defense
                    case 24: // Magic Defense
                        break;
                    default:
                        auto baseParamIdxIt = baseParamToIdx.find(baseParamSpecial);
                        if (baseParamIdxIt != baseParamToIdx.end()) {
                            gearPiece.baseParamValue[baseParamIdxIt->second] += baseParamValueSpecial;
                        }
                        break;
                }
            }
        }

		gearPiece.levelEquip = stoi(attributes[col["LevelEquip"]]);
		gearPiece.classJobCategory = stoi(attributes[col["ClassJobCategory"]]);
		gearPiece.materiaSlotCount = stoi(attributes[col["MateriaSlotCount"]]);
		gearPiece.isAdvancedMeldingPermitted = attributes[col["IsAdvancedMeldingPermitted"]] == "True";
		gearPiece.icon = stoi(attributes[col["Icon"]]);
		gearPiece.levelItem = levelItem;
		gearPiece.isUnique = attributes[col["IsUnique"]] == "True";
    } else {
        auto materiaIt = _itemToMateriaIdx.find(id);
        if (materiaIt != _itemToMateriaIdx.end()) {
            Data::Instance().materiaList[materiaIt->second].name = name;
            Data::Instance().materiaList[materiaIt->second].levelItem = levelItem;
        }

		auto foodIt = _itemActionIdToItemFoodIdx.find(itemActionId);
        if (foodIt != _itemActionIdToItemFoodIdx.end()) {
            Data::Instance().foodList[foodIt->second].name = name;
            Data::Instance().foodList[foodIt->second].levelItem = levelItem;
        }
    }

    classJobCategoryList.insert(stoi(attributes[col["ClassJobCategory"]]));
    classJobUseList.insert(stoi(attributes[col["ClassJobUse"]]));
    equipSlotCategoryList.insert(equipSlotCategory);
    baseParamModifierList.insert(stoi(attributes[col["BaseParamModifier"]]));
}

void FileReader::handleItemActionLine(const vector<string>& attributes, map<string, int>& col) {
    auto foodId = stoi(attributes[col["Data[1]"]]);
	auto foodIdx = _itemFoodIdToIdx.find(foodId);
    if (foodIdx == _itemFoodIdToIdx.end()) return;

    auto itemActionId = stoi(attributes[col["#"]]);
	_itemActionIdToItemFoodIdx[itemActionId] = foodIdx->second;
}

void FileReader::handleItemFoodLine(const vector<string>& attributes, map<string, int>& col) {
    int baseParam[3];
	for (int i = 0; i < 3; i++) {
        baseParam[i] = stoi(attributes[col["BaseParam[" + to_string(i) + "]"]]);
		if (!_combatReleventBaseParam.contains(baseParam[i])) return;
    }

    Data::Instance().foodList.emplace_back();
	auto& food = Data::Instance().foodList.back();
	food.id = stoi(attributes[col["#"]]);
	for (int i = 0; i < 3; i++) {
		food.valueHQ[i] = stoi(attributes[col["ValueHQ[" + to_string(i) + "]"]]);
		food.maxHQ[i] = stoi(attributes[col["MaxHQ[" + to_string(i) + "]"]]);
		food.baseParam[i] = baseParam[i];
	}

	_itemFoodIdToIdx[food.id] = Data::Instance().foodList.size() - 1;
}

void FileReader::handleMateriaLine(const vector<string>& attributes, map<string, int>& col) {
    auto baseParam = stoi(attributes[col["BaseParam"]]);

    if (!_materiaReleventBaseParam.contains(baseParam)) return;

    int i = 0;
    while (true) {
        string itemCol = "Item[" + to_string(i) + "]";
        if (col.find(itemCol) == col.end()) break;
        auto itemId = stoi(attributes[col[itemCol]]);
        auto value = stoi(attributes[col["Value[" + to_string(i) + "]"]]);

        if (itemId > 0) {
            Data::Instance().materiaList.emplace_back();
            auto& materia = Data::Instance().materiaList.back();
            materia.id = itemId;
            materia.grade = i + 1;
            materia.value = value;
            materia.baseParam = baseParam;
            for (int k = 0; k < 4; k++) {
                materia.overmeldPercent[k] = _materiaOvermeldPercent[i][k];
            }

            _itemToMateriaIdx[itemId] = Data::Instance().materiaList.size() - 1;
        }

        i++;
    }
}

void FileReader::handleMateriaGradeLine(const vector<string>& attributes, map<string, int>& col) {
    auto grade = stoi(attributes[col["#"]]);

    vector<int> overmeldPercent;
    for (int i = 0; i < 4; i++) {
        overmeldPercent.push_back(stoi(attributes[col["OvermeldHQPercent[" + to_string(i) + "]"]]));
    }
	_materiaOvermeldPercent[grade] = overmeldPercent;
}

void FileReader::handleClassJobLine(const vector<string>& attributes, map<string, int>& col) {
    auto jobIndex = stoi(attributes[col["JobIndex"]]);

    if (jobIndex == 0 || attributes[col["IsLimitedJob"]] == "True") return;

    Data::Instance().jobList.emplace_back();
    auto& job = Data::Instance().jobList.back();
    job.id = stoi(attributes[col["#"]]);
    job.name = attributes[col["Abbreviation"]];
    job.primaryStat = stoi(attributes[col["PrimaryStat"]]);
    string primaryStatName = "";
    switch (job.primaryStat) {
        case 1:
            primaryStatName = "Strength";
            break;
        case 2:
            primaryStatName = "Dexterity";
            break;
        case 4:
            primaryStatName = "Intelligence";
            break;
        case 5:
            primaryStatName = "Mind";
            break;
        default:
            break;
    }
    job.primaryStatMod = stoi(attributes[col["Modifier" + primaryStatName]]);
    job.role = stoi(attributes[col["Role"]]);

    _classJobNameToIdx[job.name] = Data::Instance().jobList.size() - 1;
}

void FileReader::handleClassJobCategoryLine(const vector<string>& attributes, map<string, int>& col) {
    auto category = stoi(attributes[col["#"]]);

    if (!classJobCategoryList.contains(category)) return;

    for (auto& it : col) {
        if (_classJobNameToIdx.find(it.first) == _classJobNameToIdx.end()) continue;

        if (attributes[it.second] == "True") Data::Instance().jobList[_classJobNameToIdx[it.first]].categories.insert(category);
    }
}

void FileReader::handleBaseParamLine(const vector<string>& attributes, map<string, int>& col) {
    auto id = stoi(attributes[col["#"]]);

    if (!_combatReleventBaseParam.contains(id)) return;

    auto& baseParam = Data::Instance().baseParamList[id];
    baseParam.id = stoi(attributes[col["#"]]);
    baseParam.name = attributes[col["Name"]];
    baseParam.twoHandWeaponPercent = stoi(attributes[col["TwoHandWeaponPercent"]]);
    baseParam.oneHandWeaponPercent = stoi(attributes[col["OneHandWeaponPercent"]]);
    baseParam.offHandPercent = stoi(attributes[col["OffHandPercent"]]);
    baseParam.bigLeftPercent = stoi(attributes[col["ChestPercent"]]);
    baseParam.smallLeftPercent = stoi(attributes[col["HeadPercent"]]);
    baseParam.rightPercent = stoi(attributes[col["RingPercent"]]);
}

vector<string> FileReader::readLine(ifstream& itemFile) {
    vector<string> tokens;

    string itemLine;
    getline(itemFile, itemLine);

    while (itemLine.size() > 0) {
        int tokenStart = 0;
        int tokenSize = 0;
		int delimiterIdx = 0;
        if (itemLine[0] == '"') {
            int quoteEnd = itemLine.find('"', 1);
            while (quoteEnd == -1) {
				string additionalLine;
                getline(itemFile, additionalLine);
                itemLine = itemLine + "\n" + additionalLine;
                quoteEnd = itemLine.find('"', 1);
            }
            tokenStart = 1;
            tokenSize = quoteEnd - 1;
			delimiterIdx = tokenSize + 2;
        } else {
            tokenSize = itemLine.find(',');
			delimiterIdx = tokenSize;
        }
        if (tokenSize == -1) {
            tokens.push_back(itemLine);
            break;
        }
		string token = itemLine.substr(tokenStart, tokenSize);
        tokens.push_back(token);

        itemLine = itemLine.substr(delimiterIdx + 1);
    }

    return tokens;
}

map<string, int> FileReader::GenerateColumnMap(vector<string> headers) {
    map<string, int> result;
	for (unsigned int i = 0; i < headers.size(); i++) {
        result[headers[i]] = i;
    }
    return result;
}

void FileReader::writeBaseParamLight() {
	ofstream baseParamLightFile(_baseParamLightFileName, ios::out);
    if (!baseParamLightFile) {
        cout << "Couldn't open " << _baseParamLightFileName << " for writing." << endl;
        return;
    }
    for (const auto& it : Data::Instance().baseParamList) {
        baseParamLightFile << it.second.id << "\t";
        baseParamLightFile << it.second.name << "\t";
        baseParamLightFile << it.second.twoHandWeaponPercent << "\t";
        baseParamLightFile << it.second.oneHandWeaponPercent << "\t";
        baseParamLightFile << it.second.offHandPercent << "\t";
        baseParamLightFile << it.second.bigLeftPercent << "\t";
        baseParamLightFile << it.second.smallLeftPercent << "\t";
        baseParamLightFile << it.second.rightPercent << "\n";
    }
}

void FileReader::writeFoodLight() {
	ofstream foodLightFile(_foodLightFileName, ios::out);
    if (!foodLightFile) {
        cout << "Couldn't open " << _foodLightFileName << " for writing." << endl;
        return;
    }
    for (const auto& food : Data::Instance().foodList) {
        foodLightFile << food.id << "\t";
        foodLightFile << food.name << "\t";
        for (int i = 0; i < 3; i++) {
            foodLightFile << food.valueHQ[i] << "\t";
        }
        for (int i = 0; i < 3; i++) {
            foodLightFile << food.maxHQ[i] << "\t";
        }
        for (int i = 0; i < 3; i++) {
            foodLightFile << food.baseParam[i] << "\t";
        }
        foodLightFile << food.levelItem << "\n";
    }
}

void FileReader::writeGearPieceLight() {
    ofstream gearPieceLightFile(_gearPieceLightFileName, ios::out);
    if (!gearPieceLightFile) {
        cout << "Couldn't open " << _gearPieceLightFileName << " for writing." << endl;
        return;
    }
    for (const auto& gearPiece : Data::Instance().gearPieceList) {
        gearPieceLightFile << gearPiece.id << "\t";
        gearPieceLightFile << gearPiece.name << "\t";
        gearPieceLightFile << gearPiece.equipSlotCategory << "\t";
        gearPieceLightFile << gearPiece.damagePhys << "\t";
        gearPieceLightFile << gearPiece.damageMag << "\t";
        gearPieceLightFile << gearPiece.delayms << "\t";
        for (int i = 0; i < 6; i++) {
            gearPieceLightFile << gearPiece.baseParamValue[i] << "\t";
        }
        for (int i = 0; i < 6; i++) {
            gearPieceLightFile << gearPiece.baseParam[i] << "\t";
        }
        gearPieceLightFile << gearPiece.levelEquip << "\t";
        gearPieceLightFile << gearPiece.classJobCategory << "\t";
        gearPieceLightFile << gearPiece.materiaSlotCount << "\t";
        gearPieceLightFile << (gearPiece.isAdvancedMeldingPermitted ? "1" : "0") << "\t";
        gearPieceLightFile << gearPiece.icon << "\t";
        gearPieceLightFile << gearPiece.levelItem << "\t";
        gearPieceLightFile << (gearPiece.isUnique ? "1" : "0") << "\n";
	}
}

void FileReader::writeJobLight() {
    ofstream jobLightFile(_jobLightFileName, ios::out);
    if (!jobLightFile) {
        cout << "Couldn't open " << _jobLightFileName << " for writing." << endl;
        return;
    }
    for (const auto& job : Data::Instance().jobList) {
        jobLightFile << job.id << "\t";
        jobLightFile << job.name << "\t";
        jobLightFile << job.primaryStat << "\t";
        jobLightFile << job.primaryStatMod << "\t";
        jobLightFile << job.role << "\t";
        int i = 0;
        for (auto& it : job.categories) {
            jobLightFile << it;
            if (i < job.categories.size() - 1) {
                jobLightFile << "\t";
            }
            i++;
        }
        jobLightFile << "\n";
    }
}

void FileReader::writeMateriaLight() {
	ofstream materiaLightFile(_materiaLightFileName, ios::out);
    if (!materiaLightFile) {
        cout << "Couldn't open " << _materiaLightFileName << " for writing." << endl;
        return;
    }
    for (const auto& materia : Data::Instance().materiaList) {
        materiaLightFile << materia.id << "\t";
        materiaLightFile << materia.name << "\t";
        materiaLightFile << materia.grade << "\t";
        materiaLightFile << materia.value << "\t";
        materiaLightFile << materia.baseParam << "\t";
        materiaLightFile << materia.levelItem << "\t";
        for (int i = 0; i < 4; i++) {
            materiaLightFile << materia.overmeldPercent[i];
            if (i < 3) {
                materiaLightFile << "\t";
            }
        }
        materiaLightFile << "\n";
	}
}

void FileReader::readBaseParamLight() {
    ifstream baseParamLightFile(_baseParamLightFileName, ios::in);
    if (!baseParamLightFile) {
        cout << "Couldn't open " << _baseParamLightFileName << " for reading." << endl;
        return;
    }
    Data::Instance().baseParamList.clear();

    while (!baseParamLightFile.eof()) {
        string line;
        getline(baseParamLightFile, line);
        if (line.size() == 0) continue;

        stringstream ss;
        ss.str(line);
        int id;
        ss >> id; ss.ignore();
        auto& baseParam = Data::Instance().baseParamList[id];
        baseParam.id = id;
        getline(ss, baseParam.name, '\t');
        ss >> baseParam.twoHandWeaponPercent;
        ss >> baseParam.oneHandWeaponPercent;
        ss >> baseParam.offHandPercent;
        ss >> baseParam.bigLeftPercent;
        ss >> baseParam.smallLeftPercent;
        ss >> baseParam.rightPercent;
    }
}

void FileReader::readFoodLight() {
    ifstream foodLightFile(_foodLightFileName, ios::in);
    if (!foodLightFile) {
        cout << "Couldn't open " << _foodLightFileName << " for reading." << endl;
        return;
    }
    Data::Instance().foodList.clear();

    while (!foodLightFile.eof()) {
        string line;
        getline(foodLightFile, line);
        if (line.size() == 0) continue;

        stringstream ss;
        ss.str(line);
        Data::Instance().foodList.emplace_back();
        auto& food = Data::Instance().foodList.back();
        ss >> food.id; ss.ignore();
        getline(ss, food.name, '\t');
        for (int i = 0; i < 3; i++) {
            ss >> food.valueHQ[i];
        }
        for (int i = 0; i < 3; i++) {
            ss >> food.maxHQ[i];
        }
        for (int i = 0; i < 3; i++) {
            ss >> food.baseParam[i];
        }
        ss >> food.levelItem;
    }
}

void FileReader::readGearPieceLight() {
    ifstream gearPieceLightFile(_gearPieceLightFileName, ios::in);
    if (!gearPieceLightFile) {
        cout << "Couldn't open " << _gearPieceLightFileName << " for reading." << endl;
        return;
    }
    Data::Instance().gearPieceList.clear();

    while (!gearPieceLightFile.eof()) {
        string line;
        getline(gearPieceLightFile, line);
        if (line.size() == 0) continue;

        stringstream ss;
        ss.str(line);
        Data::Instance().gearPieceList.emplace_back();
        auto& gearPiece = Data::Instance().gearPieceList.back();
        ss >> gearPiece.id; ss.ignore();
        getline(ss, gearPiece.name, '\t');
        ss >> gearPiece.equipSlotCategory;
        ss >> gearPiece.damagePhys;
        ss >> gearPiece.damageMag;
        ss >> gearPiece.delayms;
        for (int i = 0; i < 6; i++) {
            ss >> gearPiece.baseParamValue[i];
        }
        for (int i = 0; i < 6; i++) {
            ss >> gearPiece.baseParam[i];
        }
        ss >> gearPiece.levelEquip;
        ss >> gearPiece.classJobCategory;
        ss >> gearPiece.materiaSlotCount;
        ss >> gearPiece.isAdvancedMeldingPermitted;
        ss >> gearPiece.icon;
        ss >> gearPiece.levelItem;
        ss >> gearPiece.isUnique;
    }
}

void FileReader::readJobLight() {
    ifstream jobLightFile(_jobLightFileName, ios::in);
    if (!jobLightFile) {
        cout << "Couldn't open " << _jobLightFileName << " for reading." << endl;
        return;
    }
    Data::Instance().jobList.clear();

    while (!jobLightFile.eof()) {
        string line;
        getline(jobLightFile, line);
        if (line.size() == 0) continue;

        stringstream ss;
        ss.str(line);
        Data::Instance().jobList.emplace_back();
        auto& job = Data::Instance().jobList.back();
        ss >> job.id; ss.ignore();
        getline(ss, job.name, '\t');
        ss >> job.primaryStat;
        ss >> job.primaryStatMod;
        ss >> job.role;
        int category;
        while (ss >> category) {
            job.categories.insert(category);
        }
    }
}

void FileReader::readMateriaLight() {
    ifstream materiaLightFile(_materiaLightFileName, ios::in);
    if (!materiaLightFile) {
        cout << "Couldn't open " << _materiaLightFileName << " for reading." << endl;
        return;
    }
    Data::Instance().materiaList.clear();

    while (!materiaLightFile.eof()) {
        string line;
        getline(materiaLightFile, line);
        if (line.size() == 0) continue;

        stringstream ss;
        ss.str(line);
        Data::Instance().materiaList.emplace_back();
        auto& materia = Data::Instance().materiaList.back();
        ss >> materia.id; ss.ignore();
        getline(ss, materia.name, '\t');
        ss >> materia.grade;
        ss >> materia.value;
        ss >> materia.baseParam;
        ss >> materia.levelItem;
        for (int i = 0; i < 4; i++) {
            ss >> materia.overmeldPercent[i];
        }

        materia.valueByGrade[materia.grade] = materia.value;
    }
}