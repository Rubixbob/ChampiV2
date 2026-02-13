#pragma once

#include <string>

using namespace std;

class Food
{
public:
	Food();
	virtual ~Food();

	int id; // #
	string name; // Name
	int valueHQ[3]; // ValueHQ[0] to [2]
	int maxHQ[3]; // MaxHQ[0] to [2]
	int baseParam[3]; // BaseParam[0] to [2]
	int levelItem; // LevelItem
};