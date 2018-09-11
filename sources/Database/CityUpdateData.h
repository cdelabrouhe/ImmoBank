#pragma once

#include <string>
#include <vector>
#include "BoroughData.h"

class CityUpdateData
{
public:
	void Init();
	bool Process();

	sCity	m_city;
	int		m_boroughsListID = -1;
};