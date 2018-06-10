#pragma once

#include "SearchRequest.h"

struct SearchRequestCityBoroughData : public SearchRequest
{
	SearchRequestCityBoroughData() : SearchRequest(SearchRequestType_CityBoroughData) {}
	virtual ~SearchRequestCityBoroughData() {}

	virtual void Init() override;

	virtual void copyTo(SearchRequest* _target) override;
	virtual bool IsAvailable() const;

	virtual bool GetResult(std::vector<SearchRequestResult*>& _results) override;

	virtual SearchRequest* Clone() { return new SearchRequestCityBoroughData(); }

	sCity			m_city;
	BoroughData		m_data;

private:
	int				m_httpRequestsID = -1;
};