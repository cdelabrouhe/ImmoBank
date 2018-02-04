#pragma once

#include <string>

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

struct SearchRequest
{
	sCity		m_city;
	Type		m_type = Type_NONE;
	Category	m_category = Category_NONE;
	int			m_priceMin = 0;
	int			m_priceMax = 0;
	int			m_surfaceMin = 0;
	int			m_surfaceMax = 0;
	int			m_nbRooms = 0;
	int			m_nbBedRooms = 0;
};

struct SearchRequestResult
{
	SearchRequestResult() {}
	SearchRequestResult(SearchRequest& _request)
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

	SearchRequestResult& SearchRequestResult::operator=(const SearchRequest &_request)
	{
		m_city = _request.m_city;
		m_type = _request.m_type;
		m_category = _request.m_category;
		return *this;
	}
};