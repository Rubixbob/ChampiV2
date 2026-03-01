#include "MeldSolver.h"

MeldSolver::MeldSolver() {
    //ctor
}

MeldSolver::MeldSolver(Job* job, vector<GearPiece*> gearPieces, vector<Food*> foodList, vector<int> releventMateriaBaseParam, atomic_int* activeThreads, stop_token stopToken)
    : _job(job), gearPieces(gearPieces), _foodList(foodList), _releventMateriaBaseParam(releventMateriaBaseParam), _solveActiveThreads(activeThreads), _solveStopToken(stopToken) {
    //ctor
}

MeldSolver::~MeldSolver() {
    //dtor
}

void MeldSolver::findBestMelds() {
    vector<unordered_map<int, unordered_set<uint64_t>>> statCombs(gearPieces.size());
    vector<size_t> currentPerm(gearPieces.size());

    GearSet gearSet(_job, gearPieces);

    // List of stats used to build the key for saving already explored stat combinations
    vector<int> jobBaseParams;
    int ssBaseParam = _job->getSpeedBaseParam();
    for (auto& baseParam : _releventMateriaBaseParam) {
        if (baseParam == ssBaseParam) continue;
        jobBaseParams.push_back(baseParam);
    }

    if (jobBaseParams.size() < 3 || jobBaseParams.size() > 4) {
        cout << "We shouldn't be here" << endl;
        return;
    }

    auto getKey = jobBaseParams.size() == 4 ? getKey4 : getKey3;

    // Structure to display solving progress
    float maxCounter = 1.0f;
    vector<float> progressWeight(gearPieces.size(), 1.0f);
    for (int i = 0; i < gearPieces.size(); i++) {
        auto permCount = gearPieces[i]->meldPerms.size();
        maxCounter *= permCount;
        for (int j = 0; j < i; j++) {
            progressWeight[j] *= permCount;
        }
    }

    // Build the possible stat range for every slot to check against user selected range
    for (auto& [baseParam, _] : _baseParamRangeSelected) {
        auto& slotRange = _slotBaseParamRange[baseParam];
        pair<int, int> gearRange = make_pair(0, 0);
        slotRange.push_front(gearRange);
        for (auto slot = gearPieces.size() - 1; slot >= 1; slot--) {
            auto piece = gearPieces[slot];
            gearRange.second += piece->maxBaseParamMatValue[baseParam];
            slotRange.push_front(gearRange);
        }
        pair<int, int> foodRange = make_pair(_foodList.size() > 0 ? INT32_MAX : 0, 0);
        for (auto food : _foodList) {
            int foodValue = 0;
            for (int i = 0; i < 3; i++) {
                if (food->baseParam[i] != baseParam) continue;

                // For performance we take the max value instead of recalculating the % value at every step
                foodValue = food->maxHQ[i];
            }
            if (foodValue < foodRange.first) {
                foodRange.first = foodValue;
            }
            if (foodValue > foodRange.second) {
                foodRange.second = foodValue;
            }
        }
        for (auto& range : slotRange) {
            range.first += foodRange.first;
            range.second += foodRange.second;
        }
    }

    /* Main loop
    * Depth first search of meld combinations
    * All meld combinations are explored as a tree where the meld combinations of the weapon
    * would be the children of the root and the meld combinations of the second ring are leaves
    * 
    * We start by picking a combination for main hand then we go through pieces one by one until second ring
    * When all pieces have a meld combination selected we go through the foods
    * At each node we mark the total stats combination as visited in statCombs so we avoid exploring an identical part of the tree later
    * At each node we also check if we are in the user selected stat range
    * When we can't go down we explorer the next node on the same level
    * When all nodes of the same level are explored we go up
    * 
    * Credits to June for the layer by layer implementation on the xiv gear meld solver which inspired me to do this version that is lighter memory wise
    */
    bool looping = true;
    size_t slotIdx = 0;
    size_t maxSlotIdx = gearPieces.size() - 1;
    while (looping) {
        if (_solveStopToken.stop_requested()) {
            statCombs.clear(); // Clear memory in this thread otherwise it would slow down the main thread
            return;
        }
        gearSet.addMeldPerm(&gearPieces[slotIdx]->meldPerms[currentPerm[slotIdx]]);

        auto key = getKey(jobBaseParams, gearSet.meldedBaseParamValue);
        auto meldedSsBaseParamValue = gearSet.meldedBaseParamValue.at(ssBaseParam);
        auto& statCombsSS = statCombs[slotIdx][meldedSsBaseParamValue];

        // Check we are in the selected stat range before adding more melds and food
        bool isInRange = true;
        for (auto& [baseParam, selectedRange] : _baseParamRangeSelected) {
            // Check if there is any intersection between selectedRange and meldedBaseParamValue + slotRange
            auto meldedBaseParamValue = gearSet.meldedBaseParamValue.at(baseParam);
            auto& slotRange = _slotBaseParamRange[baseParam][slotIdx];
            if (meldedBaseParamValue + slotRange.first > selectedRange.second || meldedBaseParamValue + slotRange.second < selectedRange.first) {
                isInRange = false;
                break;
            }
        }

        auto [it, inserted] = statCombsSS.insert(key);
        inserted = inserted && isInRange;
        if (inserted && slotIdx == maxSlotIdx) {
            gearSet.initFedMeldedStats();

            for (auto food : _foodList) {
                gearSet.addFood(food);

                // Check we are in the selected stat range before saving the result
                bool isInRange = true;
                for (auto& [baseParam, selectedRange] : _baseParamRangeSelected) {
                    // Check if fedMeldedBaseParamValue is in selectedRange
                    auto fedMeldedBaseParamValue = gearSet.fedMeldedBaseParamValue.at(baseParam);
                    if (fedMeldedBaseParamValue > selectedRange.second || fedMeldedBaseParamValue < selectedRange.first) {
                        isInRange = false;
                        break;
                    }
                }

                // Check against saved result and save
                if (isInRange) {
                    float damageMod = gearSet.fedMeldedDamageMod();
                    int gcd = gearSet.fedMeldedGcd();
                    int idx = 250 - gcd;
                    auto [res, emplaced] = results.try_emplace(idx, gearSet); // Always save if there isn't already a result for this gcd
                    if (!emplaced && damageMod > res->second.damageMod) { // If there was already a result compare damage mods
                        res->second = gearSet;
                    }
                }
                gearSet.popFood();
            }
        }

        // Go to next node
        if (inserted && slotIdx < maxSlotIdx) {
            slotIdx++;
        } else {
            gearSet.popMeldPerm();
            currentPerm[slotIdx]++;
            while (currentPerm[slotIdx] >= gearPieces[slotIdx]->meldPerms.size()) {
                if (slotIdx > 0) {
                    currentPerm[slotIdx] = 0;
                    slotIdx--;
                    gearSet.popMeldPerm();
                    currentPerm[slotIdx]++;
                } else {
                    looping = false;
                    break;
                }
            }
        }

        // Update progress for visual feedback
        float currentCount = 0;
        for (size_t slot = 0; slot < gearPieces.size(); slot++) {
            currentCount += currentPerm[slot] * progressWeight[slot];
        }
        solvingProgress = currentCount / maxCounter;
    }

    statCombs.clear(); // Clear memory in this thread otherwise it would slow down the main thread
    (*_solveActiveThreads)--;
    done = true;
}

void MeldSolver::setBaseParamRanges(const map<int, pair<int, int>>& baseParamRangeSelected) {
    _baseParamRangeSelected = baseParamRangeSelected;
}

uint64_t MeldSolver::getKey3(const vector<int>& jobBaseParams, const map<int, int>& baseParamValue) {
    return (uint64_t)baseParamValue.at(jobBaseParams[0]) << 32
         | (uint64_t)baseParamValue.at(jobBaseParams[1]) << 16
         | (uint64_t)baseParamValue.at(jobBaseParams[2]);
}

uint64_t MeldSolver::getKey4(const vector<int>& jobBaseParams, const map<int, int>& baseParamValue) {
    return (uint64_t)baseParamValue.at(jobBaseParams[0]) << 48
         | (uint64_t)baseParamValue.at(jobBaseParams[1]) << 32
         | (uint64_t)baseParamValue.at(jobBaseParams[2]) << 16
         | (uint64_t)baseParamValue.at(jobBaseParams[3]);
}