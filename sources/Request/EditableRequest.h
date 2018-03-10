#pragma once

#include <vector>
#include "SearchRequest.h"

class EditableRequest
{
public:
	enum Type
	{
		Type_NONE = -1,
		Type_Announce,
		Type_CityData,
		Type_COUNT
	};

public:
	EditableRequest(Type _type) : m_type(_type)	{}

	virtual void Init(SearchRequest* _request = nullptr) = 0;
	virtual void Process() = 0;
	virtual void End() = 0;
	virtual void Display(unsigned int _ID) = 0;
	virtual bool IsAvailable() const = 0;
	virtual void GetResults(std::vector<SearchRequestResult*>& _results) { _results = m_result; }

private:
	virtual void Launch() = 0;
	virtual void Reset() = 0;

protected:
	Type								m_type;
	std::vector<SearchRequestResult*>	m_result;
};