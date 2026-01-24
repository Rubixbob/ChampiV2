#include "MeldFinder.h"

#define MAT 54
#define MIN_SKS 420
#define MAX_SKS 5000
// TODO: rework into gear
//int meldSlotsTable[11] = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
//int meldsPermutationsTable[11];

MeldFinder::MeldFinder():done(false)
{
    //ctor
    for (int i = 0; i < 100; i++) {
        m_saved_sets[i] = nullptr;
    }
}

MeldFinder::~MeldFinder()
{
    //dtor
//    cout << "Meldfinder destructor called" << endl;
    delete m_full_set;
    for (int i = 0; i < 100; i++) {
        if (m_saved_sets[i])
            delete m_saved_sets[i];
    }
}

void MeldFinder::findBestMelds()
{
//    char a;
    //cout << "In findBestMelds" << endl;
//    cout << "Done " << done << endl;
//    cin.get(a);
//    m_full_set->displaySet(cout);
//    cin.get(a);
//    done = true;
//    return;

//    sf::Clock m_clock;
//    m_clock.restart();
    int maxMeldSlots = 0;
    for (int i = 0; i < 12; i++) {
        if (i == 4) continue;
        maxMeldSlots += m_full_set->gear_set[i]->meldSlots;
    }
//    unsigned int maxCombinations = (maxMeldSlots+1)*(maxMeldSlots+2)*(maxMeldSlots+3)/6;
    //

    statStruct gearStats;
    gearStats.wd = 0;
    gearStats.str = baseMainStat;
    gearStats.subs[0] = 420;
    gearStats.subs[1] = 420;
    gearStats.subs[2] = 440;
    gearStats.subs[3] = 420;
    for (int i = 0; i < 12; i++) {
        if (i == 4) continue;
        GearPiece* piece = m_full_set->gear_set[i];

        gearStats.wd += piece->wd;
        gearStats.str += piece->str;
        for (int k = 0; k < 4; k++) {
            gearStats.subs[k] += piece->subs[k];
        }
    }
//    cout << gearStats.wd << " " << gearStats.str << " " << gearStats.subs[0] << " " << gearStats.subs[1] << " " << gearStats.subs[2] << " " << gearStats.subs[3] << endl;

    // Meld combinations init
    int meldComb[4];
    int meldCombOld[4];
    meldComb[3] = maxMeldSlots;
    for (int i = 0; i < 3; i++) {
        meldComb[i] = 0;
    }
    for (int i = 0; i < 4; i++) {
        meldCombOld[i] = meldComb[i];
    }

    // Materia slots init
    int availableSlots[4];
    int availableSlotsInit[4];
    int neededSlots[4];
//    unsigned int meldStatPermutation[12][9][4];
//    unsigned int matStat[12][9][4];
//    for (int i = 0; i < 12; i++) {
//        for (int j = 0; j < 9; j++) {
//            for (int k = 0; k < 4; k++) {
//                matStat[i][j][k] = 0;
//            }
//        }
//    }
    for (int i = 0; i < 4; i++) {
        availableSlotsInit[i] = 0;
    }
    for (int i = 0; i < 12; i++) {
        if (i == 4) continue;
            // TODO: check submeldtable change
        GearPiece* piece = m_full_set->gear_set[i];
        availableSlotsInit[piece->subStat] += round(1.0 * min(piece->subs[piece->mainStat] - piece->subs[piece->subStat], piece->meldSlots * MAT) / MAT);
//        cout << "type " << piece->type << " sub " << subMeldTable[piece->type] << " slots " << round(1.0 * min(subs[mainStat] - subs[subStat], meldSlots * MAT) / MAT) << endl;
        availableSlotsInit[piece->thirdStat] += piece->meldSlots;
        availableSlotsInit[piece->fourthStat] += piece->meldSlots;
    }
//    cout << "dh " << availableSlotsInit[0] << " crit " << availableSlotsInit[1] << " det " << availableSlotsInit[2] << " sks " << availableSlotsInit[3] << endl;
//    for (int i = 0; i < 12; i++) {
//        for (int j = 0; j < 9; j++) {
//            for (int k = 0; k < 4; k++) {
//                cout << setw(3) << matStat[i][j][k] << " ";
//            }
//            cout << "    ";
//        }
//        cout << endl;
//    }

    // Score init
    int maxScore = maxMeldSlots * MAT;
//    unsigned int currentScore[4];
//    for (int i = 0; i < 4; i++) {
//        currentScore[i] = 0;
//    }

    vector<vector<vector<vector<int>>>> savedScores(maxMeldSlots + 1);
    for (int i = 0; i <= maxMeldSlots; i++) {
		savedScores[i] = vector<vector<vector<int>>>(maxMeldSlots + 1);
        for (int j = 0; j <= maxMeldSlots; j++) {
			savedScores[i][j] = vector<vector<int>>(maxMeldSlots + 1);
            for (int k = 0; k <= maxMeldSlots; k++) {
				savedScores[i][j][k] = vector<int>(maxMeldSlots + 1);
                for (int l = 0; l <= maxMeldSlots; l++) {
                    savedScores[i][j][k][l] = -1;
                }
            }
        }
    }
//    int gearMeldTable[12][5];

//    meldComb[0] = 2;
//    meldComb[1] = 7;
//    meldComb[2] = 8;
//    meldComb[3] = 0;

    while (meldComb[0] <= maxMeldSlots) {
        bool possible = true;
//        cout << setw(2) << meldComb[0] << " " << setw(2) << meldComb[1] << " " << setw(2) << meldComb[2] << " " << setw(2) << meldComb[3] << endl;
        // Init
        for (int i = 0; i < 4; i++) {
            neededSlots[i] = meldComb[i];
            availableSlots[i] = availableSlotsInit[i];
            if (neededSlots[i] > availableSlots[i])
                possible = false;
        }
//        cout << "possible " << possible << endl;

//        cin.get(a);

        if (possible) {
            // Check DPS with perfect melds
            statStruct maxMeldedGearStat;
            maxMeldedGearStat.wd = gearStats.wd;
            maxMeldedGearStat.str = gearStats.str;
            for (int i = 0; i < 4; i++) {
                maxMeldedGearStat.subs[i] = gearStats.subs[i] + meldComb[i] * MAT;
            }
//            cout << maxMeldedGearStat.wd << " " << maxMeldedGearStat.str << " " << maxMeldedGearStat.subs[0] << " " << maxMeldedGearStat.subs[1] << " " << maxMeldedGearStat.subs[2] << " " << maxMeldedGearStat.subs[3] << endl;

            possible = false;
            // Iterate on foods
            for (unsigned int j = 0; j < m_foods.size(); j++) {
                statStruct maxFoodMeldedGearStat;
                maxFoodMeldedGearStat.wd = maxMeldedGearStat.wd;
                // Party bonus
                maxFoodMeldedGearStat.str = maxMeldedGearStat.str * 1.05;
                for (int i = 0; i < 4; i++) {
                    maxFoodMeldedGearStat.subs[i] = maxMeldedGearStat.subs[i] + min(maxMeldedGearStat.subs[i] * m_foods[j]->mult[i], m_foods[j]->cap[i]);
                }
//                    cout << maxFoodMeldedGearStat.wd << " " << maxFoodMeldedGearStat.str << " " << maxFoodMeldedGearStat.subs[0] << " " << maxFoodMeldedGearStat.subs[1] << " " << maxFoodMeldedGearStat.subs[2] << " " << maxFoodMeldedGearStat.subs[3] << endl;
                // Compute dps and compare to saved sets
                if (maxFoodMeldedGearStat.subs[3] >= MIN_SKS && gearStats.subs[3] <= MAX_SKS) {
                    double dps = dirMult[maxFoodMeldedGearStat.subs[0]] * critMult[maxFoodMeldedGearStat.subs[1]] * detMult[maxFoodMeldedGearStat.subs[2]] * sksMult[maxFoodMeldedGearStat.subs[3]] * strMult[maxFoodMeldedGearStat.str];
                    unsigned int gcd = floor(floor(2.5 * (1000 - floor(130 * (maxFoodMeldedGearStat.subs[3] - 420) / 2780))) * (100 - haste) / 1000);
                    unsigned int minGcd = floor(floor(2.5 * (1000 - floor(130 * (gearStats.subs[3] - 420) / 2780))) * (100 - haste) / 1000);
                    if (gearStats.subs[3] < MIN_SKS)
                        minGcd = floor(floor(2.5 * (1000 - floor(130 * (MIN_SKS - 420) / 2780))) * (100 - haste) / 1000);
                    int idx = 250 - gcd;
                    int minIdx = 250 - minGcd;
                    // Check all saved dps within gcd range
                    for (int i = minIdx; i <= idx; i++) {
//                        mutex_saved_sets[i].lock();
                        if (!m_saved_sets[i] || dps > m_saved_sets[i]->dps)
                            possible = true;
//                        mutex_saved_sets[i].unlock();
                    }
                }
            }
        }

        if (possible) {
            // Find melds
            int currentPerm[12];
            int savedPerm[12];
            for (int i = 0; i < 12; i++) {
                currentPerm[i] = 0;
                savedPerm[i] = 0;
            }
            int idx = 0;
            int score = 0;
            int savedScore = 0;
            int scoreGoal = 0;
            bool scoreGiven = false;
            while (idx < 12 && idx >= 0) {
//                char a;
//                cin.get(a);
                if (idx == 4)
                    cout << "idx = 4, we shouldn't be there" << endl;
                GearPiece* piece = m_full_set->gear_set[idx];
                // Check if perm can be used
                bool useful = true;
                for (int i = 0; i < 4; i++) {
                    if(piece->meldStatPermutation[currentPerm[idx]][i] > neededSlots[i])
                        useful = false;
                }
//                cout << "useful " << useful << endl;
                if (useful) {
                    int currentSavedScore = savedScores[neededSlots[0]][neededSlots[1]][neededSlots[2]][neededSlots[3]];
                    int nextScoreGoal = currentSavedScore - piece->matStat[currentPerm[idx]][0]
                                                          - piece->matStat[currentPerm[idx]][1]
                                                          - piece->matStat[currentPerm[idx]][2]
                                                          - piece->matStat[currentPerm[idx]][3];
                    int nextSavedScore = savedScores[neededSlots[0] - piece->meldStatPermutation[currentPerm[idx]][0]]
                                                    [neededSlots[1] - piece->meldStatPermutation[currentPerm[idx]][1]]
                                                    [neededSlots[2] - piece->meldStatPermutation[currentPerm[idx]][2]]
                                                    [neededSlots[3] - piece->meldStatPermutation[currentPerm[idx]][3]];
//                    cout << "currentSavedScore " << currentSavedScore << " nextScoreGoal " << nextScoreGoal << " nextSavedScore " << nextSavedScore << endl;
//                    cout << "scoreGiven " << scoreGiven << " scoreGoal " << scoreGoal << endl;
                    if (scoreGiven) {
                        if (idx < 11 && nextSavedScore < nextScoreGoal)
                            useful = false;
                    } else {
                        if (nextSavedScore > 0) {
                            if (nextSavedScore > nextScoreGoal) {
                                scoreGiven = true;
                                scoreGoal = nextSavedScore;
                            } else
                                useful = false;
                        }
                        else if (nextSavedScore == 0)
                            useful = false;
                    }
                }
//                cout << "useful " << useful << " scoreGiven " << scoreGiven << " scoreGoal " << scoreGoal << endl;
//                if (idx <= 12) {
//                    cout << "idx " << idx << " perm " << currentPerm[idx] << " useful " << useful << endl;
//                    cout << neededSlots[0] << " " << neededSlots[1] << " " << neededSlots[2] << " " << neededSlots[3] << endl;
//                }

                // If useful, take the slots and go to next piece
                if (useful) {
                    for (int i = 0; i < 4; i++) {
                        availableSlots[i] -= piece->meldStatPermutation[currentPerm[idx]][i];
                        neededSlots[i] -= piece->meldStatPermutation[currentPerm[idx]][i];
                    }
                    idx++;
                    if (idx == 4) idx++;
                    piece = m_full_set->gear_set[idx];
                    if (idx >= 12) {
                        score = 0;
                        int path[4];
                        for (int i = 0; i < 4; i++) {
                            path[i] = 0;
                        }
                        bool saving = true;
                        for (int j = 11; j >= 0; j--) {
                            if (j == 4) continue;
                            // Save score on the path
                            for (int i = 0; i < 4; i++) {
                                score += m_full_set->gear_set[j]->matStat[currentPerm[j]][i];
                                path[i] += m_full_set->gear_set[j]->meldStatPermutation[currentPerm[j]][i];
                            }
//                            cout << "-------------------------------------" << endl;
//                            cout << "saving " << saving << " score " << score << " savedScores " << savedScores[path[0]][path[1]][path[2]][path[3]] << endl;
                            if (saving && (int)score < savedScores[path[0]][path[1]][path[2]][path[3]])
                                saving = false;
                            if (saving)
                                savedScores[path[0]][path[1]][path[2]][path[3]] = score;
                        }
                        if (score < maxScore) {
                            // Compare to saved score and save
                            if (score > savedScore) {
//                                cout << "Saving score" << endl;
                                for (int i = 0; i < 12; i++) {
                                    savedPerm[i] = currentPerm[i];
                                }
                                savedScore = score;
                            }
                            useful = false;
                            idx--;
                            piece = m_full_set->gear_set[idx];
                            for (int i = 0; i < 4; i++) {
                                availableSlots[i] += piece->meldStatPermutation[currentPerm[idx]][i];
                                neededSlots[i] += piece->meldStatPermutation[currentPerm[idx]][i];
                            }
                            if (scoreGiven)
                                currentPerm[idx] = piece->meldPermutations;
                        }
                    }
                }
                // If not, go to next perm or up
                if (!useful) {
                    currentPerm[idx]++;
                    while (idx >= 0 && (currentPerm[idx] >= piece->meldPermutations)) {
                        currentPerm[idx] = 0;
                        if (savedScores[neededSlots[0]][neededSlots[1]][neededSlots[2]][neededSlots[3]] < 0)
                            savedScores[neededSlots[0]][neededSlots[1]][neededSlots[2]][neededSlots[3]] = 0;
                        else if (savedScores[neededSlots[0]][neededSlots[1]][neededSlots[2]][neededSlots[3]] == scoreGoal) {
                            scoreGoal = 0;
                            scoreGiven = false;
                        }
                        idx--;
                        if (idx == 4) idx--;
                        piece = m_full_set->gear_set[idx];
                        if (idx >= 0) {
                            // Give back slots and go to next perm
                            for (int i = 0; i < 4; i++) {
                                availableSlots[i] += piece->meldStatPermutation[currentPerm[idx]][i];
                                neededSlots[i] += piece->meldStatPermutation[currentPerm[idx]][i];
                            }
                            // If score is given and we're here, score has been found and we're going up
                            if (scoreGiven)
                                currentPerm[idx] = piece->meldPermutations;
                            else
                                currentPerm[idx]++;
                        }
                    }
                }

            }

            if (score == maxScore) {
                for (int i = 0; i < 12; i++) {
                    savedPerm[i] = currentPerm[i];
                }
                savedScore = score;
            }

            // Create set + melds
            if (savedScore > 0) {
                for (int i = 0; i < 12; i++) {
                    if (i == 4) continue;
                    for (int j = 0; j < 5; j++) {
                        m_full_set->gearMeldTable[i][j] = -1;
                    }
                }
                for (int j = 0; j < 12; j++) {
                    if (j == 4) continue;
                    for (int k = 0; k < m_full_set->gear_set[j]->meldSlots; k++) {
                        int meldCpt = 0;
                        for (int i = 0; i < 4; i++) {
                            if (m_full_set->gearMeldTable[j][k] < 0 && m_full_set->gear_set[j]->meldStatPermutation[savedPerm[j]][i] > 0) {
                                meldCpt += m_full_set->gear_set[j]->meldStatPermutation[savedPerm[j]][i];
                                if (meldCpt > k)
                                    m_full_set->gearMeldTable[j][k] = i;
                            }
                        }
                    }
                }

                statStruct meldedGearStat;
                meldedGearStat.wd = gearStats.wd;
                meldedGearStat.str = gearStats.str;
                for (int i = 0; i < 4; i++) {
                    meldedGearStat.subs[i] = gearStats.subs[i];
                }
                for (int j = 0; j < 12; j++) {
                    if (j == 4) continue;
                    for (int i = 0; i < 4; i++) {
                        meldedGearStat.subs[i] += m_full_set->gear_set[j]->matStat[savedPerm[j]][i];
                    }
                }
//                cout << meldedGearStat.wd << " " << meldedGearStat.str << " " << meldedGearStat.subs[0] << " " << meldedGearStat.subs[1] << " " << meldedGearStat.subs[2] << " " << meldedGearStat.subs[3] << endl;

                // Iterate on foods
                for (unsigned int j = 0; j < m_foods.size(); j++) {
                    statStruct foodMeldedGearStat;
                    foodMeldedGearStat.wd = meldedGearStat.wd;
                    // Party bonus
                    foodMeldedGearStat.str = meldedGearStat.str * 1.05;
                    for (int i = 0; i < 4; i++) {
                        foodMeldedGearStat.subs[i] = meldedGearStat.subs[i] + min(meldedGearStat.subs[i] * m_foods[j]->mult[i], m_foods[j]->cap[i]);
                    }
//                    cout << foodMeldedGearStat.wd << " " << foodMeldedGearStat.str << " " << foodMeldedGearStat.subs[0] << " " << foodMeldedGearStat.subs[1] << " " << foodMeldedGearStat.subs[2] << " " << foodMeldedGearStat.subs[3] << endl;
                    // Compute dps and compare to saved sets
                    if (foodMeldedGearStat.subs[3] >= MIN_SKS && foodMeldedGearStat.subs[3] <= MAX_SKS) {
                        double dps = dirMult[foodMeldedGearStat.subs[0]] * critMult[foodMeldedGearStat.subs[1]] * detMult[foodMeldedGearStat.subs[2]] * sksMult[foodMeldedGearStat.subs[3]] * strMult[foodMeldedGearStat.str];
                        unsigned int gcd = floor(floor(2.5 * (1000 - floor(130 * (foodMeldedGearStat.subs[3] - 420) / 2780))) * (100 - haste) / 1000);
                        int idx = 250 - gcd;
//                        mutex_saved_sets[idx].lock();
                        if (!m_saved_sets[idx] || dps > m_saved_sets[idx]->dps) {
                            // Create set
//                            for (int i = 0; i < 12; i++) {
//                                GearPiece* piece = m_full_set->gear_set[i];
//                                if (piece) {
//                                    piece->melds[0] = m_full_set->gearMeldTable[i][0];
//                                    if (piece->meldSlots >= 2)
//                                        piece->melds[1] = m_full_set->gearMeldTable[i][1];
//                                    if (piece->meldSlots >= 3)
//                                        piece->melds[2] = m_full_set->gearMeldTable[i][2];
//                                }
//                            }
                            m_full_set->food = m_foods[j];
                            m_full_set->foodMeldedGearStats.wd = foodMeldedGearStat.wd;
                            m_full_set->meldedGearStats.str = meldedGearStat.str;
                            m_full_set->foodMeldedGearStats.str = foodMeldedGearStat.str;
                            for (int i = 0; i < 4; i++) {
                                m_full_set->meldedGearStats.subs[i] = meldedGearStat.subs[i];
                                m_full_set->foodMeldedGearStats.subs[i] = foodMeldedGearStat.subs[i];
                            }
                            m_full_set->dps = dps;
                            m_full_set->gcd = gcd;

//                            copyFullSet(m_full_set, &m_saved_sets[idx]);
                            // TODO: looks bad
                            if (m_saved_sets[idx]) {
                                delete m_saved_sets[idx];
                            }
                            m_saved_sets[idx] = new FullSet(*m_full_set);
                        }
//                        mutex_saved_sets[idx].unlock();
                    }
                }
//                cin.get(a);
            }

//            if (savedScore == 0) {
//                cout << "No melds found" << endl;
//            } else {
//                cout << "Combination found, score: " << savedScore << ", max score: " << maxScore << endl;
//                for (int i = 0; i < 12; i++) {
//                    cout << savedPerm[i] << " ";
//                }
//                cout << endl;
//                for (int i = 0; i < 12; i++) {
//                    for (int k = 0; k < 5; k++) {
//                        cout << gearMeldTable[i][k] << " ";
//                    }
//                    cout << endl;
//                }
//            }
//            cin.get(a);
        }

        meldComb[0] = (meldCombOld[0] + meldCombOld[1] == maxMeldSlots) ? (meldCombOld[0] + 1) : meldCombOld[0];
        meldComb[1] = (meldComb[0] == meldCombOld[0]) ? ( meldCombOld[3] == 0 ? meldCombOld[1] + 1 : meldCombOld[1] ) : 0;
        meldComb[2] = (meldComb[1] == meldCombOld[1]) ? meldCombOld[2] + 1 : 0;
        meldComb[3] = maxMeldSlots - meldComb[0] - meldComb[1] - meldComb[2];
        for (int i = 0; i < 4; i++) {
            meldCombOld[i] = meldComb[i];
        }
    }

//    cout << "done in " << m_clock.getElapsedTime().asMilliseconds() << "ms" << endl;
//    for (int i = 0; i < 100; i++) {
//        m_saved_sets[i].displayStats(cout);
//        m_saved_sets[i].displaySet(cout);
//    }
//    cout << "blah" << endl;
//    cin.get(a);


//    for (int i = 0; i < 12; i++) {
//        delete m_full_set.gear_set[i];
//    }
//    delete m_full_set_p;


//    mutex_counter.lock();
//    int oldCount = m_counter;
//    m_counter++;
//    mutex_counter.unlock();
//    if ((int) (oldCount*100.0/max_counter) != (int) (m_counter*100.0/max_counter)) {
//        sf::Time elapsed = m_clock.getElapsedTime();
//        sf::Time remaining = sf::seconds(max_counter * elapsed.asSeconds() / (m_counter + 1) - elapsed.asSeconds());
//        mutex_display.lock();
//    //    cout << "cpt " << cpt << ",usefulCpt " << usefulCpt << endl;
//        cout << (int) (m_counter*100.0/max_counter) << "% ";
//        cout << "Elapsed time: " << (int) (elapsed.asSeconds() / 60) << "m" << setw(2) << (int) elapsed.asSeconds() % 60 << "s" << setw(3) << elapsed.asMilliseconds() % 1000 << " ";
//        cout << "Estimated remaining: " << (int) (remaining.asSeconds() / 60) << "m" << setw(2) << (int) remaining.asSeconds() % 60 << "s" << setw(3) << remaining.asMilliseconds() % 1000;
//        cout << endl;
//        mutex_display.unlock();
//    }


    mutex_active_threads.lock();
    active_threads--;
    mutex_active_threads.unlock();
    done = true;
}
