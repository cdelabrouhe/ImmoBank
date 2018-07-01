#pragma once

#include "SearchRequest.h"

struct ImGuiTextFilter;
struct SearchRequestResult
{
	SearchRequestResult(SearchRequestType _type) : m_resultType(_type) {}

	SearchRequestType m_resultType;

	virtual void PostProcess() {}
	virtual void Display(ImGuiTextFilter* _filter = nullptr) {}
};