#pragma once

#include <string>
#include <vector>

enum Type
{
	Type_NONE = -1,
	Type_Rent,
	Type_Buy,
	Type_COUNT
};

enum Category
{
	Category_NONE = -1,
	Category_House,
	Category_Apartment,
	Category_COUNT
};

struct sCity
{
	sCity(const std::string& _name = "", int _codeInsee = 0, int _zipCode = 0) : m_name(_name), m_inseeCode(_codeInsee), m_zipCode(_zipCode)	{}
	std::string m_name;
	int			m_inseeCode;
	int			m_zipCode;
};

enum SearchRequestType
{
	SearchRequestType_NONE = -1,
	SearchRequestType_Announce,
	SearchRequestType_CityBoroughs,
	SearchRequestType_PriceMin,
	SearchRequestType_Price,
	SearchRequestType_PriceMax,
	SearchRequestType_RentMin,
	SearchRequestType_Rent,
	SearchRequestType_RentMax,
	SearchRequestType_COUNT
};

struct SearchRequest
{
	SearchRequestType	m_requestType;

	virtual void copyTo(SearchRequest* _target)
	{
		_target->m_requestType = m_requestType;
	}
};

struct SearchRequestAnnounce : public SearchRequest
{
	sCity					m_city;
	Type					m_type = Type_NONE;
	std::vector<Category>	m_categories;
	int						m_priceMin = 0;
	int						m_priceMax = 0;
	int						m_surfaceMin = 0;
	int						m_surfaceMax = 0;
	int						m_nbRooms = 0;
	int						m_nbBedRooms = 0;

	virtual void copyTo(SearchRequestAnnounce* _target)
	{
		SearchRequest::copyTo(_target);
		_target->m_city = m_city;
		_target->m_type = m_type;
		_target->m_categories = m_categories;
		_target->m_priceMin = m_priceMin;
		_target->m_priceMax = m_priceMax;
		_target->m_surfaceMin = m_surfaceMin;
		_target->m_surfaceMax = m_surfaceMax;
		_target->m_nbRooms = m_nbRooms;
		_target->m_nbBedRooms = m_nbBedRooms;
	}
};

struct SearchRequestResult
{
	SearchRequestType m_resultType;

	virtual void PostProcess()	{}
	virtual void Display()		{}
};

struct SearchRequestResultAnnounce : public SearchRequestResult
{
	SearchRequestResultAnnounce() {}
	SearchRequestResultAnnounce(SearchRequestAnnounce& _request)
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