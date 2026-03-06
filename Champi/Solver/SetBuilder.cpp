#include "SetBuilder.h"

SetBuilder SetBuilder::_instance = SetBuilder();

SetBuilder::SetBuilder()
{
    //ctor
}

SetBuilder::~SetBuilder()
{
    //dtor
}

void SetBuilder::startSolve(Job* job, int level, const map<int, vector<GearPiece*>>& gearPieces, const vector<Food*>& foodList, const vector<int>& releventMateriaBaseParam) {
    isSolving = true;
    solvingTime = 0;
    _equipSlots.clear();
    _gearPieceCounter.clear();
    _gearPieceIdx.clear();
    _gearPieceSlotToIdx.clear();
    results.clear();

    _solveThread = jthread([this, job, level, gearPieces, foodList, releventMateriaBaseParam](stop_token stopToken) {
        solve(stopToken, job, level, gearPieces, foodList, releventMateriaBaseParam);
    });
}

void SetBuilder::cancelSolve() {
    auto source = _solveThread.get_stop_source();
    if (source.stop_possible() && !source.stop_requested()) {
        _solveThread.request_stop();
        _solveThread.join();
    }
}

void SetBuilder::setBaseParamRanges(const map<int, bool>& limitBaseParam, const map<int, pair<int, int>>& baseParamRangeSelected) {
    _baseParamRangeSelected.clear();
    _currentGearBaseParamRange.clear();
    _foodBaseParamRange.clear();
    for (auto& [baseParam, limited] : limitBaseParam) {
        if (!limited) continue;
        _baseParamRangeSelected[baseParam] = baseParamRangeSelected.at(baseParam);
    }
}

void SetBuilder::solve(stop_token stopToken, Job* job, int level, const map<int, vector<GearPiece*>>& gearPieces, const vector<Food*>& foodList, const vector<int>& releventMateriaBaseParam) {
    auto startTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

    activeThreads = 1;
    maxHardwareThreads = Settings::Instance().maxParallelWorkers;

    // Counters to track progress and piece switches
    _maxCounter = 1;
    _ringPerms = gearPieces.at(12).size() * (gearPieces.at(12).size() - 1) / 2
        + count_if(gearPieces.at(12).begin(), gearPieces.at(12).end(), [](auto g) { return !g->isUnique; });
    for (int slot = 1; slot < 13; slot++) {
        if (!gearPieces.contains(slot) || gearPieces.at(slot).size() == 0) continue;

        if (slot == 12) {
            _maxCounter *= _ringPerms;
        } else {
            _maxCounter *= gearPieces.at(slot).size();
        }
        _equipSlots.push_back(slot);
    }
    _switchCounter = 0;

    // Base meld solver that will be copied for each thread
    auto gearPiecesToSolve = initGear(gearPieces);
    MeldSolver defaultMeldSolver(job, gearPiecesToSolve, foodList, releventMateriaBaseParam, &activeThreads, stopToken);
    defaultMeldSolver.setBaseParamRanges(_baseParamRangeSelected);

    // Set stat ranges to check against user selected range
    // They are updated when switching a piece to avoid rebuilding the whole thing every time
    for (auto& [baseParam, _] : _baseParamRangeSelected) {
        auto& currentGearRange = _currentGearBaseParamRange[baseParam];
        currentGearRange.first = Damage::Instance().getStartingValue(baseParam);
        currentGearRange.second = Damage::Instance().getStartingValue(baseParam);
        for (auto piece : defaultMeldSolver.gearPieces) {
            currentGearRange.first += piece->minBaseParamValue[baseParam];
            currentGearRange.second += piece->maxBaseParamValue[baseParam];
        }
        updateFoodBaseParamRanges(foodList);
    }

    /* Main loop
    * The order or operations is:
    * - (Prepare a thread then launch it) * number of available threads
    * - Keep preparing threads until we have X ready, X being arbitrarily set to the number of available threads
    * - Poll until a thread is done, update visual progress
    *   Polling is done so there's no synchronization/locking mechanism slowing down any of the threads
    * - Keep launching and preparing threads whenever there is room
    * - While waiting for threads to finish, save results one by one so we can launch a new thread as soon as possible
    */
    map<size_t, jthread> threads;
    map<size_t, MeldSolver> meldSolvers;
    deque<size_t> meldSolversReadyIdx;
    set<size_t> meldSolversToSave;
    int64_t resultMeldSolverIdx = -1;
    map<int, GearSet>::iterator resultSetIt;
    while (!stopToken.stop_requested() && (_gearPieceCounter[12] < _ringPerms || meldSolversToSave.size() > 0)) {
        if (activeThreads < maxHardwareThreads && meldSolversReadyIdx.size() > 0) {
            // Launch thread
            threads.emplace(piecewise_construct, forward_as_tuple(meldSolversReadyIdx.front()), forward_as_tuple(&MeldSolver::findBestMelds, &(meldSolvers[meldSolversReadyIdx.front()])));
            meldSolversReadyIdx.pop_front();
            activeThreads++;

        } else if (meldSolversReadyIdx.size() < maxHardwareThreads && _gearPieceCounter[12] < _ringPerms) {
            // Prepare thread
            
            // Check we are in the selected stat range before solving melds for this gear pieces combination
            bool isInRange = true;
            for (auto& [baseParam, selectedRange] : _baseParamRangeSelected) {
                // Check if there is any intersection between selectedRange and currentGearRange + foodRange
                auto& currentGearRange = _currentGearBaseParamRange[baseParam];
                auto& foodRange = _foodBaseParamRange[baseParam];
                if (currentGearRange.first + foodRange.first > selectedRange.second || currentGearRange.second + foodRange.second < selectedRange.first) {
                    isInRange = false;
                    break;
                }
            }

            if (isInRange) {
                meldSolvers.emplace(_switchCounter, defaultMeldSolver);
                meldSolversReadyIdx.push_back(_switchCounter);
                meldSolversToSave.insert(_switchCounter);
            }

            // Switch gear
            switchGear(gearPieces, defaultMeldSolver.gearPieces);
            updateFoodBaseParamRanges(foodList);
            _switchCounter++;

        } else if (resultMeldSolverIdx >= 0) {
            // Compare 1 result and save (+ delete when done)
            while (resultSetIt != meldSolvers[resultMeldSolverIdx].results.end()) {
                auto [savedResult, emplaced] = results.try_emplace(resultSetIt->first, resultSetIt->second);
                if (!emplaced && resultSetIt->second.damageMod > savedResult->second.damageMod) {
                    savedResult->second = resultSetIt->second;
                    emplaced = true;
                }
                resultSetIt++;
                if (!emplaced) break;
            }
            if (resultSetIt == meldSolvers[resultMeldSolverIdx].results.end()) {
                meldSolversToSave.erase(resultMeldSolverIdx);
                meldSolvers.erase(resultMeldSolverIdx);
                threads.erase(resultMeldSolverIdx);
                resultMeldSolverIdx = -1;
            }

        } else {
            // Go through meldSolvers
            float partialProgress = 0.0f;
            for (auto idx : meldSolversToSave) {
                auto& meldSolver = meldSolvers[idx];
                if (meldSolver.done) {
                    resultMeldSolverIdx = static_cast<int64_t>(idx); // resultMeldSolverIdx can be -1 but idx is unsigned
                    resultSetIt = meldSolver.results.begin();
                } else {
                    // Given the exponential nature of the solving algorithm a simple currentCount / maxCounter gives an estimation off by orders of magnitude
                    // This formula gives an estimation in the same order of magnitude and is meant to overestimate a little
                    float solvingProgress = 0.0f;
                    if (meldSolver.currentCount > 0) {
                        auto ratio = meldSolver.currentCount / meldSolver.maxCounter;
                        solvingProgress = powf(ratio, 0.25f - 0.07f * powf(ratio, 0.2f)) * logf(meldSolver.currentCount) / meldSolver.logMaxCounter;
                    }
                    // This is calculated backwards because _switchCounter is updated when creating the meld solver
                    // So until it is done we need to remove the progress it hasn't done yet
                    partialProgress += solvingProgress - 1;
                }
            }

            // Update progress for visual feedback
            solvingProgress = (_switchCounter + partialProgress) / _maxCounter;
            auto currentTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            elapsed = currentTime - startTime;
            estimatedRemaining = (int64_t)((1 / solvingProgress - 1) * elapsed);

            // If no result to check, sleep
            if (resultMeldSolverIdx < 0) {
                this_thread::sleep_for(milliseconds(1));
            }
        }
    }
    if (!stopToken.stop_requested()) {
        solvingTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() - startTime;
    } else {
        // jthreads are joined automatically but random errors are sometimes happening when stop is requested
        // It could be due to the meldSolvers being destructed before threads when exiting this method, causing memory access errors
        // So let's join all threads to make sure they're not trying to read memory when we exit
        for (auto& [_, t] : threads) {
            if (t.joinable()) t.join();
        }
    }
    activeThreads--;
    isSolving = false;
    solvingProgress = 0.0f;
    elapsed = 0;
    estimatedRemaining = 0;
}

vector<GearPiece*> SetBuilder::initGear(const map<int, vector<GearPiece*>>& gearPieces) {
    vector<GearPiece*> result;
    for (int slot : _equipSlots) {
        auto piece = gearPieces.at(slot).at(0);
        result.push_back(piece);
        _gearPieceIdx.push_back(0);
        _gearPieceSlotToIdx[slot] = result.size() - 1;
        _gearPieceCounter[slot] = 0;
        if (slot == 12) {
            int secondRingIdx = piece->isUnique ? 1 : 0;
            result.push_back(gearPieces.at(slot).at(secondRingIdx));
            _gearPieceIdx.push_back(secondRingIdx);
        }
    }
    return result;
}

void SetBuilder::switchGear(const map<int, vector<GearPiece*>>& gearPieces, vector<GearPiece*>& gearPiecesToSolve) {
    for (int slot : _equipSlots) {
        _gearPieceCounter[slot]++;
        // Until 11, compare to size and reset
        // At 12, compare to ringPerms and don't reset to hit main loop break condition
        if (_gearPieceCounter[slot] >= (slot >= 12 ? _ringPerms : gearPieces.at(slot).size())) {
            if (slot < 12) {
                _gearPieceCounter[slot] = 0;
            }
        } else {
            switchPiece(gearPieces, gearPiecesToSolve, slot);
            return;
        }
    }
}

void SetBuilder::switchPiece(const map<int, vector<GearPiece*>>& gearPieces, vector<GearPiece*>& gearPiecesToSolve, int slot) {
    auto pieceIdx = _gearPieceSlotToIdx[slot];
    auto& slotPieces = gearPieces.at(slot);

    removePieceRanges(gearPiecesToSolve[pieceIdx]);

    if (slot < 12) {
        _gearPieceIdx[pieceIdx] = (_gearPieceIdx[pieceIdx] + 1) % slotPieces.size();
        gearPiecesToSolve[pieceIdx] = slotPieces.at(_gearPieceIdx[pieceIdx]);
    } else {
        removePieceRanges(gearPiecesToSolve[pieceIdx + 1]);

        _gearPieceIdx[pieceIdx + 1]++;
        if (_gearPieceIdx[pieceIdx + 1] == slotPieces.size()) {
            _gearPieceIdx[pieceIdx]++;
            _gearPieceIdx[pieceIdx + 1] = _gearPieceIdx[pieceIdx] + (slotPieces.at(_gearPieceIdx[pieceIdx])->isUnique ? 1 : 0);
        }
        gearPiecesToSolve[pieceIdx] = slotPieces.at(_gearPieceIdx[pieceIdx]);
        gearPiecesToSolve[pieceIdx + 1] = slotPieces.at(_gearPieceIdx[pieceIdx + 1]);

        addPieceRanges(gearPiecesToSolve[pieceIdx + 1]);
    }

    addPieceRanges(gearPiecesToSolve[pieceIdx]);
}

void SetBuilder::removePieceRanges(GearPiece* piece) {
    for (auto& [baseParam, currentGearRange] : _currentGearBaseParamRange) {
        currentGearRange.first -= piece->minBaseParamValue[baseParam];
        currentGearRange.second -= piece->maxBaseParamValue[baseParam];
    }
}

void SetBuilder::addPieceRanges(GearPiece* piece) {
    for (auto& [baseParam, currentGearRange] : _currentGearBaseParamRange) {
        currentGearRange.first += piece->minBaseParamValue[baseParam];
        currentGearRange.second += piece->maxBaseParamValue[baseParam];
    }
}

void SetBuilder::updateFoodBaseParamRanges(const vector<Food*>& foodList) {
    for (auto& [baseParam, currentGearRange] : _currentGearBaseParamRange) {
        auto& foodRange = _foodBaseParamRange[baseParam];
        foodRange.first = foodList.size() > 0 ? INT32_MAX : 0;
        foodRange.second = 0;
        for (auto food : foodList) {
            pair<int, int> foodValue = make_pair(0, 0);
            for (int i = 0; i < 3; i++) {
                if (food->baseParam[i] != baseParam) continue;
                foodValue.first = min((int)(currentGearRange.first * 0.01f * food->valueHQ[i]), food->maxHQ[i]);
                foodValue.second = min((int)(currentGearRange.second * 0.01f * food->valueHQ[i]), food->maxHQ[i]);
            }
            if (foodValue.first < foodRange.first) {
                foodRange.first = foodValue.first;
            }
            if (foodValue.second > foodRange.second) {
                foodRange.second = foodValue.second;
            }
        }
    }
}