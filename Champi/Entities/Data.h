#pragma once

#include <vector>
#include <map>

#include "BaseParam.h"
#include "Food.h"
#include "GearPiece.h"
#include "Job.h"
#include "Materia.h"

#include "../Config/FileReader.h"

using namespace std;

class Data
{
public:
	Data();
	virtual ~Data();
	static Data& Instance() { return _instance; }

	bool isLoaded = false;

	map<int, BaseParam> baseParamList;
	vector<Food> foodList;
	vector<GearPiece> gearPieceList;
	vector<Job> jobList;
	vector<Materia> materiaList;

private:
	static Data _instance;
};