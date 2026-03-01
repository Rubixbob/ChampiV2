#include "BaseParam.h"

BaseParam::BaseParam()
{
    //ctor
}

BaseParam::~BaseParam()
{
    //dtor
}

int BaseParam::getStartingValue(int baseParam, int lvlModMain, int lvlModSub) {
    switch (baseParam) {
		case Strength:
		case Dexterity:
		case Vitality:
		case Intelligence:
		case Mind:
		case Piety:
		case Determination:
			return lvlModMain;
		case Tenacity:
		case DirectHit:
		case CriticalHit:
		case SkillSpeed:
		case SpellSpeed:
			return lvlModSub;
		case DamagePhys:
		case DamageMag:
		default:
			return 0;
    }
}