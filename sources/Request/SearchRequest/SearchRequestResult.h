#pragma once

#include "SearchRequest.h"
#include "Tools\Tools.h"

struct ImGuiTextFilter;

namespace ImmoBank
{
	struct SearchRequestResult
	{
		SearchRequestResult(SearchRequestType _type) : m_resultType(_type) {}

		virtual ~SearchRequestResult() {}

		SearchRequestType m_resultType;

		virtual void Init()	{}
		virtual void End()	{}

		virtual void PostProcess() {}
		virtual bool Display(ImGuiTextFilter* _filter = nullptr) { return true; }

		static bool compare(const SearchRequestResult* _a, const SearchRequestResult* _b, Tools::SortType _sortType)
		{
			return _a->Compare(_b, _sortType);
		}

		virtual bool Compare(const SearchRequestResult* _target, Tools::SortType _sortType) const
		{
			return false;
		}
	};
}