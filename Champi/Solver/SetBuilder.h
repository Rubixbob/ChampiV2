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
	long long estimatedRemaining = 0;
	long long solvingTime = 0;

	map<int, GearSet> results;

	void startSolve(Job* job, int level, const map<int, vector<GearPiece*>>& gearPieces, const vector<Food*>& foodList, const vector<int>& releventMateriaBaseParam);
	void cancelSolve();
private:
	static SetBuilder _instance;

	jthread _solveThread;

	vector<int> _equipSlots;
	map<int, int> _gearPieceCounter;
	vector<int> _gearPieceIdx;
	map<int, int> _gearPieceSlotToIdx;
	int _ringPerms;
	int _switchCounter;
	int _maxCounter;

	void solve(stop_token stopToken, Job* job, int level, const map<int, vector<GearPiece*>>& gearPieces, const vector<Food*>& foodList, const vector<int>& releventMateriaBaseParam);
	vector<GearPiece*> initGear(const map<int, vector<GearPiece*>>& gearPieces);
	void switchGear(const map<int, vector<GearPiece*>>& gearPieces, vector<GearPiece*>& gearPiecesToSolve);
	void switchPiece(const map<int, vector<GearPiece*>>& gearPieces, vector<GearPiece*>& gearPiecesToSolve, int slot);
};