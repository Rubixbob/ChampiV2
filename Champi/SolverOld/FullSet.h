#ifndef FULLSET_H
#define FULLSET_H

#include "CommonStructs.h"
#include "GearPiece.h"

class FullSet
{
    public:
        FullSet();
        FullSet(const FullSet& other);
        virtual ~FullSet();

        GearPiece* gear_set[12];
        statStruct gearStats;
        statStruct meldedGearStats;
        statStruct foodMeldedGearStats;
        unsigned int gcd;
        double dps;
        unsigned int setRank;
        food* food;
        int gearMeldTable[12][5];

        void updateStats();
        void switchPiece(int type, vector<GearPiece*>* gearPieces);
        void displayStats(std::ostream& output);
        void displaySet(std::ostream& output);
        double computeDps();

    protected:

    private:
};

#endif // FULLSET_H
