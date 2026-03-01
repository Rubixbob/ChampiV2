#include "Job.h"

Job::Job()
{
    //ctor
}

Job::~Job()
{
    //dtor
}

int Job::getSpeedBaseParam() const {
    return primaryStat == 4 || primaryStat == 5 ? BaseParam::SpellSpeed : BaseParam::SkillSpeed;
}

int Job::getWeaponDamageBaseParam() const {
    return primaryStat == 4 || primaryStat == 5 ? BaseParam::DamageMag : BaseParam::DamagePhys;
}