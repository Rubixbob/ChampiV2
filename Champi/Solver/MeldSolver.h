#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <atomic>
#include <thread>

#include "GearSet.h"

#include "../Entities/Food.h"
#include "../Entities/GearPiece.h"
#include "../Entities/Job.h"
#include "../Entities/Materia.h"

using namespace std;

class MeldSolver
{
public:
	MeldSolver();
	MeldSolver(Job* job, vector<GearPiece*> gearPieces, vector<Food*> foodList, vector<int> releventMateriaBaseParam, atomic_int* activeThreads, stop_token stopToken);
	virtual ~MeldSolver();

	vector<GearPiece*> gearPieces;
	map<int, GearSet> results;
	float currentCount = 0.0f;
	float maxCounter = 1.0f;
	float logMaxCounter = 1.0f;
	bool done = false;

	void findBestMelds();

	void setBaseParamRanges(const map<int, pair<int, int>>& baseParamRangeSelected);
private:
	const Job* _job = nullptr;
	vector<Food*> _foodList;
	vector<int> _releventMateriaBaseParam;
	stop_token _solveStopToken;

	atomic_int* _solveActiveThreads = nullptr;

	map<int, pair<int, int>> _baseParamRangeSelected;
	map<int, deque<pair<int, int>>> _slotBaseParamRange;
};