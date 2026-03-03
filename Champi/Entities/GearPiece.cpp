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
            if (baseParam[i] == 0 || baseParamValue[i] == 0) continue;
            perm.baseParamTotalValue[baseParam[i]] = baseParamValue[i];
        }
        meldPerms.push_back(perm);
        return;
    }

    set<int> releventMateriaBaseParamSet;
    for (auto& baseParam : releventMateriaBaseParam) {
        releventMateriaBaseParamSet.insert(baseParam);
    }

    // Find capped substat
    int substatCap = -1;
    int maxReleventBaseParam = -1;
    map<int, int> baseParamToIdx;
    for (int i = 0; i < 6; i++) {
        if (baseParam[i] == 0) continue;
        if (!releventMateriaBaseParamSet.contains(baseParam[i])) continue;

        if (baseParamValue[i] > substatCap) {
            substatCap = baseParamValue[i];
            maxReleventBaseParam = baseParam[i];
        }
		baseParamToIdx[baseParam[i]] = i;
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
    set<uint64_t> permStats;
	float materiaIgnorePercent = Settings::Instance().minMateriaRatio;
    int maxSlot = isAdvancedMeldingPermitted ? 4 : materiaSlotCount - 1;
    int firstOvermeldSlot = isAdvancedMeldingPermitted ? materiaSlotCount : -1;
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
            if (baseParam[i] == 0 || baseParamValue[i] == 0) continue;
            perm.baseParamTotalValue[baseParam[i]] = baseParamValue[i];
        }
        bool ignorePerm = false;
        for (int slot = 0; slot < 5; slot++) {
            if (slot >= materiaSlotCount && !isAdvancedMeldingPermitted) break;

            int materiaBaseParam = perm.materia[slot]->baseParam;
			int materiaValue = perm.materia[slot]->value;

            int materiaActualValue = min(materiaValue, substatCap - perm.baseParamTotalValue[materiaBaseParam]);
            if (materiaActualValue < materiaValue * materiaIgnorePercent) {
                ignorePerm = true;
                break;
            }
            if (materiaActualValue == 0) continue;
            perm.baseParamTotalValue[materiaBaseParam] += materiaActualValue;
            perm.baseParamMatValue[materiaBaseParam] += materiaActualValue;
        }

        if (!ignorePerm) {
            int i = 0;
            for (auto& it : releventMateriaBaseParamSet) {
                if (perm.baseParamMatValue.contains(it)) {
                    perm.matKey |= (uint64_t)perm.baseParamMatValue[it] << (12 * i);
                }
                i++;
            }
            // Save stats to prevent duplicates from pentamelds
            if (permStats.contains(perm.matKey)) {
                ignorePerm = true;
            } else {
                permStats.insert(perm.matKey);
            }
        }

        if (!ignorePerm) {
            meldPerms.push_back(perm);
        }

        // Increment idx
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

    // Init min/max stats
    minBaseParamValue.clear();
    maxBaseParamValue.clear();
    maxBaseParamMatValue.clear();
    for (auto& baseParam : releventMateriaBaseParam) {
        int value = baseParamToIdx.contains(baseParam) ? baseParamValue[baseParamToIdx[baseParam]] : 0;
        minBaseParamValue[baseParam] = value;
        maxBaseParamValue[baseParam] = value;
    }
    for (auto& perm : meldPerms) {
        for (auto& [baseParam, value] : perm.baseParamTotalValue) {
            if (find(releventBaseParam.begin(), releventBaseParam.end(), baseParam) == releventBaseParam.end()) continue;

			if (value > maxBaseParamValue[baseParam]) {
                maxBaseParamValue[baseParam] = value;
            }
        }
    }
    for (auto& perm : meldPerms) {
        for (auto& [baseParam, value] : perm.baseParamMatValue) {
            if (find(releventBaseParam.begin(), releventBaseParam.end(), baseParam) == releventBaseParam.end()) continue;

			if (value > maxBaseParamMatValue[baseParam]) {
                maxBaseParamMatValue[baseParam] = value;
            }
        }
    }
}