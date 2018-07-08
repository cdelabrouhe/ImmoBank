#include "Tools.h"

float Tools::ComputeRentabilityRate(float _rent, float _price)
{
	return _rent * 12.f * 100.f / _price;
}
