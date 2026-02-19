#pragma once

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>

#include "../Entities/Food.h"
#include "../Entities/Materia.h"
#include "../Entities/GearPiece.h"
#include "../Entities/Job.h"

#include "../Solver/Damage.h"

using namespace std;

class GearSet
{
public:
	GearSet();
	GearSet(const Job* job, const vector<GearPiece*>& gearPieces);
	virtual ~GearSet();

	vector<MeldPerm*> meldPerms;
	float damageMod;
	int gcdInt; // Actual GCD x100
	Food* selectedFood = nullptr;
	const Job* selectedJob = nullptr;

	int damagePhys;
	int damageMag;
	int delayms;

	map<int, int> meldedBaseParamValue;
	map<int, int> fedMeldedBaseParamValue;
	int mainBaseParamValue;

	void addMeldPerm(MeldPerm* meldPerm);
	void popMeldPerm();
	void addFood(Food* food);
	void popFood();
	void initFedMeldedStats();

	float meldedDamageMod();
	float fedMeldedDamageMod();

	int meldedGcd();
	int fedMeldedGcd();
	string gcdStr();

	string toJson();
private:
	float calcDamageMod(const map<int, int>& baseParamValue);
	int gcd(const map<int, int>& baseParamValue);
};