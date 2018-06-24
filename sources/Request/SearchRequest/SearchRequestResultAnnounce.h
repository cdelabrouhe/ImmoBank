#pragma once

#include "SearchRequestResult.h"
#include "SearchRequestAnnounce.h"

struct SearchRequestResultAnnounce : public SearchRequestResult
{
	SearchRequestResultAnnounce() : SearchRequestResult(SearchRequestType_Announce) {}
	SearchRequestResultAnnounce(SearchRequestAnnounce& _request) : SearchRequestResult(SearchRequestType_Announce)
	{
		*this = _request;
	}
	std::string m_database;
	std::string m_name;
	std::string m_description;
	std::string m_URL;
	sCity		m_city;
	Type		m_type = Type_NONE;
	Category	m_category = Category_NONE;
	BoroughData	m_selectedBorough;
	std::vector<BoroughData> m_boroughs;
	int			m_price = 0;
	float		m_surface = 0.f;
	int			m_nbRooms = 0;
	int			m_nbBedRooms = 0;
	int			m_selectedBoroughID = 0;

	virtual void PostProcess() override;
	virtual void Display() override;

	SearchRequestResultAnnounce& SearchRequestResultAnnounce::operator=(const SearchRequestAnnounce &_request)
	{
		m_city = _request.m_city;
		m_type = _request.m_type;
		return *this;
	}
};