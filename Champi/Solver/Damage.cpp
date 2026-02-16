#include "Damage.h"

Damage Damage::_instance = Damage();

Damage::Damage()
{
    //ctor
}

Damage::~Damage()
{
    //dtor
}

void Damage::init(Job* job, int level) {
    _wdMod.clear();
    _mainStatMod.clear();
    _dhMod.clear();
    _critMod.clear();
    _detMod.clear();
    _tenMod.clear();
    _gcd.clear();

    _jobMod = job->primaryStatMod;
    if (job->name == "MNK") {
        _haste = 20;
    } else if (job->name == "NIN") {
        _haste = 15;
    } else if (job->name == "SAM") {
        _haste = 13;
    } else if (job->name == "VPR") {
        _haste = 15;
    } else {
        _haste = 0;
    }

    if (level == 100) {
        _lvlModAP = 237;
        _lvlModMain = 440;
        _lvlModSub = 420;
        _lvlModDiv = 2780;
    } else if (level == 90) {
        _lvlModAP = 195;
        _lvlModMain = 390;
        _lvlModSub = 400;
        _lvlModDiv = 1900;
    } else if (level == 80) {
        _lvlModAP = 165;
        _lvlModMain = 340;
        _lvlModSub = 380;
        _lvlModDiv = 1300;
    } else if (level == 70) {
        _lvlModAP = 125;
        _lvlModMain = 292;
        _lvlModSub = 364;
        _lvlModDiv = 900;
    }

    // TODO: Get data from file ItemLevel
    for (int i = 0; i <= 158; i++) { // Max PhysicalDamage
        _wdMod.emplace_back((float)(_lvlModMain * _jobMod / 1000 + i));
    }
    for (int i = 0; i <= 6887; i++) { // (_lvlModMain * primaryStatMod + max strength) * 1.05
        _mainStatMod.emplace_back((100 + (_lvlModAP * (i - _lvlModMain) / _lvlModMain)) / 100.0f);
    }
    for (int i = 0; i <= 5761; i++) { // max(_lvlModMain, _lvlModSub) + max substat + max food + max slots * max materia
        _dhMod.emplace_back(((i - _lvlModSub) * 550 / _lvlModDiv) / 1000.0f * 0.25f + 1);
        _critMod.emplace_back((50 + (i - _lvlModSub) * 200 / _lvlModDiv) / 1000.0f * (400 + (i - _lvlModSub) * 200 / _lvlModDiv) / 1000.0f + 1);
        _detMod.emplace_back((1000 + (i - _lvlModMain) * 140 / _lvlModDiv) / 1000.0f);
        _tenMod.emplace_back((1000 + (i - _lvlModSub) * 112 / _lvlModDiv) / 1000.0f);
        _gcd.emplace_back((int)floor(floor(2.5f * (1000 - (130 * (i - _lvlModSub) / _lvlModDiv))) * (100 - _haste) / 1000.0f));
    }
}

float Damage::wdMod(int wd) {
    return _wdMod[wd];
}

float Damage::mainStatMod(int mainStat) {
    return _mainStatMod[mainStat];
}

float Damage::dhMod(int dh) {
    return _dhMod[dh];
}

float Damage::critMod(int crit) {
    return _critMod[crit];
}

float Damage::detMod(int det) {
    return _detMod[det];
}

float Damage::tenMod(int ten) {
    return _tenMod[ten];
}

int Damage::gcd(int ss) {
    return _gcd[ss];
}

float Damage::damageMod(int wd, int mainStat, int dh, int crit, int det) {
    return damageMod(wd, mainStat, dh, crit, det, _lvlModSub);
}

float Damage::damageMod(int wd, int mainStat, int dh, int crit, int det, int ten) {
    return wdMod(wd) * mainStatMod(mainStat) * dhMod(dh) * critMod(crit) * detMod(det) * tenMod(ten);
}