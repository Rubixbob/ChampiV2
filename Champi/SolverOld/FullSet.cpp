#include "FullSet.h"

string gearTypeTable[11] = { "Weapon", "Head", "Body", "Hands", "Waist", "Legs", "Feet",
                             "Earrings", "Necklace", "Bracelets", "Ring" };
string materiaTable[5] = { "Direct Hit Rate", "Critical Hit", "Determination", "Skill Speed", "Vitality" };

FullSet::FullSet()
{
    //ctor
}

FullSet::FullSet(const FullSet& other)
{
    for (int i = 0; i < 12; i++) {
        if (i == 4) continue;
        gear_set[i] = other.gear_set[i];
    }
    gearStats.wd = other.gearStats.wd;
    meldedGearStats.wd = other.meldedGearStats.wd;
    foodMeldedGearStats.wd = other.foodMeldedGearStats.wd;
    gearStats.str = other.gearStats.str;
    meldedGearStats.str = other.meldedGearStats.str;
    foodMeldedGearStats.str = other.foodMeldedGearStats.str;
    for (int i = 0; i < 4; i++) {
        gearStats.subs[i] = other.gearStats.subs[i];
        meldedGearStats.subs[i] = other.meldedGearStats.subs[i];
        foodMeldedGearStats.subs[i] = other.foodMeldedGearStats.subs[i];
    }
    gcd = other.gcd;
    dps = other.dps;
    setRank = other.setRank;
    food = other.food;
    for (int i = 0; i < 12; i++) {
        if (i == 4) continue;
        for (int j = 0; j < 5; j++) {
            gearMeldTable[i][j] = other.gearMeldTable[i][j];
        }
    }
}

FullSet::~FullSet()
{
    //dtor
}

void FullSet::updateStats()
{
    gearStats.wd = 0;
    gearStats.str = baseMainStat;
    gearStats.subs[0] = 420;
    gearStats.subs[1] = 420;
    gearStats.subs[2] = 440;
    gearStats.subs[3] = 420;
    for (int i = 0; i < 12; i++) {
        if (i == 4) continue;
        GearPiece* piece = gear_set[i];

        gearStats.wd += piece->wd;
        gearStats.str += piece->str;
        for (int k = 0; k < 4; k++) {
            gearStats.subs[k] += piece->subs[k];
        }
    }

    meldedGearStats.wd = gearStats.wd;
    meldedGearStats.str = gearStats.str;
    for (int i = 0; i < 4; i++) {
        meldedGearStats.subs[i] = gearStats.subs[i];
    }
    for (int j = 0; j < 12; j++) {
        if (j == 4) continue;
        GearPiece* piece = gear_set[j];
        for (int i = 0; i < 4; i++) {
            meldedGearStats.subs[i] += piece->matStat[piece->permutation][i];
        }
    }

    foodMeldedGearStats.wd = meldedGearStats.wd;
    foodMeldedGearStats.str = meldedGearStats.str * 1.05;
    for (int i = 0; i < 4; i++) {
        foodMeldedGearStats.subs[i] = meldedGearStats.subs[i] + min(meldedGearStats.subs[i] * food->mult[i], food->cap[i]);
    }
}

void FullSet::switchPiece(int type, vector<GearPiece*>* gearPieces)
{
    if (type == 11) {
        cout << "Switch Piece Type 11, shouldn't be here..." << endl;
        return;
    }
    if (type == 4) {
        cout << "Switch Piece Type 4, shouldn't be here..." << endl;
        return;
    }
    // TODO: weapon ///////////////////////////////////////
    if (type <= 0)// || type >= 10)
        return;


    GearPiece* piece[2];
    unsigned int index[2];
    piece[0] = gear_set[type];
    vector<GearPiece*> availablePieces = gearPieces[type];
    index[0] = piece[0]->index;

    if (type == 10) {
        piece[1] = gear_set[type + 1];
        index[1] = piece[1]->index;
//        cout << "Before " << index[0] << " " << index[1] << endl;
        if (index[1] == availablePieces.size() - 1) {
            index[0]++;
            if (!availablePieces[index[0]]->isUnique)
                index[1] = index[0];
            else
                index[1] = index[0] + 1;
        } else {
            index[1]++;
        }
//        cout << "After " << index[0] << " " << index[1] << endl;
    } else {
        index[0] = (index[0] + 1) % availablePieces.size();
    }


    for (int j = 0; j <= (type == 10 ? 1 : 0); j++) {
        gearStats.wd -= piece[j]->wd;
        gearStats.str -= piece[j]->str;
        for (int i = 0; i < 4; i++) {
            gearStats.subs[i] -= piece[j]->subs[i];
        }

        gear_set[type + j] = availablePieces[index[j]];
        piece[j] = gear_set[type + j];

        gearStats.wd += piece[j]->wd;
        gearStats.str += piece[j]->str;
        for (int i = 0; i < 4; i++) {
            gearStats.subs[i] += piece[j]->subs[i];
        }
    }
}

void FullSet::displayStats(std::ostream& output)
{
    output << "---------- GCD: " << gcd / 100.0 << /*" | Mult:" << dps << " | Rank:" << setRank <<*/ " ----------" << endl
          << "Food: " << food->name << endl
          << "Wd  : " << foodMeldedGearStats.wd << endl
          << "Str : " << gearStats.str << " | " << foodMeldedGearStats.str << endl
          << "Dh  : " << foodMeldedGearStats.subs[0]                       << endl
          << "Crit: " << foodMeldedGearStats.subs[1]                       << endl
          << "Det : " << foodMeldedGearStats.subs[2]                       << endl
          << "Sks : " << foodMeldedGearStats.subs[3]                       << endl;
//    output << "---------- GCD: " << gcd / 100.0 << /*" | Mult:" << dps << " | Rank:" << setRank <<*/ " ----------" << endl
//          << "Food: " << food->name << endl
//          << "Wd  : " << foodMeldedGearStats.wd << endl
//          << "Str : " << gearStats.str << " | " << foodMeldedGearStats.str << " | Mult: " << strMult[foodMeldedGearStats.str]   << endl
//          << "Dh  : " << foodMeldedGearStats.subs[0]                       << " | Mult: " << dirMult[foodMeldedGearStats.subs[0]]   << endl
//          << "Crit: " << foodMeldedGearStats.subs[1]                       << " | Mult: " << critMult[foodMeldedGearStats.subs[1]] << endl
//          << "Det : " << foodMeldedGearStats.subs[2]                       << " | Mult: " << detMult[foodMeldedGearStats.subs[2]]   << endl
//          << "Sks : " << foodMeldedGearStats.subs[3]                       << " | Mult: " << sksMult[foodMeldedGearStats.subs[3]]   << endl;
}

void FullSet::displaySet(std::ostream& output)
{
    for (int i = 0; i < 12; i++) {
        if (i == 4) continue;

        GearPiece* piece = gear_set[i];
        if (piece) {
            output << left << setw(10) << gearTypeTable[piece->type] << " " << left << setw(6) << piece->name << " "
                   << left << setw(16) << materiaTable[gearMeldTable[i][0]];
            if (piece->meldSlots >= 2)
                output << " " << left << setw(16) << materiaTable[gearMeldTable[i][1]];
            if (piece->meldSlots >= 3)
                output << " " << left << setw(16) << materiaTable[gearMeldTable[i][2]];
            output << endl;
        }
    }
}

double FullSet::computeDps()
{
    return dirMult[foodMeldedGearStats.subs[0]] * critMult[foodMeldedGearStats.subs[1]] * detMult[foodMeldedGearStats.subs[2]] * sksMult[foodMeldedGearStats.subs[3]] * strMult[foodMeldedGearStats.str];
}
