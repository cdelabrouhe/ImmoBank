#pragma once

#include "SearchRequestResult.h"
#include "SearchRequestAnnounce.h"

struct SearchRequestResulCityBorough : public SearchRequestResult
{
	SearchRequestResulCityBorough() : SearchRequestResult(SearchRequestType_CityBoroughs) {}
	SearchRequestResulCityBorough(SearchRequestAnnounce& _request) : SearchRequestResult(SearchRequestType_CityBoroughs)
	{
		*this = _request;
	}

	std::string		m_name;
	unsigned int	m_internalID = 0xFFFFFFFF;
};