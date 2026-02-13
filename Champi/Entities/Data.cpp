#include "Data.h"

Data Data::_instance = Data();

Data::Data()
{
    FileReader::Instance().readBaseParamLight();
    FileReader::Instance().readFoodLight();
    FileReader::Instance().readGearPieceLight();
    FileReader::Instance().readJobLight();
    FileReader::Instance().readMateriaLight();
}

Data::~Data()
{
    //dtor
}