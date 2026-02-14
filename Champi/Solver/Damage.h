#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <mutex>

#include "../Entities/Job.h"

using namespace std;

class Damage
{
public:
	Damage();
	virtual ~Damage();
	static Damage& Instance() { return _instance; }

	void init(Job* job, int level);

	float wdMod(int wd);
	float mainStatMod(int mainStat);
	float dhMod(int dh);
	float critMod(int crit);
	float detMod(int det);
	float tenMod(int ten);
	int gcd(int ss);
	float damageMod(int wd, int mainStat, int dh, int crit, int det);
	float damageMod(int wd, int mainStat, int dh, int crit, int det, int ten);

	int getLvlModMain() { return _lvlModMain; }
	int getLvlModSub() { return _lvlModSub; }

private:
	static Damage _instance;

	int _jobMod = 100; // Set from job
	int _lvlModAP = 237; // Set from level
	int _lvlModMain = 440; // Set from level
	int _lvlModSub = 420; // Set from level
	int _lvlModDiv = 2780; // Set from level
	int _haste = 0; // Set from job

	vector<float> _wdMod;
	vector<float> _mainStatMod;
	vector<float> _dhMod;
	vector<float> _critMod;
	vector<float> _detMod;
	vector<float> _tenMod;
	vector<int> _gcd;

	mutex _wdMutex;
	mutex _mainStatMutex;
	mutex _dhMutex;
	mutex _critMutex;
	mutex _detMutex;
	mutex _tenMutex;
	mutex _gcdMutex;
};