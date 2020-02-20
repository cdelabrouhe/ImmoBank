#pragma once

#include "SearchRequestResult.h"
#include "SearchRequestAnnounce.h"

namespace ImmoBank
{
	struct SearchRequestResulCityBorough : public SearchRequestResult
	{
		SearchRequestResulCityBorough() : SearchRequestResult(SearchRequestType_CityBoroughs) {}
		SearchRequestResulCityBorough(SearchRequestAnnounce& _request) : SearchRequestResult(SearchRequestType_CityBoroughs)
		{
			*this = _request;
		}

		std::string		m_name;
		std::string		m_logicImmoID;
		unsigned int	m_internalID = 0xFFFFFFFF;
		unsigned int	m_selogerID = 0xFFFFFFFF;
	};
}