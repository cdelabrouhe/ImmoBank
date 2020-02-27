#pragma once

#include <vector>
#include "Request/SearchRequest/SearchRequest.h"

namespace ImmoBank
{
	class CitySelector
	{
	public:
		CitySelector() {}

		bool Display();
		char* GetText() { return m_inputTextCity; }
		inline bool HasChanged() const { return m_changed; }
		inline void SetDisplayAllResults(bool _state) { m_displayAllResults = _state; }

	private:
		std::map<std::string, sCity>		m_cities;
		std::map<std::string, std::string>	m_logicImmoKeys;
		std::map<std::string, unsigned int>	m_papKeys;
		std::vector<sCityData>				m_waitingForData;
		char								m_inputTextCity[256];
		int									m_selectedCityID = 0;
		int									m_cityNameRequestID = -1;
		int									m_logicImmoKeyID = -1;
		int									m_papKeyID = -1;
		bool								m_changed = false;
		bool								m_displayAllResults = true;
	};
}