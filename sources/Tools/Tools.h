#pragma once

#include <string>
#include <vector>

struct sPrice;

class Tools
{
public:
	static float ComputeRentabilityRate(float _rent, float _price);
	static bool ExtractPricesFromHTMLSource(const std::string& _source, sPrice& _rentT1, sPrice& _rentT2, sPrice& _rentT3, sPrice& _rentT4Plus, sPrice& _buyApartment, sPrice& _buyHouse);
};