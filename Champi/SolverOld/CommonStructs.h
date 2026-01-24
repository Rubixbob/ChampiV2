#ifndef COMMONSTRUCTS_H_INCLUDED
#define COMMONSTRUCTS_H_INCLUDED

#include <string>
#include <mutex>
#include <chrono>

#include "GearPiece.h"

using namespace std;

struct food { string name; double mult[4]; double cap[4]; };
struct statStruct { int wd; int str; int subs[4]; };
extern double dirMult[6000];
extern double critMult[6000];
extern double detMult[6000];
extern double sksMult[6000];
extern double strMult[9000];
extern unsigned int active_threads;
extern mutex mutex_active_threads;
extern mutex mutex_counter;
extern mutex mutex_display;
extern unsigned int m_counter;
extern int max_counter;
//extern sf::Clock m_clock;
extern uint_fast64_t startTime;
extern unsigned int baseMainStat;
extern unsigned int haste;

#endif // COMMONSTRUCTS_H_INCLUDED
