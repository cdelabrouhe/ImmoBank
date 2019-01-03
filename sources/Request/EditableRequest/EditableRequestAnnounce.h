#pragma once

#include "EditableRequest.h"
#include "UI\CitySelector.h"
#include "Request/SearchRequest/SearchRequestAnnounce.h"

namespace ImmoBank
{
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

		void RecomputeBoroughSelections();

	protected:
		SearchRequestAnnounce				m_searchRequest;
		CitySelector						m_citySelector;
		int									m_requestID = -1;
		bool								m_apartment = true;
		bool								m_house = true;
		bool								m_available = false;

		std::vector<sCity>					m_cities;
		BoroughData							m_borough;
		std::vector<BoroughData>			m_boroughList;
		std::vector<int>					m_selectionIDs;
		std::vector<int>					m_subSelections;
		char								m_inputTextCity[256];
		int									m_selectedCityID = 0;
		int									m_selectedBoroughID = 0;
		bool								m_updateList = true;
	};
}