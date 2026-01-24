#include "GearPiece.h"

// TODO: delete later
#define MAT 54

GearPiece::GearPiece()
{
    //ctor
}

GearPiece::GearPiece(int t, string n, ifstream *file, int idx)
{
    type = t;
    name = n;
    *file >> wd;
    *file >> str;
    *file >> subs[0];
    *file >> subs[1];
    *file >> subs[2];
    *file >> subs[3];
    index = idx;

    init();
}

GearPiece::GearPiece(const GearPiece& other)
{
    type = other.type;
    name = other.name;
    wd = other.wd;
    str = other.str;
    mainStat = other.mainStat;
    subStat = other.subStat;
    thirdStat = other.thirdStat;
    fourthStat = other.fourthStat;
    permutation = other.permutation;
    index = other.index;
    for (int i = 0; i < 4; i++) {
        subs[i] = other.subs[i];
    }
//    for (int i = 0; i < 5; i++) {
//        melds[i] = other.melds[i];
//    }
    for (int j = 0; j < 9; j++) {
        for (int k = 0; k < 4; k++) {
            meldStatPermutation[j][k] = other.meldStatPermutation[j][k];
            matStat[j][k] = other.matStat[j][k];
        }
    }
    meldSlots = other.meldSlots;
    meldPermutations = other.meldPermutations;
    isUnique = other.isUnique;
}

GearPiece::~GearPiece()
{
    //dtor
}

void GearPiece::init()
{
    //TODO: check it's correct
    int maxSub = max(max(max(subs[0], subs[1]), subs[2]), subs[3]);
    thirdStat = -1;
    for (int i = 0; i < 4; i++) {
        if (subs[i] == maxSub)
            mainStat = i;
        else if (subs[i] != 0 && subs[i] != maxSub)
            subStat = i;
        else if (thirdStat < 0)
            thirdStat = i;
        else
            fourthStat = i;
    }

    //TODO: read from file ?
    meldSlots = 2;
//    if (type == 0)
//        meldSlots = 3;
    if ((name == "Dung" || name == "Ex2" || name == "Tomes1") && (type == 4 || type >= 7))
        meldSlots = 1;
    if (name == "Ult" && type == 0)
        meldSlots = 3;

    switch(meldSlots) {
        case 1:
            meldPermutations = 3;
            break;
        case 2:
            meldPermutations = (min(subs[mainStat] - subs[subStat], meldSlots * MAT) >= 1.5 * MAT) ? 6 : 5;
            break;
        case 3:
            meldPermutations = 9;
            break;
    }

    // TODO: do something about this
    permutation = 0;
//    melds[0] = subStat;
//
//
//    if (meldPermutations != 5) {
//        melds[1] = subStat;
//    } else {
//        permutation = 1;
//        melds[1] = thirdStat;
//    }


//    melds[2] = thirdStat;

    for (int j = 0; j < 9; j++) {
        for (int k = 0; k < 4; k++) {
            matStat[j][k] = 0;
        }
    }
    // Permutations
    switch(meldPermutations) {
        case 3:
            meldStatPermutation[0][mainStat] = 0;
            meldStatPermutation[0][subStat] = 1;
            meldStatPermutation[0][thirdStat] = 0;
            meldStatPermutation[0][fourthStat] = 0;
            matStat[0][subStat] = min(MAT, subs[mainStat] - subs[subStat]);
            meldStatPermutation[1][mainStat] = 0;
            meldStatPermutation[1][subStat] = 0;
            meldStatPermutation[1][thirdStat] = 1;
            meldStatPermutation[1][fourthStat] = 0;
            matStat[1][thirdStat] = MAT;
            meldStatPermutation[2][mainStat] = 0;
            meldStatPermutation[2][subStat] = 0;
            meldStatPermutation[2][thirdStat] = 0;
            meldStatPermutation[2][fourthStat] = 1;
            matStat[2][fourthStat] = MAT;
            break;
        case 5:
            meldStatPermutation[0][mainStat] = 0;
            meldStatPermutation[0][subStat] = 1;
            meldStatPermutation[0][thirdStat] = 1;
            meldStatPermutation[0][fourthStat] = 0;
            matStat[0][subStat] = min(MAT, subs[mainStat] - subs[subStat]);
            matStat[0][thirdStat] = MAT;
            meldStatPermutation[1][mainStat] = 0;
            meldStatPermutation[1][subStat] = 0;
            meldStatPermutation[1][thirdStat] = 2;
            meldStatPermutation[1][fourthStat] = 0;
            matStat[1][thirdStat] = MAT * 2;
            meldStatPermutation[2][mainStat] = 0;
            meldStatPermutation[2][subStat] = 0;
            meldStatPermutation[2][thirdStat] = 1;
            meldStatPermutation[2][fourthStat] = 1;
            matStat[2][thirdStat] = MAT;
            matStat[2][fourthStat] = MAT;
            meldStatPermutation[3][mainStat] = 0;
            meldStatPermutation[3][subStat] = 0;
            meldStatPermutation[3][thirdStat] = 0;
            meldStatPermutation[3][fourthStat] = 2;
            matStat[3][fourthStat] = MAT * 2;
            meldStatPermutation[4][mainStat] = 0;
            meldStatPermutation[4][subStat] = 1;
            meldStatPermutation[4][thirdStat] = 0;
            meldStatPermutation[4][fourthStat] = 1;
            matStat[4][subStat] = min(MAT, subs[mainStat] - subs[subStat]);
            matStat[4][fourthStat] = MAT;
            break;
        case 6:
            meldStatPermutation[0][mainStat] = 0;
            meldStatPermutation[0][subStat] = 2;
            meldStatPermutation[0][thirdStat] = 0;
            meldStatPermutation[0][fourthStat] = 0;
            matStat[0][subStat] = min(MAT * 2, subs[mainStat] - subs[subStat]);
            meldStatPermutation[1][mainStat] = 0;
            meldStatPermutation[1][subStat] = 1;
            meldStatPermutation[1][thirdStat] = 1;
            meldStatPermutation[1][fourthStat] = 0;
            matStat[1][subStat] = MAT;
            matStat[1][thirdStat] = MAT;
            meldStatPermutation[2][mainStat] = 0;
            meldStatPermutation[2][subStat] = 0;
            meldStatPermutation[2][thirdStat] = 2;
            meldStatPermutation[2][fourthStat] = 0;
            matStat[2][thirdStat] = MAT * 2;
            meldStatPermutation[3][mainStat] = 0;
            meldStatPermutation[3][subStat] = 0;
            meldStatPermutation[3][thirdStat] = 1;
            meldStatPermutation[3][fourthStat] = 1;
            matStat[3][thirdStat] = MAT;
            matStat[3][fourthStat] = MAT;
            meldStatPermutation[4][mainStat] = 0;
            meldStatPermutation[4][subStat] = 0;
            meldStatPermutation[4][thirdStat] = 0;
            meldStatPermutation[4][fourthStat] = 2;
            matStat[4][fourthStat] = MAT * 2;
            meldStatPermutation[5][mainStat] = 0;
            meldStatPermutation[5][subStat] = 1;
            meldStatPermutation[5][thirdStat] = 0;
            meldStatPermutation[5][fourthStat] = 1;
            matStat[5][subStat] = MAT;
            matStat[5][fourthStat] = MAT;
            break;
        case 9:
            meldStatPermutation[0][mainStat] = 0;
            meldStatPermutation[0][subStat] = 2;
            meldStatPermutation[0][thirdStat] = 1;
            meldStatPermutation[0][fourthStat] = 0;
            matStat[0][subStat] = min(MAT * 2, subs[mainStat] - subs[subStat]);
            matStat[0][thirdStat] = MAT;
            meldStatPermutation[1][mainStat] = 0;
            meldStatPermutation[1][subStat] = 1;
            meldStatPermutation[1][thirdStat] = 2;
            meldStatPermutation[1][fourthStat] = 0;
            matStat[1][subStat] = MAT;
            matStat[1][thirdStat] = MAT * 2;
            meldStatPermutation[2][mainStat] = 0;
            meldStatPermutation[2][subStat] = 0;
            meldStatPermutation[2][thirdStat] = 3;
            meldStatPermutation[2][fourthStat] = 0;
            matStat[2][thirdStat] = MAT * 3;
            meldStatPermutation[3][mainStat] = 0;
            meldStatPermutation[3][subStat] = 0;
            meldStatPermutation[3][thirdStat] = 2;
            meldStatPermutation[3][fourthStat] = 1;
            matStat[3][thirdStat] = MAT * 2;
            matStat[3][fourthStat] = MAT;
            meldStatPermutation[4][mainStat] = 0;
            meldStatPermutation[4][subStat] = 0;
            meldStatPermutation[4][thirdStat] = 1;
            meldStatPermutation[4][fourthStat] = 2;
            matStat[4][thirdStat] = MAT;
            matStat[4][fourthStat] = MAT * 2;
            meldStatPermutation[5][mainStat] = 0;
            meldStatPermutation[5][subStat] = 0;
            meldStatPermutation[5][thirdStat] = 0;
            meldStatPermutation[5][fourthStat] = 3;
            matStat[5][fourthStat] = MAT * 3;
            meldStatPermutation[6][mainStat] = 0;
            meldStatPermutation[6][subStat] = 1;
            meldStatPermutation[6][thirdStat] = 0;
            meldStatPermutation[6][fourthStat] = 2;
            matStat[6][subStat] = MAT;
            matStat[6][fourthStat] = MAT * 2;
            meldStatPermutation[7][mainStat] = 0;
            meldStatPermutation[7][subStat] = 1;
            meldStatPermutation[7][thirdStat] = 1;
            meldStatPermutation[7][fourthStat] = 1;
            matStat[7][subStat] = MAT;
            matStat[7][thirdStat] = MAT;
            matStat[7][fourthStat] = MAT;
            meldStatPermutation[8][mainStat] = 0;
            meldStatPermutation[8][subStat] = 2;
            meldStatPermutation[8][thirdStat] = 0;
            meldStatPermutation[8][fourthStat] = 1;
            matStat[8][subStat] = min(MAT * 2, subs[mainStat] - subs[subStat]);
            matStat[8][fourthStat] = MAT;
            break;
    }

    if (name == "Crafted")
        isUnique = false;
    else
        isUnique = true;

    // TODO : new way of calculating perms
//    meldComb[0] = (meldCombOld[0] + meldCombOld[1] == maxMeldSlots) ? (meldCombOld[0] + 1) : meldCombOld[0];
//    meldComb[1] = (meldComb[0] == meldCombOld[0]) ? ( meldCombOld[3] == 0 ? meldCombOld[1] + 1 : meldCombOld[1] ) : 0;
//    meldComb[2] = (meldComb[1] == meldCombOld[1]) ? meldCombOld[2] + 1 : 0;
//    meldComb[3] = maxMeldSlots - meldComb[0] - meldComb[1] - meldComb[2];
//    for (int i = 0; i < 4; i++) {
//        meldCombOld[i] = meldComb[i];
//    }
    // If stats given by all mats >= 0.5 * MAT) ?
    // Save the perm and move on
    // Otherwise ignore

    // Double that for pentamelds
}
