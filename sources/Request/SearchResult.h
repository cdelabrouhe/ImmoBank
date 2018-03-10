#pragma once

#include "SearchRequest.h"

struct SearchRequestResult
{
	SearchRequestResult(SearchRequestType _type) : m_resultType(_type) {}

	SearchRequestType m_resultType;

	virtual void PostProcess() {}
	virtual void Display() {}
};

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
	int			m_price = 0;
	float		m_surface = 0.f;
	int			m_nbRooms = 0;
	int			m_nbBedRooms = 0;

	virtual void PostProcess() override;
	virtual void Display() override;

	SearchRequestResultAnnounce& SearchRequestResultAnnounce::operator=(const SearchRequestAnnounce &_request)
	{
		m_city = _request.m_city;
		m_type = _request.m_type;
		return *this;
	}
};