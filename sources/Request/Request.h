#pragma once

#include <vector>
#include "SearchRequest.h"

class Request
{
public:
	void Init(SearchRequest* _request = nullptr);
	void Process();
	void End();

	void Display();

	bool IsAvailable() const;
	void GetResults(std::vector<SearchRequestResult>& _results) { _results = m_result; }

private:
	void Launch();
	void Reset();

protected:
	SearchRequest						m_searchRequest;
	std::vector<SearchRequestResult>	m_result;
	std::vector<sCity>					m_cities;
	char								m_inputTextCity[256];
	int									m_selectedCityID = 0;
	int									m_requestID = -1;
	int									m_cityNameRequestID = -1;
	bool								m_available = false;
};