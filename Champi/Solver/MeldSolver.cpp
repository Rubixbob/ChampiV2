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

    int ssBaseParam = _job->primaryStat == 4 || _job->primaryStat == 5 ? 46 : 45;
    vector<int> jobBaseParams;
    for (auto& baseParam : _releventMateriaBaseParam) {
        if (baseParam == ssBaseParam) continue;
        jobBaseParams.push_back(baseParam);
    }

    if (jobBaseParams.size() < 3 || jobBaseParams.size() > 4) {
        cout << "We shouldn't be here" << endl;
        return;
    }

    auto getKey = jobBaseParams.size() == 4 ? getKey4 : getKey3;

    size_t slotIdx = 0;
    size_t maxSlotIdx = gearPieces.size() - 1;

    float maxCounter = 1.0f;
    vector<float> progressWeight(gearPieces.size(), 1.0f);
    for (int i = 0; i < gearPieces.size(); i++) {
        auto permCount = gearPieces[i]->meldPerms.size();
        maxCounter *= permCount;
        for (int j = 0; j < i; j++) {
            progressWeight[j] *= permCount;
        }
    }

    bool looping = true;
    while (looping) {
        if (_solveStopToken.stop_requested()) {
            statCombs.clear();
            return;
        }

        gearSet.addMeldPerm(&gearPieces[slotIdx]->meldPerms[currentPerm[slotIdx]]);

        auto key = getKey(jobBaseParams, gearSet.meldedBaseParamValue);
        auto& statCombsSS = statCombs[slotIdx][gearSet.meldedBaseParamValue.at(ssBaseParam)];

        auto [it, inserted] = statCombsSS.insert(key);
        if (inserted && slotIdx == maxSlotIdx) {
            gearSet.initFedMeldedStats();

            for (auto food : _foodList) {
                gearSet.addFood(food);
                float damageMod = gearSet.fedMeldedDamageMod();
                int gcd = gearSet.fedMeldedGcd();
                int idx = 250 - gcd;
                auto [res, emplaced] = results.try_emplace(idx, gearSet);
                if (!emplaced && damageMod > res->second.damageMod) {
                    res->second = gearSet;
                }
                gearSet.popFood();
            }
        }

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

        float currentCount = 0;
        for (size_t slot = 0; slot < gearPieces.size(); slot++) {
            currentCount += currentPerm[slot] * progressWeight[slot];
        }
        solvingProgress = currentCount / maxCounter;
    }

    statCombs.clear();
    (*_solveActiveThreads)--;
    done = true;
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