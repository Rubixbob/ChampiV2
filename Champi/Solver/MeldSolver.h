#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
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
	// TODO: make access to the global structure of results to compare to

	atomic_int* _solveActiveThreads = nullptr;

	size_t _switchCounter = 0;
	size_t _maxCounter = 1;

	uint64_t getSaveKey(const map<int, vector<int>>& slots);
	uint64_t getSaveKey(const map<int, map<int, int>>& slots, const vector<int>& materiaGrades, const map<int, int>& materiaBaseParamToIdx);
	void checkAndSaveSet(GearSet& gearSet);
	void switchToNextCombination(const vector<int>& materiaGrades, const map<int, int>& maxMeldSlots, map<int, vector<int>>& meldComb, size_t lastBaseParam, size_t secondToLastBaseParam);
};