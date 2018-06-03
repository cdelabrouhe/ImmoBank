#pragma once

#include "SearchRequest.h"

struct SearchRequestCityBoroughs : public SearchRequest
{
	SearchRequestCityBoroughs() : SearchRequest(SearchRequestType_CityBoroughs) {}
	virtual ~SearchRequestCityBoroughs() {}

	virtual void Init() override;

	virtual void copyTo(SearchRequest* _target) override;
	virtual bool IsAvailable() const;

	virtual bool GetResult(std::vector<SearchRequestResult*>& _results) override;

	virtual SearchRequest* Clone() { return new SearchRequestCityBoroughs(); }

	sCity					m_city;

private:
	std::vector<int>		m_httpRequestsID;
};
