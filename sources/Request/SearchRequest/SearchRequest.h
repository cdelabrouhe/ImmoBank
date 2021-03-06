#pragma once

#include <string>
#include <vector>
#include "Database\DatabaseManager.h"

namespace ImmoBank
{
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

	enum SearchRequestType
	{
		SearchRequestType_NONE = -1,
		SearchRequestType_Announce,
		SearchRequestType_CityBoroughs,
		SearchRequestType_CityBoroughData,
		SearchRequestType_CityData,
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
		SearchRequest(SearchRequestType _type) : m_requestType(_type) {}
		virtual ~SearchRequest() {}

		SearchRequestType	m_requestType;

		virtual void Init() = 0;
		virtual void Process() {}
		virtual void End() {}

		virtual bool GetResult(std::vector<SearchRequestResult*>& _results) = 0;

		virtual void copyTo(SearchRequest* _target) = 0;

		virtual bool IsAvailable() const = 0;

		virtual SearchRequest* Clone() = 0;
	};
}