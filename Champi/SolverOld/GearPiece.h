#ifndef GEARPIECE_H
#define GEARPIECE_H

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <sstream>
#include <vector>
#include <math.h>

using namespace std;

//enum gearType { Weapon=0, Head=1, Body=2, Hands=3, Waist=4, Legs=5, Feet=6,
//                Earrings=7, Necklace=8, Bracelets=9, Ring=10 };

class GearPiece
{
    public:
        GearPiece();
        GearPiece(int t, string n, ifstream *file, int idx);
        GearPiece(const GearPiece& other);
        virtual ~GearPiece();

        int type;
        string name;
        int wd;
        int str;
        int mainStat;
        int subStat;
        int thirdStat;
        int fourthStat;
        int permutation;
        int index;
        int subs[4];
//        int melds[5];

        int meldStatPermutation[9][4];
        int matStat[9][4];

        int meldSlots;
        int meldPermutations;

        bool isUnique;

        void init();

    protected:

    private:
};

#endif // GEARPIECE_H
