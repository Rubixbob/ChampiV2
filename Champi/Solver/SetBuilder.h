#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <set>
#include <algorithm>
#include <thread>
#include <chrono>
#include <atomic>
#include <utility>

#include "Damage.h"
#include "MeldSolver.h"

#include "../Entities/Data.h"
#include "../Entities/GearPiece.h"
#include "../Entities/Job.h"
#include "../Entities/Materia.h"

using namespace std;
using namespace std::chrono;

class SetBuilder
{
public:
	SetBuilder();
	virtual ~SetBuilder();
	static SetBuilder& Instance() { return _instance; }

	bool isSolving = false;
	float solvingProgress = 0.0f;
	int64_t estimatedRemaining = 0;
	int64_t solvingTime = 0;

	map<int, GearSet> results;

	void startSolve(Job* job, int level, const map<int, vector<GearPiece*>>& gearPieces, const vector<Food*>& foodList, const vector<int>& releventMateriaBaseParam);
	void cancelSolve();
private:
	static SetBuilder _instance;

	jthread _solveThread;

	vector<int> _equipSlots;
	map<int, size_t> _gearPieceCounter;
	vector<size_t> _gearPieceIdx;
	map<int, size_t> _gearPieceSlotToIdx;
	size_t _ringPerms;
	size_t _switchCounter;
	size_t _maxCounter;

	void solve(stop_token stopToken, Job* job, int level, const map<int, vector<GearPiece*>>& gearPieces, const vector<Food*>& foodList, const vector<int>& releventMateriaBaseParam);
	vector<GearPiece*> initGear(const map<int, vector<GearPiece*>>& gearPieces);
	void switchGear(const map<int, vector<GearPiece*>>& gearPieces, vector<GearPiece*>& gearPiecesToSolve);
	void switchPiece(const map<int, vector<GearPiece*>>& gearPieces, vector<GearPiece*>& gearPiecesToSolve, int slot);
};