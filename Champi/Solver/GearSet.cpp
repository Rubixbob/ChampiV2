#include "GearSet.h"

GearSet::GearSet()
{
    //ctor
}

GearSet::GearSet(const Job* job, const vector<GearPiece*>& gearPieces)
{
    selectedJob = job;
    mainBaseParamValue = Damage::Instance().getLvlModMain() * job->primaryStatMod / 100;
    meldedBaseParamValue[22] = Damage::Instance().getLvlModSub(); // DH
    meldedBaseParamValue[27] = Damage::Instance().getLvlModSub(); // CRIT
    meldedBaseParamValue[44] = Damage::Instance().getLvlModMain(); // DET
    int ssBaseParam = job->primaryStat == 4 || job->primaryStat == 5 ? 46 : 45;
    meldedBaseParamValue[ssBaseParam] = Damage::Instance().getLvlModSub(); // SS
    if (job->role == 1) meldedBaseParamValue[19] = Damage::Instance().getLvlModSub(); // TNC
    if (job->role == 4) meldedBaseParamValue[6] = Damage::Instance().getLvlModSub(); // PIE

    for (auto piece : gearPieces) {
        damagePhys += piece->damagePhys;
        damageMag += piece->damageMag;
        delayms += piece->delayms;
        for (int i = 0; i < 6; i++) {
            if (piece->baseParam[i] == job->primaryStat) {
                mainBaseParamValue += piece->baseParamValue[i];
            } else {
                auto meldedBaseParamIt = meldedBaseParamValue.find(piece->baseParam[i]);
                if (meldedBaseParamIt == meldedBaseParamValue.end()) continue;
                meldedBaseParamIt->second += piece->baseParamValue[i];
            }
        }
    }

    // Party bonus
    mainBaseParamValue = (int)(mainBaseParamValue * 1.05f);
}

GearSet::~GearSet()
{
    //dtor
}

void GearSet::addMeldPerm(MeldPerm* meldPerm) {
    meldPerms.push_back(meldPerm);

    for (auto& baseParamIt : meldPerm->baseParamMatValue) {
        meldedBaseParamValue[baseParamIt.first] += baseParamIt.second;
    }
}

void GearSet::popMeldPerm() {
    auto meldPerm = meldPerms.back();
    meldPerms.pop_back();

    for (auto& baseParamIt : meldPerm->baseParamMatValue) {
        meldedBaseParamValue[baseParamIt.first] -= baseParamIt.second;
    }
}

void GearSet::addFood(Food* food) {
    selectedFood = food;

    for (int i = 0; i < 3; i++) {
        if (food->baseParam[i] == 3) continue; // Ignore Vitality
        
        auto baseParamIt = fedMeldedBaseParamValue.find(food->baseParam[i]);
        if (baseParamIt == fedMeldedBaseParamValue.end()) continue;
        baseParamIt->second += min((int)(baseParamIt->second * 0.01f * food->valueHQ[i]), food->maxHQ[i]);
    }
}

void GearSet::popFood() {
    for (int i = 0; i < 3; i++) {
        if (selectedFood->baseParam[i] == 3) continue; // Ignore Vitality
        
        auto fedBaseParamIt = fedMeldedBaseParamValue.find(selectedFood->baseParam[i]);
        auto meldedBaseParamIt = meldedBaseParamValue.find(selectedFood->baseParam[i]);
        if (fedBaseParamIt == fedMeldedBaseParamValue.end() || meldedBaseParamIt == meldedBaseParamValue.end()) continue;
        fedBaseParamIt->second = meldedBaseParamIt->second;
    }
}

void GearSet::initFedMeldedStats() {
    for (auto& it : meldedBaseParamValue) {
        fedMeldedBaseParamValue[it.first] = it.second;
    }
}

float GearSet::meldedDamageMod() {
    return calcDamageMod(meldedBaseParamValue);
}

float GearSet::fedMeldedDamageMod() {
    return calcDamageMod(fedMeldedBaseParamValue);
}

float GearSet::calcDamageMod(const map<int, int>& baseParamValue) {
    auto result = (selectedJob->role == 1) ?
        Damage::Instance().damageMod(damagePhys, mainBaseParamValue, baseParamValue.at(22), baseParamValue.at(27), baseParamValue.at(44), baseParamValue.at(19)) :
        Damage::Instance().damageMod(damagePhys, mainBaseParamValue, baseParamValue.at(22), baseParamValue.at(27), baseParamValue.at(44));
    damageMod = result;
    return result;
}

int GearSet::meldedGcd() {
    return gcd(meldedBaseParamValue);
}

int GearSet::fedMeldedGcd() {
    return gcd(fedMeldedBaseParamValue);
}

int GearSet::gcd(const map<int, int>& baseParamValue) {
    int ssBaseParam = selectedJob->primaryStat == 4 || selectedJob->primaryStat == 5 ? 46 : 45;
    auto result = Damage::Instance().gcd(baseParamValue.at(ssBaseParam));
    gcdInt = result;
    return result;
}

string GearSet::gcdStr() {
    stringstream gcdStream;
    gcdStream << fixed << setprecision(2) << gcdInt / 100.0f;
    return gcdStream.str();
}

string GearSet::toJson() {
    stringstream setJson;
    setJson << "{"
            << "\"name\":\"" << gcdStr() << "\","
            << "\"items\":{";

    int ringIdx = 0;
    for (int i = 0; i < meldPerms.size(); i++) {
        auto& perm = meldPerms[i];
        auto piece = perm->gearPiece;

        if (i > 0) setJson << ",";

        setJson << "\"" << GearPiece::equipSlotXivGearName.at(piece->equipSlotCategory);

        if (piece->equipSlotCategory == 12) {
            setJson << GearPiece::equipSlotXivGearRingSuffix.at(ringIdx);
            ringIdx++;
        }

        setJson << "\":{"
                << "\"id\":" << piece->id << ","
                << "\"materia\":[";

        for (int j = 0; j < 5; j++) {
            auto materia = perm->materia[j];
            if (materia == nullptr) break;

            if (j > 0) setJson << ",";

            setJson << "{\"id\":" << materia->id << "}";
        }

        setJson << "]}";
    }

    setJson << "},"
            << "\"food\":" << selectedFood->id << ","
            << "\"job\":\"" << selectedJob->name << "\","
            << "\"level\":" << 100 << ","
            << "\"partyBonus\":" << 5 << "}";

    return setJson.str();
}