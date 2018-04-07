#pragma once

#include "EditableRequest.h"
#include "UI\CitySelector.h"

class EditableRequestAnnounce : public EditableRequest
{
public:
	EditableRequestAnnounce() : EditableRequest(Type_Announce) {}

	virtual void Init(SearchRequest* _request = nullptr) override;
	virtual void Process() override;
	virtual void End() override;
	virtual void Display(unsigned int _ID) override;
	virtual bool IsAvailable() const override;

private:
	virtual void Launch() override;
	virtual void Reset() override;

protected:
	SearchRequestAnnounce				m_searchRequest;
	CitySelector						m_citySelector;
	int									m_requestID = -1;
	bool								m_apartment = true;
	bool								m_house = true;
	bool								m_available = false;
};