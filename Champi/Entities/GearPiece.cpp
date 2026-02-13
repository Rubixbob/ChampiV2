#include "GearPiece.h"

GearPiece::GearPiece()
{
    //ctor
}

GearPiece::~GearPiece()
{
    //dtor
}

void GearPiece::setMeldPerms(const vector<int>& releventMateriaBaseParam, vector<Materia>& materiaList) {
    meldPerms.clear();
    if (materiaSlotCount == 0) {
        MeldPerm perm;
        perm.gearPiece = this;
        for (int i = 0; i < 6; i++) {
            if (baseParam[i] == 0 || baseParamValue[i] == 0) break;
            perm.baseParamTotalValue[baseParam[i]] = baseParamValue[i];
        }
        meldPerms.push_back(perm);
        return;
    }

    // Find capped substat
    int substatCap = -1;
    int maxReleventBaseParam = -1;
    for (int i = 0; i < 6; i++) {
        if (baseParam[i] == 0) break;
        if (find(releventMateriaBaseParam.begin(), releventMateriaBaseParam.end(), baseParam[i]) == releventMateriaBaseParam.end()) continue;

        if (baseParamValue[i] > substatCap) {
            substatCap = baseParamValue[i];
            maxReleventBaseParam = baseParam[i];
        }
    }

    // Exclude capped substat
    vector<int> releventBaseParam;
    for (auto& materiaBaseParam : releventMateriaBaseParam) {
        if (materiaBaseParam == maxReleventBaseParam) continue;
        releventBaseParam.push_back(materiaBaseParam);
    }

    // Find best materia to use for each slot then for each stat
    map<int, Materia*> slotMateria[5];
    for (auto& materia : materiaList) {
        if (materia.levelItem > levelItem) continue; // Ignore too high grade materia
        if (find(releventBaseParam.begin(), releventBaseParam.end(), materia.baseParam) == releventBaseParam.end()) continue;

        for (int slot = 0; slot < 5; slot++) {
            if (slot >= materiaSlotCount && !isAdvancedMeldingPermitted) break;
            int overmeldIdx = slot - materiaSlotCount;
            if (overmeldIdx >= 0 && materia.overmeldPercent[overmeldIdx] == 0) continue;

            if (!slotMateria[slot].contains(materia.baseParam) || materia.value > slotMateria[slot][materia.baseParam]->value) {
                slotMateria[slot][materia.baseParam] = &materia;
            }
        }
    }

    // Sanity check
    for (int slot = 0; slot < 5; slot++) {
        if (slot >= materiaSlotCount && !isAdvancedMeldingPermitted) break;
        if (slotMateria[slot].size() != releventBaseParam.size()) {
            cout << "slotMateria has the wrong size, we shouldn't be here" << slotMateria[slot].size() << " " << releventBaseParam.size() << endl;
            return;
        }
    }

    // Build all meld permutations
    int baseParamIdx[5] = { 0 }; // Index of what materia to use for each slot
    set<string> permStats;
	int materiaIgnorePercent = Settings::Instance().minMateriaRatio;
    bool done = false;
    while (!done) {
        MeldPerm perm;
        perm.gearPiece = this;
        for (int slot = 0; slot < 5; slot++) {
            if (slot >= materiaSlotCount && !isAdvancedMeldingPermitted) break;

            auto materia = slotMateria[slot][releventBaseParam[baseParamIdx[slot]]];
            perm.materia[slot] = materia;
            perm.baseParamMatCount[materia->grade][materia->baseParam]++;
        }

        // Calc materia and total value
        for (int i = 0; i < 6; i++) {
            if (baseParam[i] == 0 || baseParamValue[i] == 0) break;
            perm.baseParamTotalValue[baseParam[i]] = baseParamValue[i];
        }
        bool ignorePerm = false;
        for (int slot = 0; slot < 5; slot++) {
            if (slot >= materiaSlotCount && !isAdvancedMeldingPermitted) break;

            int materiaValue = min(perm.materia[slot]->value, substatCap - perm.baseParamTotalValue[perm.materia[slot]->baseParam]);
            if (materiaValue < perm.materia[slot]->value * materiaIgnorePercent) {
                ignorePerm = true;
                break;
            }
            if (materiaValue == 0) continue;
            perm.baseParamTotalValue[perm.materia[slot]->baseParam] += materiaValue;
            perm.baseParamMatValue[perm.materia[slot]->baseParam] += materiaValue;
        }

        if (!ignorePerm) {
            // Save stats to prevent duplicates from pentamelds
            string permKey = "";
            for (int i = 0; i < releventBaseParam.size(); i++) {
                permKey += to_string(perm.baseParamMatValue.contains(releventBaseParam[i])  ? perm.baseParamMatValue[releventBaseParam[i]] : 0);
                if (i < releventBaseParam.size() - 1) permKey += "|";
            }
            if (permStats.contains(permKey)) {
                ignorePerm = true;
            } else {
                permStats.insert(permKey);
            }
        }

        if (!ignorePerm) {
            meldPerms.push_back(perm);

            // Max available slots for the piece
            for (auto& gradeIt : perm.baseParamMatCount) {
                for (auto& baseParamIt : gradeIt.second) {
                    if (availableSlots[gradeIt.first][baseParamIt.first] < baseParamIt.second) {
                        availableSlots[gradeIt.first][baseParamIt.first] = baseParamIt.second;
                    }
                }
            }
        }

        // Increment idx
        int maxSlot = isAdvancedMeldingPermitted ? 4 : materiaSlotCount - 1;
        int firstOvermeldSlot = isAdvancedMeldingPermitted ? materiaSlotCount : -1;
        int slot = maxSlot;
        while (true) {
            baseParamIdx[slot]++;
            if (baseParamIdx[slot] == releventBaseParam.size()) {
                if (slot == 0) {
                    done = true;
                    break;
                }
                if (slot == firstOvermeldSlot + 1) {
                    for (int i = slot; i <= maxSlot; i++) {
                        baseParamIdx[i] = 0;
                    }
                }
                slot--;
            } else {
                for (int i = slot + 1; i <= (slot <= firstOvermeldSlot ? firstOvermeldSlot : maxSlot); i++) {
                    baseParamIdx[i] = baseParamIdx[slot];
                }
                break;
            }
        }
    }
}