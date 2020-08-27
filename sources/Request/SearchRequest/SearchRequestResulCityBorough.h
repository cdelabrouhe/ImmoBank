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
		virtual ~SearchRequestResulCityBorough() {}

		std::string		m_name;
		unsigned int	m_internalID = 0xFFFFFFFF;
	};
}