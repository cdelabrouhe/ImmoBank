#pragma once

#include <vector>
#include "Request\SearchRequest.h"

class CitySelector
{
public:
	CitySelector() {}

	bool Display();
	const sCity* GetSelectedCity() const;

private:
	std::vector<sCity>					m_cities;
	char								m_inputTextCity[256];
	int									m_selectedCityID = 0;
	int									m_cityNameRequestID = -1;
};