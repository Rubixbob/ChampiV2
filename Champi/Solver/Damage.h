#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <mutex>

#include "../Entities/Job.h"
#include "../Entities/BaseParam.h"

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
	int tenModIntToMin(int tenModInt);
	int tenModIntToMax(int tenModInt);
	int pieMod(int pie);
	int pieModToMin(int pie);
	int pieModToMax(int pie);
	int gcd(int ss);
	int gcdToMinSpeed(int gcd);
	int gcdToMaxSpeed(int gcd);
	float damageMod(int wd, int mainStat, int dh, int crit, int det);
	float damageMod(int wd, int mainStat, int dh, int crit, int det, int ten);

	int getLvlModMain() { return _lvlModMain; }
	int getLvlModSub() { return _lvlModSub; }
	int getStartingValue(int baseParam);

private:
	static Damage _instance;

	Job* _selectedJob = nullptr;
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
	map<int, int> _tenModIntToMin;
	map<int, int> _tenModIntToMax;
	vector<int> _pieMod;
	map<int, int> _pieModToMin;
	map<int, int> _pieModToMax;
	vector<int> _gcd;
	map<int, int> _gcdToMinSpeed;
	map<int, int> _gcdToMaxSpeed;

	mutex _wdMutex;
	mutex _mainStatMutex;
	mutex _dhMutex;
	mutex _critMutex;
	mutex _detMutex;
	mutex _tenMutex;
	mutex _gcdMutex;
};