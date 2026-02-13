#pragma once

#include <iostream>
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
	GearSet(vector<GearPiece*> gearPieces);
	virtual ~GearSet();

	vector<MeldPerm*> meldPerms;
	float damageMod;
	int gcdInt; // Actual GCD x100
	Food* selectedFood = nullptr;

	int damagePhys;
	int damageMag;
	int delayms;

	map<int, int> setBaseParamValue;
	map<int, int> meldedBaseParamValue;
	map<int, int> fedMeldedBaseParamValue;
	int mainBaseParamValue;

	void initStats(const Job* job);
	void updateMeldedStats();
	void updateMeldedStats(const map<int, vector<int>>& meldComb, const vector<int>& releventMateriaBaseParam);
	void updateFedMeldedStats(Food* food);

	float setDamageMod();
	float meldedDamageMod();
	float fedMeldedDamageMod();

	int setGcd();
	int meldedGcd();
	int fedMeldedGcd();
private:
	const Job* _job = nullptr;

	float calcDamageMod(const map<int, int>& baseParamValue);
	int gcd(const map<int, int>& baseParamValue);
};