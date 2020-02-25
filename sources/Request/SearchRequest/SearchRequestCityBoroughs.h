#pragma once

#include "SearchRequestResulCityBorough.h"

namespace ImmoBank
{
	struct SearchRequestCityBoroughs : public SearchRequest
	{
		enum State
		{
			State_NONE = -1,
			State_GetRawList,
			State_CheckSeLoger,
			State_CheckLogicImmo,
			State_CheckPap,
			State_DONE,
			Stat_COUNT
		};

		SearchRequestCityBoroughs() : SearchRequest(SearchRequestType_CityBoroughs) {}
		virtual ~SearchRequestCityBoroughs() {}

		virtual void Init() override;
		virtual void Process() override;

		void SwitchState(State _state);
		virtual void copyTo(SearchRequest* _target) override;
		virtual bool IsAvailable() const;

		virtual bool GetResult(std::vector<SearchRequestResult*>& _results) override;

		virtual SearchRequest* Clone() { return new SearchRequestCityBoroughs(); }

		sCity					m_city;

	private:
		std::vector<int>			m_httpRequestsID;
		std::vector<SearchRequestResulCityBorough>	m_boroughs;
		State	m_state = State_NONE;
	};
}