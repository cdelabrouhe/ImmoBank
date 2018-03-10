#pragma once

#include "EditableRequest.h"

class EditableRequestCityData : public EditableRequest
{
public:
	EditableRequestCityData() : EditableRequest(Type_CityData) {}

	virtual void Init(SearchRequest* _request = nullptr) override;
	virtual void Process() override;
	virtual void End() override;
	virtual void Display(unsigned int _ID) override;
	virtual bool IsAvailable() const override;

private:
	virtual void Launch() override;
	virtual void Reset() override;

protected:
	SearchRequestCityData				m_searchRequest;
	std::vector<sCity>					m_cities;
	char								m_inputTextCity[256];
	int									m_selectedCityID = 0;
	int									m_requestID = -1;
	int									m_cityNameRequestID = -1;
	bool								m_available = false;
};