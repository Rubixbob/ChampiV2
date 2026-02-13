#pragma once

#include <string>
#include <map>

using namespace std;

class BaseParam
{
public:
	BaseParam();
	virtual ~BaseParam();

	int id; // #
	string name; // Name
	int twoHandWeaponPercent; // TwoHandWeaponPercent
	int oneHandWeaponPercent; // OneHandWeaponPercent
	int offHandPercent; // OffHandPercent
	int bigLeftPercent; // ChestPercent
	int smallLeftPercent; // HeadPercent
	int rightPercent; // RingPercent

	inline static const map<int, string> abbr = {
		{1, "STR"},
		{2, "DEX"},
		{3, "VIT"},
		{4, "INT"},
		{5, "MND"},
		{6, "PIE"},
		{12, "WD"},
		{13, "WD"},
		{19, "TNC"},
		{22, "DHT"},
		{27, "CRT"},
		{44, "DET"},
		{45, "SKS"},
		{46, "SPS"}
	};
};
