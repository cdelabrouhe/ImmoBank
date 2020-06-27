#include "CitySelector.h"
#include "extern/ImGui/imgui.h"
#include "Online/OnlineManager.h"
#include "Tools/StringTools.h"
#include "extern/jsoncpp/reader.h"
#include <time.h>
DISABLE_OPTIMIZE
using namespace ImmoBank;

//--------------------------------------------------------------------------------------------------------------
void CitySelector::_UpdateLogicImmoKeys()
{
	// LogicImmo
	if ((m_logicImmoKeyID > -1) && OnlineManager::getSingleton()->IsHTTPRequestAvailable(m_logicImmoKeyID))
	{
		std::string result;
		OnlineManager::getSingleton()->GetBasicHTTPRequestResult(m_logicImmoKeyID, result);
		m_logicImmoKeyID = -1;
		Json::Value root;
		Json::Reader reader;
		if (reader.parse(result, root))
		{
			// Parse LogicImmo keys
			if (root.isObject())
			{
				Json::Value& items = root["items"];
				unsigned int nbCities = items.size();
				for (unsigned int ID = 0; ID < nbCities; ++ID)
				{
					Json::Value val = items.get(ID, Json::nullValue);
					std::string name = val["name"].asString();
					std::string zipCode = val["postCode"].asString();
					if (zipCode.size() == 0)
						continue;

					int zip = stoi(zipCode);
					zip /= 1000;
					StringTools::TransformToLower(name);
					StringTools::FixName(name);
					StringTools::ConvertToImGuiText(name);
					m_logicImmoKeys[std::make_pair(name, zip)] = val["key"].asString();
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------------------
void CitySelector::_UpdatePapKeys()
{
	// Pap
	if ((m_papKeyID > -1) && OnlineManager::getSingleton()->IsHTTPRequestAvailable(m_papKeyID))
	{
		std::string result;
		OnlineManager::getSingleton()->GetBasicHTTPRequestResult(m_papKeyID, result);
		m_papKeyID = -1;
		Json::Value root;
		Json::Reader reader;
		if (reader.parse(result, root))
		{
			// Parse keys
			if (root.isObject())
			{
				Json::Value& items = root["_embedded"]["place"];
				unsigned int nbCities = items.size();
				for (unsigned int ID = 0; ID < nbCities; ++ID)
				{
					Json::Value val = items.get(ID, Json::nullValue);
					std::string name = val["slug"].asString();
					int zip = -1;
					int delimiter = name.find_last_of("-");
					if (delimiter != std::string::npos)
					{
						std::string zipStr = name.substr(delimiter + 1, name.size());
						zip = stoi(zipStr);
						if (zipStr.size() > 3)
							zip /= 1000;
						name = name.substr(0, delimiter);
					}

					StringTools::ReplaceBadSyntax(name, "-", " ");
					StringTools::TransformToLower(name);
					StringTools::FixName(name);
					StringTools::ConvertToImGuiText(name);
					auto pair = std::make_pair(name, zip);
					auto it = m_papKeys.find(pair);
					if (it == m_papKeys.end())
						m_papKeys[pair] = val["id"].asUInt();
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------------------
void CitySelector::_UpdateCitiesList()
{
	// Get city name list
	if ((m_cityNameRequestID > -1) && OnlineManager::getSingleton()->IsHTTPRequestAvailable(m_cityNameRequestID))
	{
		std::string result;
		OnlineManager::getSingleton()->GetBasicHTTPRequestResult(m_cityNameRequestID, result);
		m_cityNameRequestID = -1;
		StringTools::RemoveEOL(result);
		Json::Value root;
		Json::Reader reader;

		if (reader.parse(result, root))
		{
			// Parse LogicImmo keys
			unsigned int nbMaxCities = 10;
			unsigned int nbCities = root.isArray() ? (root.size() < nbMaxCities ? root.size() : nbMaxCities) : 0;
			for (unsigned int ID = 0; ID < nbCities; ++ID)
			{
				Json::Value val = root.get(ID, Json::nullValue);
				std::string name = val["nom"].asString();
				StringTools::FixName(name);
				StringTools::ConvertToImGuiText(name);
				StringTools::ReplaceBadSyntax(name, "-", " ");
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
				if (m_cities.find(std::make_pair(name, zipCode)) != m_cities.end())
					continue;

				m_cities[std::make_pair(name, zipCode)] = sCity(name, code, zipCode);

				sCityData city;
				city.m_data.m_name = name;
				city.m_data.m_zipCode = zipCode;
				city.m_data.m_inseeCode = code;
				time_t t = time(0);   // get time now
				struct tm* now = localtime(&t);
				int year = 1900 + now->tm_year;
				city.m_timeUpdate.SetDate(year, now->tm_mon, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);

				m_waitingForData.push_back(city);
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------------------
void ImmoBank::CitySelector::_UpdateAsynchronousData()
{
	// Asynchronous update different DB keys
	auto itCity = m_waitingForData.begin();
	while (itCity != m_waitingForData.end())
	{
		sCityData& city = *itCity;
		std::string name = city.m_data.m_name;

		// LogicImmo
		StringTools::TransformToLower(name);
		int zip = city.m_data.m_zipCode / 1000;
		auto itLogicImmo = m_logicImmoKeys.find(std::make_pair(name, zip));
		if (itLogicImmo == m_logicImmoKeys.end())
		{
			++itCity;
			continue;
		}

		// Pap
		auto itPap = m_papKeys.find(std::make_pair(name, zip));
		if (itPap == m_papKeys.end())
		{
			++itCity;
			continue;
		}

		city.m_data.m_logicImmoKey = itLogicImmo->second;
		city.m_data.m_papKey = itPap->second;
		DatabaseManager::getSingleton()->AddCity(city);

		m_waitingForData.erase(itCity);

		m_changed = true;
	}
}

//--------------------------------------------------------------------------------------------------------------
bool CitySelector::Display()
{
	m_changed = false;

	_UpdateLogicImmoKeys();
	_UpdatePapKeys();	
	_UpdateCitiesList();
	_UpdateAsynchronousData();	

	// left
	if (ImGui::InputText("Search city", (char*)m_inputTextCity, 256))
	{
		if (strlen(m_inputTextCity) >= 2)
		{
			// Ask for a city list
			std::string str = m_inputTextCity;
			StringTools::ConvertToImGuiText(str);
			StringTools::RemoveSpecialCharacters(str);
			StringTools::ReplaceBadSyntax(str, " ", "%20");
			std::string request = "https://geo.api.gouv.fr/communes?nom=" + str + "&boost=population";
			if (m_cityNameRequestID > -1)
				OnlineManager::getSingleton()->CancelBasicHTTPRequest(m_cityNameRequestID);
			m_cityNameRequestID = OnlineManager::getSingleton()->SendBasicHTTPRequest(request);

			// LogicImmo
			if (m_logicImmoKeyID > -1)
			{
				OnlineManager::getSingleton()->CancelBasicHTTPRequest(m_logicImmoKeyID);
				m_logicImmoKeyID = -1;
			}

			sCityData data;
			DatabaseManager::getSingleton()->GetCityData(str, -1, data);
			if (data.m_data.m_logicImmoKey.empty())
			{
				request = BoroughData::ComputeLogicImmoKeyURL(str);
				m_logicImmoKeyID = OnlineManager::getSingleton()->SendBasicHTTPRequest(request, true);
			}
			else
				m_logicImmoKeys[std::make_pair(str, -1)] = data.m_data.m_logicImmoKey;

			// Pap
			if (m_papKeyID > -1)
			{
				OnlineManager::getSingleton()->CancelBasicHTTPRequest(m_papKeyID);
				m_papKeyID = -1;
			}

			if (data.m_data.m_papKey == 0)
			{
				request = BoroughData::ComputePapKeyURL(str);
				m_papKeyID = OnlineManager::getSingleton()->SendBasicHTTPRequest(request, true);
			}
			else
				m_papKeys[std::make_pair(str, -1)] = data.m_data.m_papKey;
		}
		m_changed = true;
	}

	std::string citiesStr[100];
	const char* cities[100];
	const int nbCities = m_cities.size() > 100 ? 100 : m_cities.size();
	auto it = m_cities.begin();
	int ID = 0;
	while (ID < nbCities)
	{
		citiesStr[ID++] = it->second.m_name + " (" + std::to_string(it->second.m_zipCode) + ")";
		++it;
	}

	ID = 0;
	while (ID < nbCities)
	{
		cities[ID] = citiesStr[ID++].c_str();
	}

	if (m_displayAllResults && m_cities.size() > 0)
	{
		if (ImGui::Combo("City name", &m_selectedCityID, cities, (int)m_cities.size()))
		{
			std::string str = cities[m_selectedCityID];
			int size = str.size() < 256 ? (int)str.size() : 256;
			char* dest = m_inputTextCity;
			const char* source = str.c_str();
			memcpy(dest, source, size);
		}

		return m_selectedCityID > -1;
	}

	return false;
}