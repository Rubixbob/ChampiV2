#pragma once

#include <string>
#include <set>

#include "BaseParam.h"

using namespace std;

class Job
{
public:
	Job();
	virtual ~Job();

	int id; // #
	string name; // Abbreviation
	int primaryStat; // PrimaryStat
	int primaryStatMod; // ModifierXxx
	int role; // Role
	set<int> categories;

	int getSpeedBaseParam() const;
	int getWeaponDamageBaseParam() const;
};
