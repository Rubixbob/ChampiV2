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

	inline static const int Strength = 1;
	inline static const int Dexterity = 2;
	inline static const int Vitality = 3;
	inline static const int Intelligence = 4;
	inline static const int Mind = 5;
	inline static const int Piety = 6;
	inline static const int DamagePhys = 12;
	inline static const int DamageMag = 13;
	inline static const int Tenacity = 19;
	inline static const int DirectHit = 22;
	inline static const int CriticalHit = 27;
	inline static const int Determination = 44;
	inline static const int SkillSpeed = 45;
	inline static const int SpellSpeed = 46;

	static int getStartingValue(int baseParam, int lvlModMain, int lvlModSub);
};
