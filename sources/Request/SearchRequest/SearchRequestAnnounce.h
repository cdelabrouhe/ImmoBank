#pragma once

#include "SearchRequest.h"

namespace ImmoBank
{
	class OnlineDatabase;
	struct SearchRequestAnnounce : public SearchRequest
	{
		SearchRequestAnnounce() : SearchRequest(SearchRequestType_Announce) {}
		virtual ~SearchRequestAnnounce() {}

		virtual void Init() override;
		virtual void End() override;

		virtual void copyTo(SearchRequest* _target) override;
		virtual bool IsAvailable() const override;

		virtual bool GetResult(std::vector<SearchRequestResult*>& _results) override;

		virtual SearchRequest* Clone() override { return new SearchRequestAnnounce(); }

		void AddBorough(BoroughData& _data);
		void RemoveBorough(const std::string& _name);
		bool HasBorough(const std::string& _name);

		sCity						m_city;
		std::vector<BoroughData>	m_boroughList;
		Type						m_type = Type_NONE;
		std::vector<Category>		m_categories;
		int							m_priceMin = 0;
		int							m_priceMax = 0;
		int							m_surfaceMin = 0;
		int							m_surfaceMax = 0;
		int							m_nbRoomsMin = 0;
		int							m_nbRoomsMax = 0;
		int							m_nbBedRoomsMin = 0;
		int							m_nbBedRoomsMax = 0;

	private:
		std::vector<BoroughData>	m_boroughs;
		std::vector<std::pair<OnlineDatabase*, int>>	m_internalRequests;
	};
}