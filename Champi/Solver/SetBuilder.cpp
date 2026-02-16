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

void SetBuilder::solve(stop_token stopToken, Job* job, int level, const map<int, vector<GearPiece*>>& gearPieces, const vector<Food*>& foodList, const vector<int>& releventMateriaBaseParam) {
    auto startTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

    auto maxHardwareThreads = max(Settings::Instance().maxParallelWorkers - 1, 1);
    Damage::Instance().init(job, level);
    for (auto& it : gearPieces) {
        for (auto& gearPiece : it.second) {
            gearPiece->setMeldPerms(releventMateriaBaseParam, Data::Instance().materiaList);
        }
    }

    _maxCounter = 1;
    _ringPerms = gearPieces.at(12).size() * (gearPieces.at(12).size() - 1) / 2
        + count_if(gearPieces.at(12).begin(), gearPieces.at(12).end(), [](auto g) { return !g->isUnique; });
    for (int slot = 1; slot < 13; slot++) {
        if (!gearPieces.contains(slot) || gearPieces.at(slot).size() == 0) continue;

        if (slot == 12) {
            _maxCounter *= _ringPerms;
        }
        else {
            _maxCounter *= gearPieces.at(slot).size();
        }
        _equipSlots.push_back(slot);
    }
    _switchCounter = 0;
    atomic_int activeThreads = 0;
    auto gearPiecesToSolve = initGear(gearPieces);
    deque<size_t> meldSolversReadyIdx;
    set<size_t> meldSolversToSave;
    MeldSolver defaultMeldSolver(job, gearPiecesToSolve, foodList, releventMateriaBaseParam, &activeThreads, stopToken);
    map<size_t, MeldSolver> meldSolvers;
    map<size_t, jthread> threads;
    //int foodMaxSks = 0;
    //for (unsigned int i = 0; i < m_foods.size(); i++) {
    //    if (m_foods[i].cap[3] > foodMaxSks)
    //        foodMaxSks = m_foods[i].cap[3];
    //}
    int64_t resultMeldSolverIdx = -1;
    map<int, GearSet>::iterator resultSetIt;
    while (!stopToken.stop_requested() && (_gearPieceCounter[12] < _ringPerms || meldSolversToSave.size() > 0)) {
        if (activeThreads < maxHardwareThreads && meldSolversReadyIdx.size() > 0) {
            threads.emplace(piecewise_construct, forward_as_tuple(meldSolversReadyIdx.front()), forward_as_tuple(&MeldSolver::findBestMelds, &(meldSolvers[meldSolversReadyIdx.front()])));
            meldSolversReadyIdx.pop_front();
            activeThreads++;
        } else if (meldSolversReadyIdx.size() < maxHardwareThreads && _gearPieceCounter[12] < _ringPerms) {
            // TODO: Rework for pentamelds, probably in FullSet+GearPiece
        //int sksSlotsAvailable = 0;
        //for (int i = 0; i < 12; i++) {
        //    if (i == 4) continue;
        //    GearPieceOld* piece = m_full_set.gear_set[i];
        //    // TODO: rework sksSlotsAvailable into sksAvailable
        //    if (piece->subStat == 3)
        //        sksSlotsAvailable = sksSlotsAvailable + round(1.0 * min(piece->subs[piece->mainStat] - piece->subs[piece->subStat], piece->meldSlots * MAT) / MAT);
        //    else if (piece->thirdStat == 3 || piece->fourthStat == 3)
        //        sksSlotsAvailable = sksSlotsAvailable + piece->meldSlots;
        //}

        //if (!m_full_set.gear_set[10]->name.compare("Crafted") || m_full_set.gear_set[10]->name.compare(m_full_set.gear_set[11]->name)) {
            //if (m_full_set.gearStats.subs[3] <= MAX_SKS && m_full_set.gearStats.subs[3] + sksSlotsAvailable * MAT + foodMaxSks >= MIN_SKS) {
            meldSolvers.emplace(_switchCounter, defaultMeldSolver);
            meldSolversReadyIdx.push_back(_switchCounter);
            meldSolversToSave.insert(_switchCounter);
            //}
        //}
            // Switch gear
            switchGear(gearPieces, defaultMeldSolver.gearPieces);
            _switchCounter++;
        } else if (resultMeldSolverIdx >= 0) {
            // Compare 1 result and save (+ delete when done)
            while (resultSetIt != meldSolvers[resultMeldSolverIdx].results.end() && results.contains(resultSetIt->first) && results[resultSetIt->first].damageMod >= resultSetIt->second.damageMod)
                resultSetIt++;
            if (resultSetIt != meldSolvers[resultMeldSolverIdx].results.end()) {
                results[resultSetIt->first] = resultSetIt->second;
                resultSetIt++;
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
                if (meldSolvers[idx].done) {
                    resultMeldSolverIdx = static_cast<int64_t>(idx);
                    resultSetIt = meldSolvers[idx].results.begin();
                    //break;
                } else {
                    partialProgress += meldSolvers[idx].solvingProgress - 1;
                }
            }
            solvingProgress = (_switchCounter + partialProgress) / _maxCounter;
            auto currentTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            auto elapsed = currentTime - startTime;
            estimatedRemaining = (int64_t)((1 / solvingProgress - 1) * elapsed);
            // If no result to check, sleep
            if (resultMeldSolverIdx < 0) {
                this_thread::sleep_for(milliseconds(1));
            }
        }
    }
    if (!stopToken.stop_requested()) {
        solvingTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() - startTime;
    }
    isSolving = false;
    solvingProgress = 0.0f;
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
        // At 12, compare to ringPerms and don't reset
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
    if (slot < 12) {
        _gearPieceIdx[pieceIdx] = (_gearPieceIdx[pieceIdx] + 1) % slotPieces.size();
        gearPiecesToSolve[pieceIdx] = slotPieces.at(_gearPieceIdx[pieceIdx]);
    } else {
        _gearPieceIdx[pieceIdx + 1]++;
        if (_gearPieceIdx[pieceIdx + 1] == slotPieces.size()) {
            _gearPieceIdx[pieceIdx]++;
            _gearPieceIdx[pieceIdx + 1] = _gearPieceIdx[pieceIdx] + (slotPieces.at(_gearPieceIdx[pieceIdx])->isUnique ? 1 : 0);
        }
        gearPiecesToSolve[pieceIdx] = slotPieces.at(_gearPieceIdx[pieceIdx]);
        gearPiecesToSolve[pieceIdx + 1] = slotPieces.at(_gearPieceIdx[pieceIdx + 1]);
    }
}