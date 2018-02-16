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

struct SearchRequestResult;
struct SearchRequest
{
	SearchRequest(SearchRequestType _type)	: m_requestType(_type)	{}
	virtual ~SearchRequest() {}

	SearchRequestType	m_requestType;

	virtual void Init() = 0;
	virtual void Process()		{}
	virtual void End()			{}

	virtual bool GetResult(std::vector<SearchRequestResult*>& _results) = 0;

	virtual void copyTo(SearchRequest* _target) = 0;

	virtual bool IsAvailable() const = 0;
};

class OnlineDatabase;
struct SearchRequestAnnounce : public SearchRequest
{
	SearchRequestAnnounce() : SearchRequest(SearchRequestType_Announce)	{}
	virtual ~SearchRequestAnnounce() {}

	virtual void Init() override;
	virtual void Process() override;
	virtual void End() override;

	virtual void copyTo(SearchRequest* _target) override;
	virtual bool IsAvailable() const override;

	virtual bool GetResult(std::vector<SearchRequestResult*>& _results) override;

	sCity					m_city;
	Type					m_type = Type_NONE;
	std::vector<Category>	m_categories;
	int						m_priceMin = 0;
	int						m_priceMax = 0;
	int						m_surfaceMin = 0;
	int						m_surfaceMax = 0;
	int						m_nbRooms = 0;
	int						m_nbBedRooms = 0;

private:
	std::vector<std::string>	m_boroughs;
	std::vector<std::pair<OnlineDatabase*, int>>	m_internalRequests;
	int		m_boroughsRequestID = -1;
};

struct SearchRequestCityBoroughs : public SearchRequest
{
	SearchRequestCityBoroughs() : SearchRequest(SearchRequestType_CityBoroughs) {}
	virtual ~SearchRequestCityBoroughs() {}

	virtual void Init() override;
	
	virtual void copyTo(SearchRequest* _target) override;
	virtual bool IsAvailable() const;

	virtual bool GetResult(std::vector<SearchRequestResult*>& _results) override;

	sCity					m_city;

private:
	int		m_httpRequestID = -1;
};

struct SearchRequestResult
{
	SearchRequestResult(SearchRequestType _type) : m_resultType(_type)	{}

	SearchRequestType m_resultType;

	virtual void PostProcess()	{}
	virtual void Display()		{}
};

struct SearchRequestResulCityBorough : public SearchRequestResult
{
	SearchRequestResulCityBorough() : SearchRequestResult(SearchRequestType_CityBoroughs) {}
	SearchRequestResulCityBorough(SearchRequestAnnounce& _request) : SearchRequestResult(SearchRequestType_CityBoroughs)
	{
		*this = _request;
	}

	std::string m_name;
};

struct SearchRequestResultAnnounce : public SearchRequestResult
{
	SearchRequestResultAnnounce() : SearchRequestResult(SearchRequestType_Announce)	{}
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