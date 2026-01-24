#include "BiSFinder.h"

//#define vit_melds
//#define ult_weapon // Use meldSlots = 3 in GearPiece
//#define MIN_SKS 2000
//#define MIN_SKS 715
#define MIN_SKS 420
//#define MIN_SKS 2278
//#define MAX_SKS 440
//#define MAX_SKS 850
#define MAX_SKS 5000
#define ILVL 710
#define MAT 54
#define MAT5 18
#define MAT7 20

#define MAX_THREADS 11

BiSFinder BiSFinder::m_instance = BiSFinder();

//int capTable[11];
//int subTable[11];
//int subMeldTable[11];
int capTableCrafted[11];
int subTableCrafted[11];
//int subMeldTableCrafted[11];
//int meldsPermutationsTable[11];
bool exclusionTable[3][11];

//#ifndef ult_weapon
//const int weaponMelds = 2;
//#else
//const int weaponMelds = 3;
//#endif
//int meldSlotsTable[11] = {weaponMelds, 2, 2, 2, 1, 2, 2, 1, 1, 1, 1};
//int meldSlotsTable[11] = {weaponMelds, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};

double bestDps = 0;
mutex mutex_dps;

unsigned int active_threads = 0;
mutex mutex_active_threads;

FullSet* full_set_buffer;
bool set_recovered = false;

mutex mutex_display;
//sf::Mutex mutex_saved_set;
mutex mutex_saved_sets[100];

mutex mutex_counter;
unsigned int m_counter;
int max_counter;
uint_fast64_t startTime;

double dirMult[6000];
double critMult[6000];
double detMult[6000];
double sksMult[6000];
double strMult[9000];

unsigned int baseMainStat;
unsigned int haste;

BiSFinder::BiSFinder()
{
    //ctor
    for (int i = 0; i < 100; i++) {
        m_saved_sets[i] = nullptr;
    }
}

BiSFinder::~BiSFinder()
{
    //dtor
}

void BiSFinder::init()
{
    cout << "Init started" << endl;
//    addExclusion("Raid", Weapon);
//    addExclusion("Raid", Head);
//    addExclusion("Raid", Body);
//    addExclusion("Raid", Hands);
//    addExclusion("Raid", Waist);
//    addExclusion("Raid", Legs);
//    addExclusion("Raid", Feet);
//    addExclusion("Raid", Earrings);
//    addExclusion("Raid", Necklace);
//    addExclusion("Raid", Bracelets);
//    addExclusion("Raid", Ring);

//    addExclusion("Tomes", Weapon);
//    addExclusion("Tomes", Head);
//    addExclusion("Tomes", Body);
//    addExclusion("Tomes", Hands);
//    addExclusion("Tomes", Waist);
//    addExclusion("Tomes", Legs);
//    addExclusion("Tomes", Feet);
//    addExclusion("Tomes", Earrings);
//    addExclusion("Tomes", Necklace);
//    addExclusion("Tomes", Bracelets);
//    addExclusion("Tomes", Ring);

//    addExclusion("Crafted", Weapon);
//    addExclusion("Crafted", Head);
//    addExclusion("Crafted", Body);
//    addExclusion("Crafted", Hands);
//    addExclusion("Crafted", Waist);
//    addExclusion("Crafted", Legs);
//    addExclusion("Crafted", Feet);
//    addExclusion("Crafted", Earrings);
//    addExclusion("Crafted", Necklace);
//    addExclusion("Crafted", Bracelets);
//    addExclusion("Crafted", Ring);

    ifstream configFile("SolverOld/config.txt", ios::in);
    if (!configFile)
        cout << "Erreur lecture SolverOld/config.txt" << endl;

    string label;
    configFile >> label >> baseMainStat >> label >> haste;

    string file_name = "SolverOld/gear.txt";
//    if (ILVL == 340) {
//        file_name = "bis/i340.txt";
//    } else if (ILVL == 370) {
//        file_name = "bis/i370.txt";
//    } else if (ILVL == 375) {
//        file_name = "bis/i375.txt";
//    } else if (ILVL == 400) {
//        file_name = "bis/i400.txt";
//    } else if (ILVL == 450) {
//        file_name = "bis/i450.txt";
//    } else if (ILVL == 470) {
//        file_name = "bis/i470.txt";
//    } else if (ILVL == 475) {
//        file_name = "bis/i475.txt";
//    } else if (ILVL == 499) {
//        file_name = "bis/i499.txt";
//    } else if (ILVL == 500) {
//        file_name = "bis/i500.txt";
//    } else if (ILVL == 530) {
//        file_name = "bis/i530.txt";
//    } else if (ILVL == 600) {
//        file_name = "bis/i600.txt";
//    } else if (ILVL == 605) {
//        file_name = "bis/i605.txt";
//    } else if (ILVL == 630) {
//        file_name = "bis/i630.txt";
//    } else if (ILVL == 660) {
//        file_name = "bis/i660.txt";
//    } else {
//        cout << "Unknown iLvl" << endl;
//    }
    ifstream file(file_name.c_str(), ios::in);
    if (!file)
        cout << "Erreur lecture " << file_name << endl;
    while (!file.eof()) {
        int type;
        string name;
        file >> type;
        if (file.eof())
            break;
        file >> name;

        if (isExcluded(name, type)) {
            getline(file, name);
            continue;
        }

        m_gearPieces[type].push_back(new GearPiece(type, name, &file, m_gearPieces[type].size()));
    }
    file.close();

    // TODO: rework in FullSet
    m_full_set.gear_set[0] = m_gearPieces[0][0]; // weapon
    m_full_set.gear_set[1] = m_gearPieces[1][0]; // head
    m_full_set.gear_set[2] = m_gearPieces[2][0]; // body
    m_full_set.gear_set[3] = m_gearPieces[3][0]; // hands
    //m_full_set.gear_set[4] = m_gearPieces[4][0]; // waist
    m_full_set.gear_set[5] = m_gearPieces[5][0]; // legs
    m_full_set.gear_set[6] = m_gearPieces[6][0]; // feet
    m_full_set.gear_set[7] = m_gearPieces[7][0]; // earrings
    m_full_set.gear_set[8] = m_gearPieces[8][0]; // necklace
    m_full_set.gear_set[9] = m_gearPieces[9][0]; // bracelets
    // TODO: ring perms and isUnique
    m_full_set.gear_set[10] = m_gearPieces[10][0]; // ring 1
    m_full_set.gear_set[11] = m_gearPieces[10][1]; // ring 2


    ifstream foodFile("SolverOld/food.txt", ios::in);
    if (!foodFile)
        cout << "Erreur lecture SolverOld/food.txt" << endl;
    while (!foodFile.eof()) {
        m_foods.emplace_back();
        food& newFood = m_foods.back();
        string line;
        getline(foodFile, line);
        newFood.name = line.substr(0, line.find("\t"));
        stringstream foodStream;
        foodStream << line.substr(line.find("\t"));
        for (int i = 0; i < 4; i++) {
            foodStream >> newFood.mult[i] >> newFood.cap[i];
        }
    }
    foodFile.close();

    m_full_set.food = &m_foods[0];
    m_full_set.updateStats();

    ifstream critMultFile("SolverOld/crit.txt", ios::in);
    if (!critMultFile)
        cout << "Erreur lecture SolverOld/crit.txt" << endl;
//    ifstream sksMultFile("bis/sks.txt", ios::in);
//    if (!sksMultFile)
//        cout << "Erreur lecture bis/sks.txt" << endl;

    for (int i = 0; i < 5000; i++) {
        dirMult[i+420] = floor( i * 550 / 2780.0 ) / 1000.0 * 0.25 + 1;
//        critMultFile >> critMult[i+420];
//        critMult[i+420] = ((floor( i / (2780.0 * 5) * 1000 ) / 1000.0 + 0.05 + 0.02) * (floor( i / (2780.0 * 5) * 1000 ) / 1000.0 + 0.40) + 1) / 1.028;
        critMult[i+420] = (floor( i / (2780.0 * 5) * 1000 ) / 1000.0 + 0.05) * (floor( i / (2780.0 * 5) * 1000 ) / 1000.0 + 0.40) + 1;
        detMult[i+440] = floor( i * 140 / 2780.0 ) / 1000.0 + 1;
//        sksMultFile >> sksMult[i+420];
        sksMult[i+420] = 1; // Needs checking
//         sksMult[i+384] = 1 + 0.233 * (floor(1000.0 / (1 - (floor( i * 130 / 3300.0 ) / 1000.0))) / 1000 - 1) // AA + DoT
//                            + 0.546 * (2.5 / (floor((1 - floor( i * 130 / 3300.0 ) / 1000.0 ) * 2.5 * 100) / 100) - 1); // GCD
//        sksMult[i+364] = 1 + 0.223 * (floor(1000.0 / (1 - (floor( i * 130 / 2170.0 ) / 1000.0))) / 1000 - 1)
//                           + 0.559 * (2.5 / (floor((1 - floor( i * 130 / 2170.0 ) / 1000.0 ) * 2.5 * 100) / 100) - 1);
    }
    for (int i = 0; i < 8000; i++) {
        strMult[i+440] = floor( 100 + ( i * 237 / 440.0) ) / 100.0;
    }
    critMultFile.close();
//    sksMultFile.close();
    cout << "Init done" << endl;
}

void BiSFinder::findBiS()
{
	auto maxHardwareThreads = thread::hardware_concurrency();
	cout << "Max hardware threads: " << maxHardwareThreads << endl;
    cout << "Solving started" << endl;
    startTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    mutex_counter.lock();
    m_counter = 0;
    mutex_counter.unlock();
    //TODO : rework with isUnique
    int ringPerms = m_gearPieces[10].size() * (m_gearPieces[10].size() - 1) / 2; // Combination stuff
    max_counter = ringPerms;
    for (int i = 0; i < 10; i++) {
        if (i == 4) continue;
        max_counter *= m_gearPieces[i].size();
    }
//    vector<sf::Thread*> threads;
    vector<MeldFinder*> meldFindersReady;
    vector<MeldFinder*> meldFinders;
//    cout << max_counter << endl;
//    char a;
//    cin.get(a);

    int pieceCounter[12];
    for (int i = 0; i < 12; i++) {
        pieceCounter[i] = 0;
    }

    int foodMaxSks = 0;
    for (unsigned int i = 0; i < m_foods.size(); i++) {
        if (m_foods[i].cap[3] > foodMaxSks)
            foodMaxSks = m_foods[i].cap[3];
    }

    int resultMeldFinder = -1;
    int resultSavedSet = -1;

    while (pieceCounter[10] < ringPerms || meldFinders.size() > 0) {
        if (active_threads < MAX_THREADS && meldFindersReady.size() > 0) {
//        if (threadsReady.size() > 0) {
            // Launch thread, switch to other list
//            threads.push_back(threadsReady.back());
            MeldFinder* meldFinder = meldFindersReady.back();
            meldFindersReady.pop_back();
            new jthread(&MeldFinder::findBestMelds, meldFinder);
            mutex_active_threads.lock();
            active_threads++;
            mutex_active_threads.unlock();
//            threads.back()->launch();
        } else if (meldFindersReady.size() < MAX_THREADS && pieceCounter[10] < ringPerms) {
//        } else if (pieceCounter[10] < ringPerms) {
            // Prepare thread
            //TODO: Rework for pentamelds, probably in FullSet+GearPiece
            int sksSlotsAvailable = 0;
            for (int i = 0; i < 12; i++) {
                if (i == 4) continue;
                GearPiece* piece = m_full_set.gear_set[i];
                // TODO: rework sksSlotsAvailable into sksAvailable
                if (piece->subStat == 3)
                    sksSlotsAvailable = sksSlotsAvailable + round(1.0 * min(piece->subs[piece->mainStat] - piece->subs[piece->subStat], piece->meldSlots * MAT) / MAT);
                else if (piece->thirdStat == 3 || piece->fourthStat == 3)
                    sksSlotsAvailable = sksSlotsAvailable + piece->meldSlots;
            }

            if (!m_full_set.gear_set[10]->name.compare("Crafted") || m_full_set.gear_set[10]->name.compare(m_full_set.gear_set[11]->name)) {
                if (m_full_set.gearStats.subs[3] <= MAX_SKS && m_full_set.gearStats.subs[3] + sksSlotsAvailable * MAT + foodMaxSks >= MIN_SKS) {
                    MeldFinder* meldFinder = new MeldFinder();
                    meldFinder->m_full_set = new FullSet(m_full_set);
                    for (int i = 0; i < (int) m_foods.size(); i++) {
                        meldFinder->m_foods.push_back(&m_foods[i]);
                    }
                    meldFindersReady.push_back(meldFinder);
                    meldFinders.push_back(meldFinder);
                }
            }
            // Switch gear
            bool switchP = true;
            for (int i = 1; i < 11; i++) { // TODO : RINGS + WEAPON /////////////////////////////////////////////////////////////////////
                if (switchP) {
                    pieceCounter[i]++;
                    // until 9, compare to size and reset
                    // at 10, compare to ringPerms and don't reset
                    if (pieceCounter[i] >= ( i >= 10 ? ringPerms : (int) m_gearPieces[i].size() )) {
                        if (i < 10)
                            pieceCounter[i] = 0;
                    } else {
                        m_full_set.switchPiece(i, m_gearPieces);
                        switchP = false;
                    }
                }
            }
        } else if (resultMeldFinder >= 0) {
            // Compare 1 result and save (+ delete when done)
            while (resultSavedSet < 100 && !(meldFinders[resultMeldFinder]->m_saved_sets[resultSavedSet] && (!m_saved_sets[resultSavedSet] || meldFinders[resultMeldFinder]->m_saved_sets[resultSavedSet]->dps > m_saved_sets[resultSavedSet]->dps)))
                resultSavedSet++;
            if (resultSavedSet < 100) {
                if (m_saved_sets[resultSavedSet]) {
                    delete m_saved_sets[resultSavedSet];
                }
                m_saved_sets[resultSavedSet] = meldFinders[resultMeldFinder]->m_saved_sets[resultSavedSet];
                meldFinders[resultMeldFinder]->m_saved_sets[resultSavedSet] = nullptr;
            }
            resultSavedSet++;
            if (resultSavedSet >= 100) {
                delete meldFinders[resultMeldFinder];
                meldFinders.erase(meldFinders.begin() + resultMeldFinder);
                resultMeldFinder = -1;
                resultSavedSet = -1;
            }
        } else {
            // Go through meldfinders
            int k = 0;
            while (resultMeldFinder < 0 && k < (int) meldFinders.size()) {
                // If done, save pos and break loop
                if (meldFinders[k]->done) {
                    resultMeldFinder = k;
                    resultSavedSet = 0;
                }
                k++;
            }
            // If no result to check, sleep
            if (resultMeldFinder < 0) {
                this_thread::sleep_for(milliseconds(1));
            }
        }
    }

    uint_fast64_t currentTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    uint_fast64_t elapsed = currentTime - startTime;
    cout << "Solving done in: " << (int) (elapsed / 1000 / 60) << "m" << setw(2) << (int) (elapsed / 1000) % 60 << "s" << setw(3) << elapsed % 1000 << endl
         << "Results can be found in SolverOld/results.txt" << endl;
    displayResults();

//    for (int i = 0; i < 100; i++) {
//        if (m_saved_sets[i])
//            delete m_saved_sets[i];
//    }
//    for (int i = 0; i < 11; i++) {
//        for (unsigned int j = 0; j < m_gearPieces[i].size(); j++) {
//            delete m_gearPieces[i][j];
//        }
//    }
//    threads.clear();
}

void BiSFinder::displayResults()
{
    // Rankings
    for (int i = 0; i < 100; i++) {
        if (m_saved_sets[i]) {
            m_saved_sets[i]->setRank = i + 1;
            for (int j = 0; j < i; j++) {
                if (m_saved_sets[j] && m_saved_sets[i]->dps > m_saved_sets[j]->dps) {
                    m_saved_sets[i]->setRank--;
                    m_saved_sets[j]->setRank++;
                }
            }
        }
    }
    ofstream resultFile("SolverOld/results.txt", ios::out);
//    cout << "Results:" << endl;
    for (int i = 0; i < 100; i++) {
        if (m_saved_sets[i]) {
//        m_saved_sets[i].displayStats(cout);
//        m_saved_sets[i].displaySet(cout);
            m_saved_sets[i]->displayStats(resultFile);
            m_saved_sets[i]->displaySet(resultFile);
        }
    }
}

//double BiSFinder::computeDps(const fullSet& full_set)
//{
////    return (m_wd + floor(292 * 115 / 1000.0)) * floor( (1 + ( (m_str - 292) * 1.25 / 292) ) * 100) / 100.0
////    return ((floor( (m_dir_f - 364) * 550 / 2170.0 ) / 1000.0) * (0.25) + 1)
////         * ((floor( (m_crit_f - 364) / (2170.0 * 5) * 1000 ) / 1000.0 + 0.05 + 0.02) * (floor( (m_crit_f - 364) / (2170.0 * 5) * 1000 ) / 1000.0 + 0.40) + 1) / 1.028
////         * (floor( (m_det_f - 292) * 130 / 2170.0 ) / 1000.0 + 1)
////         * (1+0.779*(2.5/((1-( (m_sks_f - 364) * 0.13 / 2170 ))*2.5)-1));
//    return dirMult[full_set.dir_f] * critMult[full_set.crit_f] * detMult[full_set.det_f] * sksMult[full_set.sks_f] * strMult[full_set.str_p];
//}

void BiSFinder::addExclusion(string name, int type)
{
    int row = -1;
    if (!name.compare("Raid")) {
        row = 0;
    } else if (!name.compare("Tomes")) {
        row = 1;
    } else {
        row = 2;
    }
    exclusionTable[row][type] = true;
}

bool BiSFinder::isExcluded(string name, int type)
{
    int row = -1;
    if (!name.compare("Raid")) {
        row = 0;
    } else if (!name.compare("Tomes")) {
        row = 1;
    } else {
        row = 2;
    }
    return exclusionTable[row][type];
}
