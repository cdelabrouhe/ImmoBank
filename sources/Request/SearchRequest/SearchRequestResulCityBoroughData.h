#pragma once

#include "SearchRequestResult.h"
#include "SearchRequestAnnounce.h"

struct SearchRequestResulCityBoroughData : public SearchRequestResult
{
	SearchRequestResulCityBoroughData() : SearchRequestResult(SearchRequestType_CityBoroughData) {}
	SearchRequestResulCityBoroughData(SearchRequestAnnounce& _request) : SearchRequestResult(SearchRequestType_CityBoroughData)
	{
		*this = _request;
	}

	BoroughData	m_data;
};