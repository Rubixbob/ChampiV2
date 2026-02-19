#pragma once

#include <iostream>
#include <string>
#include <vector>
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
	float solvingProgress = 0.0f;
	bool done = false;

	void findBestMelds();
private:
	const Job* _job = nullptr;
	vector<Food*> _foodList;
	vector<int> _releventMateriaBaseParam;
	stop_token _solveStopToken;

	atomic_int* _solveActiveThreads = nullptr;

	static uint64_t getKey3(const vector<int>& jobBaseParams, const map<int, int>& baseParamValue);
	static uint64_t getKey4(const vector<int>& jobBaseParams, const map<int, int>& baseParamValue);
};