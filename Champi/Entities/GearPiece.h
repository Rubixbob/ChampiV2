#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>

#include "Materia.h"

#include "../Solver/Settings.h"

using namespace std;

class GearPiece;

struct MeldPerm
{
	GearPiece* gearPiece = nullptr;
	Materia* materia[5] = { nullptr };
	map<int, map<int, int>> baseParamMatCount; // Mat count per grade then per baseParam
	map<int, int> baseParamMatValue;
	map<int, int> baseParamTotalValue;
};

class GearPiece
{
public:
    GearPiece();
    virtual ~GearPiece();

    int id; // #
    string name; // Name
	int equipSlotCategory; // EquipSlotCategory
	int damagePhys; // DamagePhys
	int damageMag; // DamageMag
	int delayms; // Delayms
	int baseParamValue[6]; // BaseParamValue[0] to [5]
	int baseParam[6]; // BaseParam[0] to [5]
	int levelEquip; // LevelEquip
	int classJobCategory; // ClassJobCategory
	int materiaSlotCount; // MateriaSlotCount
	bool isAdvancedMeldingPermitted; // IsAdvancedMeldingPermitted
	int icon; // Icon
	int levelItem; // LevelItem
	bool isUnique; // IsUnique

	vector<MeldPerm> meldPerms;
	map<int, map<int, int>> availableSlots; // Max available slots per grade then per baseParam

	inline static const map<int, string> equipSlotName = {
		{1, "Weapon"},
		{2, "Shield"},
		{3, "Head"},
		{4, "Body"},
		{5, "Hands"},
		{7, "Legs"},
		{8, "Feet"},
		{9, "Earrings"},
		{10, "Necklace"},
		{11, "Bracelets"},
		{12, "Ring"},
		{13, "Weapon"}
	};

	inline static const map<int, string> equipSlotXivGearName = {
		{1, "Weapon"},
		{2, "OffHand"},
		{3, "Head"},
		{4, "Body"},
		{5, "Hand"},
		{7, "Legs"},
		{8, "Feet"},
		{9, "Ears"},
		{10, "Neck"},
		{11, "Wrist"},
		{12, "Ring"},
		{13, "Weapon"}
	};

	inline static const map<int, string> equipSlotXivGearRingSuffix = {
		{0, "Right"},
		{1, "Left"}
	};

	void setMeldPerms(const vector<int>& releventMateriaBaseParam, vector<Materia>& materiaList);
};