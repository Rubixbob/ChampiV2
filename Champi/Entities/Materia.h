#pragma once

#include <string>
#include <map>

using namespace std;

class Materia
{
public:
	Materia();
	virtual ~Materia();

	int id; // #
	string name; // Name
	int grade;
	int value; // Value
	int baseParam; // BaseParam
	int levelItem; // LevelItem
	int overmeldPercent[4]; // OvermeldHQPercent[0] to [3]

	inline static map<int, int> valueByGrade;
};