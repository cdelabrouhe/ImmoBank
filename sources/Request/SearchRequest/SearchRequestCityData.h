#pragma once

#include "SearchRequest.h"

struct SearchRequestCityData : public SearchRequest
{
	enum UpdateStep
	{
		UpdateStep_NONE = -1,
		UpdateStep_GetCityData,
		UpdateStep_GetBoroughList,
		UpdateStep_ComputeBoroughsPrices,
		UpdateStep_COUNT
	};

	SearchRequestCityData() : SearchRequest(SearchRequestType_CityData) {}
	virtual ~SearchRequestCityData() {}

	virtual void Init() override;
	virtual void Process() override;
	virtual void End() override;

	virtual void copyTo(SearchRequest* _target) override;
	virtual bool IsAvailable() const;

	virtual bool GetResult(std::vector<SearchRequestResult*>& _results) override;

	virtual SearchRequest* Clone() { return new SearchRequestCityData(); }

private:
	void InitBoroughPricesRequest();

public:
	sCity					m_city;

private:
	std::vector<int>			m_httpRequestsID;
	std::vector<BoroughData>	m_boroughs;
	int							m_boroughsRequestID = -1;

private:
	void IncreaseStep() { m_state = (UpdateStep)((int)m_state + 1); }

private:
	UpdateStep		m_state = UpdateStep_NONE;
};