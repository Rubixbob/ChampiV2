#include "GearSet.h"

GearSet::GearSet()
{
    //ctor
}

GearSet::GearSet(vector<GearPiece*> gearPieces)
{
    for (auto piece : gearPieces) {
        meldPerms.push_back(&piece->meldPerms[0]);
    }
}

GearSet::~GearSet()
{
    //dtor
}

void GearSet::initStats(const Job* job)
{
    selectedJob = job;
    damagePhys = 0;
    damageMag = 0;
    delayms = 0;
    mainBaseParamValue = Damage::Instance().getLvlModMain() * job->primaryStatMod / 100;
    setBaseParamValue[22] = Damage::Instance().getLvlModSub(); // DH
    setBaseParamValue[27] = Damage::Instance().getLvlModSub(); // CRIT
    setBaseParamValue[44] = Damage::Instance().getLvlModMain(); // DET
    int ssBaseParam = job->primaryStat == 4 || job->primaryStat == 5 ? 46 : 45;
    setBaseParamValue[ssBaseParam] = Damage::Instance().getLvlModSub(); // SS
    if (job->role == 1) setBaseParamValue[19] = Damage::Instance().getLvlModSub(); // TNC
    if (job->role == 4) setBaseParamValue[6] = Damage::Instance().getLvlModSub(); // PIE

    for (auto perm : meldPerms) {
        auto piece = perm->gearPiece;
        damagePhys += piece->damagePhys;
        damageMag += piece->damageMag;
        delayms += piece->delayms;
        for (int i = 0; i < 6; i++) {
            if (piece->baseParam[i] == 0) break;
            if (piece->baseParam[i] == 3) continue; // Ignore Vitality
            if (piece->baseParam[i] == job->primaryStat) mainBaseParamValue += piece->baseParamValue[i];
            setBaseParamValue[piece->baseParam[i]] += piece->baseParamValue[i];
        }
    }

    // Party bonus
    mainBaseParamValue *= 1.05f;
}

void GearSet::updateMeldedStats() {
    for (auto& it : setBaseParamValue) {
        meldedBaseParamValue[it.first] = it.second;
    }

    for (auto perm : meldPerms) {
        for (auto& baseParamIt : perm->baseParamMatValue) {
            meldedBaseParamValue[baseParamIt.first] += baseParamIt.second;
        }
    }
}

void GearSet::updateMeldedStats(const map<int, vector<int>>& meldComb, const vector<int>& releventMateriaBaseParam) {
    for (auto& it : setBaseParamValue) {
        meldedBaseParamValue[it.first] = it.second;
    }

    for (auto& gradeIt : meldComb) {
        int value = Materia::valueByGrade[gradeIt.first];
        for (int idx = 0; idx < gradeIt.second.size(); idx++) {
            int matCount = gradeIt.second[idx];
            if (matCount == 0) continue;
            meldedBaseParamValue[releventMateriaBaseParam[idx]] += matCount * value;
        }
    }
}

void GearSet::updateFedMeldedStats(Food* food) {
    selectedFood = food;

    for (auto& it : meldedBaseParamValue) {
        fedMeldedBaseParamValue[it.first] = it.second;
    }

    for (int i = 0; i < 3; i++) {
        if (food->baseParam[i] == 3) continue; // Ignore Vitality

        fedMeldedBaseParamValue[food->baseParam[i]] += min((int) (fedMeldedBaseParamValue[food->baseParam[i]] * 0.01f * food->valueHQ[i]), food->maxHQ[i]);
    }
}

float GearSet::setDamageMod() {
    return calcDamageMod(setBaseParamValue);
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

int GearSet::setGcd() {
    return gcd(setBaseParamValue);
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