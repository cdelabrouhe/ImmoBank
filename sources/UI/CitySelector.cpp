#include "CitySelector.h"
#include "extern/ImGui/imgui.h"
#include "Online/OnlineManager.h"
#include "Tools/StringTools.h"
#include "extern/jsoncpp/reader.h"
#include <time.h>

using namespace ImmoBank;

bool CitySelector::Display()
{
	m_changed = false;

	// Get city name list
	if ((m_cityNameRequestID > -1) && OnlineManager::getSingleton()->IsBasicHTTPRequestAvailable(m_cityNameRequestID))
	{
		m_cities.clear();
		std::string result;
		OnlineManager::getSingleton()->GetBasicHTTPRequestResult(m_cityNameRequestID, result);
		m_cityNameRequestID = -1;
		StringTools::RemoveEOL(result);
		Json::Value root;
		Json::Reader reader;
		if (reader.parse(result, root))
		{
			unsigned int nbMaxCities = 10;
			unsigned int nbCities = root.size() < nbMaxCities ? root.size() : nbMaxCities;
			for (unsigned int ID = 0; ID < nbCities; ++ID)
			{
				Json::Value& val = root.get(ID, Json::nullValue);
				std::string name = val["nom"].asString();
				StringTools::FixName(name);
				StringTools::ReplaceBadSyntax(name, "é", "e");
				StringTools::ReplaceBadSyntax(name, "è", "e");
				StringTools::ReplaceBadSyntax(name, "É", "E");
				StringTools::ReplaceBadSyntax(name, "î", "i");
				std::string codeStr = val["code"].asString();
				std::string zipCodeStr = val["codesPostaux"].get(0u, Json::nullValue).asString();

				// Replace 001 by 000
				auto it = zipCodeStr.find("001");
				if (it != std::string::npos)
					StringTools::ReplaceBadSyntax(zipCodeStr, "001", "000");

				int code = std::stoi(codeStr);
				if (zipCodeStr.empty())
					continue;

				int zipCode = std::stoi(zipCodeStr);
				m_cities.push_back(sCity(name, code, zipCode));

				sCityData city;
				city.m_data.m_name = name;
				city.m_data.m_zipCode = zipCode;
				city.m_data.m_inseeCode = code;
				time_t t = time(0);   // get time now
				struct tm * now = localtime(&t);
				int year = 1900 + now->tm_year;
				city.m_timeUpdate.SetDate(year, now->tm_mon, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
				DatabaseManager::getSingleton()->AddCity(city);
			}
		}
	}

	// left
	if (ImGui::InputText("Search city", (char*)m_inputTextCity, 256))
	{
		if (strlen(m_inputTextCity) >= 2)
		{
			// Ask for a city list
			std::string str = m_inputTextCity;
			StringTools::ReplaceBadSyntax(str, "é", "e");
			StringTools::ReplaceBadSyntax(str, "è", "e");
			StringTools::ReplaceBadSyntax(str, "ê", "e");
			StringTools::ReplaceBadSyntax(str, "É", "E");
			StringTools::ReplaceBadSyntax(str, "î", "i");
			std::string request = "https://geo.api.gouv.fr/communes?nom=" + str + "&boost=population";
			if (m_cityNameRequestID > -1)
				OnlineManager::getSingleton()->CancelBasicHTTPRequest(m_cityNameRequestID);
			m_cityNameRequestID = OnlineManager::getSingleton()->SendBasicHTTPRequest(request);
		}
		m_changed = true;
	}

	if (m_cities.size() > 100)
		m_cities.resize(100);

	const char* cities[100];
	for (size_t ID = 0; ID < m_cities.size(); ++ID)
		cities[ID] = m_cities[ID].m_name.c_str();

	if (m_displayAllResults && m_cities.size() > 0)
	{
		if (ImGui::Combo("City name", &m_selectedCityID, cities, (int)m_cities.size()))
		{
			std::string str = m_cities[m_selectedCityID].m_name;
			int size = str.size() < 256 ? (int)str.size() : 256;
			char* dest = m_inputTextCity;
			const char* source = str.c_str();
			memcpy(dest, source, size);
		}

		return m_selectedCityID > -1;
	}

	return false;
}

const sCity* CitySelector::GetSelectedCity() const
{
	if (m_selectedCityID > -1)
		return &m_cities[m_selectedCityID];
	return nullptr;
}
