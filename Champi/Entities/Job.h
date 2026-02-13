#pragma once

#include <string>
#include <set>

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
};
