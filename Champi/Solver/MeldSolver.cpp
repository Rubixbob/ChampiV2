#include "MeldSolver.h"

MeldSolver::MeldSolver() {
    //ctor
}

MeldSolver::MeldSolver(const Job* job, vector<GearPiece*> gearPieces, vector<Food*> foodList, vector<int> releventMateriaBaseParam, atomic_int* activeThreads, stop_token stopToken)
    : _job(job), gearPieces(gearPieces), _foodList(foodList), _releventMateriaBaseParam(releventMateriaBaseParam), _solveActiveThreads(activeThreads), _solveStopToken(stopToken) {
    //ctor
}

MeldSolver::~MeldSolver() {
    //dtor
}

void MeldSolver::findBestMelds() {
    vector<int> materiaGrades;
    set<int> materiaGradesTemp;
    map<int, int> maxMeldSlots; // Max slots per grade of materia
    map<int, int> materiaBaseParamToIdx;
    for (auto piece : gearPieces) {
        if (piece->meldPerms.size() == 0) continue;

        for (auto materia : piece->meldPerms[0].materia) {
            if (materia == nullptr) break;
            materiaGradesTemp.insert(materia->grade);
            maxMeldSlots[materia->grade]++;
        }
    }
    if (materiaGradesTemp.size() > 2) return; // TODO: display properly that we can't solve for more than 2 different materia grades
    for (int grade : materiaGradesTemp) {
        materiaGrades.push_back(grade);
    }
    for (int idx = 0; idx < _releventMateriaBaseParam.size(); idx++) {
        materiaBaseParamToIdx[_releventMateriaBaseParam[idx]] = idx;
    }

    GearSet gearSet(gearPieces);
    gearSet.initStats(_job);

    // Meld combinations init
    map<int, vector<int>> meldComb;
    for (int grade : materiaGrades) {
        meldComb[grade].push_back(maxMeldSlots[grade]);
        for (int i = 1; i < _releventMateriaBaseParam.size(); i++) {
            meldComb[grade].push_back(0);
        }
        int s = maxMeldSlots[grade];
        _maxCounter *= _releventMateriaBaseParam.size() == 4 ? (s + 1) * (s + 2) * (s + 3) / 6 : (s + 1) * (s + 2) * (s + 3) * (s + 4) / 24;
    }

    if (_maxCounter == 1) {
        checkAndSaveSet(gearSet);
        (*_solveActiveThreads)--;
        done = true;
        return;
    }

    int highestGrade = materiaGrades[materiaGrades.size() - 1];
    int lastBaseParam = _releventMateriaBaseParam.size() - 1;
    int secondToLastBaseParam = _releventMateriaBaseParam.size() - 2;

    // Materia slots init
    map<int, vector<int>> availableSlots;
    map<int, vector<int>> availableSlotsInit;
    map<int, vector<int>> neededSlots;
    for (int grade : materiaGrades) {
        for (int i = 0; i < _releventMateriaBaseParam.size(); i++) {
            availableSlots[grade].push_back(0);
            availableSlotsInit[grade].push_back(0);
            neededSlots[grade].push_back(0);
        }
    }
    for (auto piece : gearPieces) {
        for (auto& gradeIt : piece->availableSlots) {
            for (auto& baseParamIt : gradeIt.second) {
            //for (int idx = 0; idx < _releventMateriaBaseParam.size(); idx++) {
                availableSlotsInit[gradeIt.first][materiaBaseParamToIdx[baseParamIt.first]] += baseParamIt.second;
            }
        }
    }

    // Score init
    int maxScore = 0;
    for (auto& gradeIt : maxMeldSlots) {
        maxScore += gradeIt.second * Materia::valueByGrade[gradeIt.first];
    }
    map<uint64_t, int> savedScores;

    while (meldComb[highestGrade][lastBaseParam] <= maxMeldSlots[highestGrade]) {
        if (_solveStopToken.stop_requested()) {
            return;
        }
        bool possible = true;
        // Init
        for (int grade : materiaGrades) {
            for (int i = 0; i < _releventMateriaBaseParam.size(); i++) {
                neededSlots[grade][i] = meldComb[grade][i];
                availableSlots[grade][i] = availableSlotsInit[grade][i];
                if (neededSlots[grade][i] > availableSlots[grade][i]) {
                    possible = false;
                    break;
                }
            }
            if (!possible) break;
        }

        if (possible) {
            // Check DPS with perfect melds
            gearSet.updateMeldedStats(meldComb, _releventMateriaBaseParam);

            possible = false;
            // Iterate on foods
            for (auto food : _foodList) {
                gearSet.updateFedMeldedStats(food);
                // Compute dps and compare to saved sets
                //if (maxFoodMeldedGearStat.subs[3] >= MIN_SKS && gearStats.subs[3] <= MAX_SKS) {
                float damageMod = gearSet.fedMeldedDamageMod();
                int gcd = gearSet.fedMeldedGcd();
                int minGcd = gearSet.setGcd();
                    //if (gearStats.subs[3] < MIN_SKS)
                        //minGcd = floor(floor(2.5 * (1000 - floor(130 * (MIN_SKS - 420) / 2780))) * (100 - haste) / 1000);
                int idx = 250 - gcd;
                int minIdx = 250 - minGcd;
                // Check all saved dps within gcd range
                for (int i = minIdx; i <= idx; i++) {
                    if (!results.contains(i) || damageMod > results[i].damageMod)
                        possible = true;
                }
                //}
            }
        }

        if (possible) {
            // Find melds
            vector<int> currentPerm(gearPieces.size());
            vector<int> savedPerm(gearPieces.size());
            int idx = 0;
            int score = 0;
            int savedScore = 0;
            int scoreGoal = 0;
            bool scoreGiven = false;
            while (idx < gearPieces.size() && idx >= 0) {
                auto piece = gearPieces[idx];
                auto& perm = piece->meldPerms[currentPerm[idx]];
                // Check if perm can be used
                bool useful = true;
                for (int grade : materiaGrades) {
                    for (auto& it : perm.baseParamMatCount[grade]) {
                        if (it.second > neededSlots[grade][materiaBaseParamToIdx[it.first]]) {
                            useful = false;
                            break;
                        }
                    }
                    if (!useful) break;
                }
                if (useful) {
                    int currentSaveKey = getSaveKey(neededSlots);
                    int currentSavedScore = savedScores.contains(currentSaveKey) ? savedScores[currentSaveKey] : -1;
                    int nextScoreGoal = currentSavedScore;
                    for (auto& it : perm.baseParamMatValue) {
                        nextScoreGoal -= it.second;
                    }
                    int nextSaveKey = currentSaveKey - getSaveKey(perm.baseParamMatCount, materiaGrades, materiaBaseParamToIdx);

                    if (scoreGiven) {
                        if (idx < gearPieces.size() - 1 && (!savedScores.contains(nextSaveKey) || savedScores[nextSaveKey] < nextScoreGoal)) {
                            useful = false;
                        }
                    } else if (savedScores.contains(nextSaveKey)) {
                        int nextSavedScore = savedScores[nextSaveKey];
                        if (nextSavedScore > 0) {
                            if (nextSavedScore > nextScoreGoal) {
                                scoreGiven = true;
                                scoreGoal = nextSavedScore;
                            } else {
                                useful = false;
                            }
                        } else if (nextSavedScore == 0) {
                            useful = false;
                        }
                    }
                }

                // If useful, take the slots and go to next piece
                if (useful) {
                    for (int grade : materiaGrades) {
                        for (auto& it : piece->meldPerms[currentPerm[idx]].baseParamMatCount[grade]) {
                            availableSlots[grade][materiaBaseParamToIdx[it.first]] -= it.second;
                            neededSlots[grade][materiaBaseParamToIdx[it.first]] -= it.second;
                        }
                    }
                    idx++;
                    if (idx < gearPieces.size()) {
                        piece = gearPieces[idx];
                    } else {
                        score = 0;
                        map<int, vector<int>> path;
                        for (int grade : materiaGrades) {
                            for (int i = 0; i < _releventMateriaBaseParam.size(); i++) {
                                path[grade].push_back(0);
                            }
                        }
                        bool saving = true;
                        for (int j = gearPieces.size() - 1; j >= 0; j--) {
                            // Save score on the path
                            for (auto& baseParamIt : gearPieces[j]->meldPerms[currentPerm[j]].baseParamMatValue) {
                                score += baseParamIt.second;
                            }
                            for (auto& gradeIt : gearPieces[j]->meldPerms[currentPerm[j]].baseParamMatCount) {
                                for (auto& baseParamIt : gradeIt.second) {
                                    path[gradeIt.first][materiaBaseParamToIdx[baseParamIt.first]] += baseParamIt.second;
                                }
                            }
                            int saveKey = getSaveKey(path);
                            if (saving && savedScores.contains(saveKey) && score < savedScores[saveKey]) {
                                saving = false;
                            }
                            if (saving) {
                                savedScores[saveKey] = score;
                            }
                        }
                        if (score < maxScore) {
                            // Compare to saved score and save
                            if (score > savedScore) {
                                for (int i = 0; i < gearPieces.size(); i++) {
                                    savedPerm[i] = currentPerm[i];
                                }
                                savedScore = score;
                            }
                            useful = false;
                            idx--;
                            piece = gearPieces[idx];
                            for (int grade : materiaGrades) {
                                for (auto& it : piece->meldPerms[currentPerm[idx]].baseParamMatCount[grade]) {
                                    availableSlots[grade][materiaBaseParamToIdx[it.first]] += it.second;
                                    neededSlots[grade][materiaBaseParamToIdx[it.first]] += it.second;
                                }
                            }
                            if (scoreGiven) {
                                currentPerm[idx] = piece->meldPerms.size();
                            }
                        }
                    }
                }
                // If not, go to next perm or up
                if (!useful) {
                    currentPerm[idx]++;
                    while (idx >= 0 && (currentPerm[idx] >= piece->meldPerms.size())) {
                        currentPerm[idx] = 0;
                        int saveKey = getSaveKey(neededSlots);
                        if (!savedScores.contains(saveKey)) {
                            savedScores[saveKey] = 0;
                        } else if (savedScores[saveKey] == scoreGoal) {
                            scoreGoal = 0;
                            scoreGiven = false;
                        }
                        idx--;
                        if (idx >= 0) {
                            piece = gearPieces[idx];
                            // Give back slots and go to next perm
                            for (int grade : materiaGrades) {
                                for (auto& it : piece->meldPerms[currentPerm[idx]].baseParamMatCount[grade]) {
                                    availableSlots[grade][materiaBaseParamToIdx[it.first]] += it.second;
                                    neededSlots[grade][materiaBaseParamToIdx[it.first]] += it.second;
                                }
                            }
                            // If score is given and we're here, score has been found and we're going up
                            if (scoreGiven) {
                                currentPerm[idx] = piece->meldPerms.size();
                            } else {
                                currentPerm[idx]++;
                            }
                        }
                    }
                }

            }

            if (score == maxScore) {
                for (int i = 0; i < gearPieces.size(); i++) {
                    savedPerm[i] = currentPerm[i];
                }
                savedScore = score;
            }

            // Create set + melds
            if (savedScore > 0) {
                for (int i = 0; i < gearPieces.size(); i++) {
                    gearSet.meldPerms[i] = &gearPieces[i]->meldPerms[savedPerm[i]];
                }
                checkAndSaveSet(gearSet);
            }
        }

        switchToNextCombination(materiaGrades, maxMeldSlots, meldComb, lastBaseParam, secondToLastBaseParam);
        _switchCounter++;
        solvingProgress = (float)_switchCounter / _maxCounter;
    }

    (*_solveActiveThreads)--;
    done = true;
}

uint64_t MeldSolver::getSaveKey(const map<int, vector<int>>& slots) {
    uint64_t key = 0;
    int cpt = 0;
    for (auto& gradeIt : slots) {
        for (int idx = 0; idx < gradeIt.second.size(); idx++) {
            key |= slots.at(gradeIt.first)[idx] << (cpt * 6);
            cpt++;
        }
    }
    return key;
}

uint64_t MeldSolver::getSaveKey(const map<int, map<int, int>>& slots, const vector<int>& materiaGrades, const map<int, int>& materiaBaseParamToIdx) {
    uint64_t key = 0;
    for (int idx = 0; idx < materiaGrades.size(); idx++) {
        auto grade = materiaGrades[idx];
        if (!slots.contains(grade)) continue;
        int cpt = idx * materiaBaseParamToIdx.size();
        for (auto& baseParamIt : slots.at(grade)) {
            key |= baseParamIt.second << ((cpt + materiaBaseParamToIdx.at(baseParamIt.first)) * 6);
        }
    }
    return key;
}

void MeldSolver::checkAndSaveSet(GearSet& gearSet) {
    gearSet.updateMeldedStats();

    // Iterate on foods
    for (auto food : _foodList) {
        gearSet.updateFedMeldedStats(food);
        // Compute dps and compare to saved sets
        //if (foodMeldedGearStat.subs[3] < MIN_SKS && foodMeldedGearStat.subs[3] > MAX_SKS) continue;
        float damageMod = gearSet.fedMeldedDamageMod();
        int gcd = gearSet.fedMeldedGcd();
        int idx = 250 - gcd;
        if (!results.contains(idx) || damageMod > results[idx].damageMod) {
            results[idx] = gearSet;
        }
    }
}

void MeldSolver::switchToNextCombination(const vector<int>& materiaGrades, const map<int, int>& maxMeldSlots, map<int, vector<int>>& meldComb, int lastBaseParam, int secondToLastBaseParam) {
    int grade = materiaGrades[0];
    auto& meldCombGrade = meldComb[grade];
    if (meldCombGrade[secondToLastBaseParam] + meldCombGrade[lastBaseParam] == maxMeldSlots.at(grade)) meldCombGrade[lastBaseParam]++;
    if (meldCombGrade[0] > 0) {
        meldCombGrade[0]--;
        meldCombGrade[1]++;
    }
    else {
        if (meldCombGrade[1] > 0) {
            meldCombGrade[0] = meldCombGrade[1] - 1;
            meldCombGrade[1] = 0;
            meldCombGrade[2]++;
        }
        else {
            if (meldCombGrade[2] > 0) {
                meldCombGrade[0] = meldCombGrade[2] - 1;
                meldCombGrade[2] = 0;
                if (_releventMateriaBaseParam.size() > 4) meldCombGrade[3]++;
            }
            else {
                meldCombGrade[0] = meldCombGrade[3] - 1;
                if (_releventMateriaBaseParam.size() > 4) meldCombGrade[3] = 0;
            }
        }
    }

    if (meldCombGrade[lastBaseParam] > maxMeldSlots.at(grade) && materiaGrades.size() > 1) {
        meldCombGrade[0] = maxMeldSlots.at(grade);
        meldCombGrade[lastBaseParam] = 0;

        grade = materiaGrades[1];
        auto& meldCombGrade = meldComb[grade];
        if (meldCombGrade[secondToLastBaseParam] + meldCombGrade[lastBaseParam] == maxMeldSlots.at(grade)) meldCombGrade[lastBaseParam]++;
        if (meldCombGrade[0] > 0) {
            meldCombGrade[0]--;
            meldCombGrade[1]++;
        }
        else {
            if (meldCombGrade[1] > 0) {
                meldCombGrade[0] = meldCombGrade[1] - 1;
                meldCombGrade[1] = 0;
                meldCombGrade[2]++;
            }
            else {
                if (meldCombGrade[2] > 0) {
                    meldCombGrade[0] = meldCombGrade[2] - 1;
                    meldCombGrade[2] = 0;
                    if (_releventMateriaBaseParam.size() > 4) meldCombGrade[3]++;
                }
                else {
                    meldCombGrade[0] = meldCombGrade[3] - 1;
                    if (_releventMateriaBaseParam.size() > 4) meldCombGrade[3] = 0;
                }
            }
        }
    }
}