#ifndef BISFINDER_H
#define BISFINDER_H

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <sstream>
#include <vector>
#include <math.h>
#include <chrono>
#include <thread>

#include "MeldFinder.h"
#include "GearPiece.h"
#include "CommonStructs.h"
#include "FullSet.h"

using namespace std;
using namespace std::chrono;

class BiSFinder
{
    public:
        BiSFinder();
        virtual ~BiSFinder();
        static BiSFinder& Instance() { return m_instance; }
        void init();
        void findBiS();
//        void switchPiece(int type);
//        void initMeldsRight(GearPiece* piece);
//        void initMeldsLeft(GearPiece* piece);
//        void initMeldsWeapon(GearPiece* piece);
//        void initMeldsRight(craftedRight* piece);
//        void initMeldsLeft(craftedLeft* piece);
//        void initMeldsWeapon(craftedWeapon* piece);
//        void switchMelds(gearPiece* piece);
//        void switchMelds(GearPiece* piece, FullSet& full_set);
//        void displayStats();
//        void displayStats(const FullSet& full_set, std::ostream& output);
//        void displaySet(const FullSet& full_set, std::ostream& output);
//        void displaySavedSet();
        void displayResults();
//        double computeDps();
//        double computeDps(const FullSet& full_set);
//        void saveSet();
//        void saveSet(const fullSet& full_set);
//        void loadSet();
    protected:
    private:
        vector<GearPiece*> m_gearPieces[11];
//        fullSet m_saved_set;
        FullSet* m_saved_sets[100];
//        food m_food;
        vector<food> m_foods;
        FullSet m_full_set;
//        int64_t _hash() { return (int64_t)3000*(int64_t)3000*(int64_t)3000*(int64_t)(m_sks-364)+(int64_t)3000*(int64_t)3000*(int64_t)(m_det-292)+
//                          (int64_t)3000*(int64_t)(m_crit-364)+(int64_t)(m_dir-364); };
//        int64_t _hash(const fullSet& full_set) { return (int64_t)5000*(int64_t)5000*(int64_t)5000*(int64_t)(full_set.sks-380)+(int64_t)5000*(int64_t)5000*(int64_t)(full_set.det-340)
//                                                 +(int64_t)5000*(int64_t)(full_set.crit-380)+(int64_t)(full_set.dir-380); };
//        void fillNewPiece(ifstream *file, gearPiece *piece);
//        unsigned int cap(unsigned int a, unsigned int b, unsigned int c, unsigned int d);
//        unsigned int sub(unsigned int a, unsigned int b, unsigned int c, unsigned int d);
//        void increaseMeldCounter(int i, int* meldCounter);//, int* switchMeldsCounter);
//        void increaseMeldCounter(int i, int* meldCounter, fullSet& full_set);//, int* switchMeldsCounter);

//        void updateStats();
//        void updateStats(FullSet& full_set);
//        void updateFood();
//        void updateFood(FullSet& full_set);
//        int valueFromMateria(GearPiece* piece, int stat);
//        void copyFullSet(fullSet* full_set);
//        void copyFullSet(const fullSet& src, fullSet* dst);
//        void findBestMelds();
//        void findBestMeldsTry();
//        void findBestMeldsNew();
//        int baseSks(const FullSet& full_set);
        void addExclusion(string name, int type);
        bool isExcluded(string name, int type);

        static BiSFinder m_instance;
};

#endif // BISFINDER_H
