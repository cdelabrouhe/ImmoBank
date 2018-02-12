#include "Request.h"
#include <windows.h>
#include <shellapi.h>

#include "RequestManager.h"
#include "Database/DatabaseManager.h"

#include "extern/ImGui/imgui.h"
#include "extern/jsoncpp/reader.h"
#include "Tools/StringTools.h"

void Request::Init(SearchRequestAnnounce* _request)
{
	if (_request)
		m_searchRequest = *_request;
}

void Request::Process()
{
	if ((m_requestID > -1) && !m_available)
	{
		m_available = DatabaseManager::getSingleton()->IsRequestAvailable(m_requestID);
		if (m_available)
		{
			DatabaseManager::getSingleton()->GetRequestResult(m_requestID, m_result);

			for (auto result : m_result)
			{
				result->PostProcess();
			}
		}
	}
}

void Request::End()
{
	if (m_requestID > -1)
		DatabaseManager::getSingleton()->DeleteRequest(m_requestID);

	m_requestID = -1;
	m_available = false;
}

bool Request::IsAvailable() const
{
	return m_available;
}

void Request::Reset()
{
	if (m_requestID > -1)
		DatabaseManager::getSingleton()->DeleteRequest(m_requestID);

	m_requestID = -1;
	m_available = false;
	for (auto result : m_result)
		delete result;
	m_result.clear();
}

void Request::Launch()
{
	Reset();
	m_searchRequest.m_requestType = SearchRequestType_Announce;
	m_requestID = DatabaseManager::getSingleton()->SendRequest(&m_searchRequest);
}

void Request::Display()
{
	unsigned int ID = *(int*)this;
	std::string name = "Request##" + std::to_string(ID);

	ImGui::SetNextWindowSize(ImVec2(900, 500), ImGuiCond_FirstUseEver);
	ImGui::Begin(name.c_str());

	// Get city name list
	if ((m_cityNameRequestID > -1) && DatabaseManager::getSingleton()->IsBasicHTTPRequestAvailable(m_cityNameRequestID))
	{
		m_cities.clear();
		std::string result;
		DatabaseManager::getSingleton()->GetBasicHTTPRequestResult(m_cityNameRequestID, result);
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
				std::string codeStr = val["code"].asString();
				std::string zipCodeStr = val["codesPostaux"].get(0u, Json::nullValue).asString();
				int code = std::stoi(codeStr);
				int zipCode = std::stoi(zipCodeStr);
				m_cities.push_back(sCity(name, code, zipCode));
			}
		}
	}

	// left
	static int selected = 0;
	ImGui::BeginChild("Search request", ImVec2(300, 0), true);
	if (ImGui::InputText("Search city", (char*)m_inputTextCity, 256))
	{
		//if (strlen(m_inputTextCity) >= 3)
		{
			// Ask for a city list
			std::string request = "https://geo.api.gouv.fr/communes?nom=" + std::string(m_inputTextCity);
			if (m_cityNameRequestID > -1)
				DatabaseManager::getSingleton()->CancelBasicHTTPRequest(m_cityNameRequestID);
			m_cityNameRequestID = DatabaseManager::getSingleton()->SendBasicHTTPRequest(request);
		}
	}

	if (m_cities.size() > 100)
		m_cities.resize(100);

	const char* cities[100];
	for (auto ID = 0; ID < m_cities.size(); ++ID)
		cities[ID] = m_cities[ID].m_name.c_str();

	if (m_cities.size() > 0)
	{
		if (ImGui::Combo("City name", &m_selectedCityID, cities, (int)m_cities.size()))
		{
			std::string str = m_cities[m_selectedCityID].m_name;
			int size = str.size() < 256 ? (int)str.size() : 256;
			char* dest = m_inputTextCity;
			const char* source = str.c_str();
			memcpy(dest, source, size);
		}

		if (m_selectedCityID > -1)
			m_searchRequest.m_city = m_cities[m_selectedCityID];
	}


	if (ImGui::Checkbox("Appartement", &m_apartment))
	{
		auto it = std::find(m_searchRequest.m_categories.begin(), m_searchRequest.m_categories.end(), Category_Apartment);
		if (m_apartment)
		{
			if (it == m_searchRequest.m_categories.end())
				m_searchRequest.m_categories.push_back(Category_Apartment);
		}
		else
		{
			if (it != m_searchRequest.m_categories.end())
				m_searchRequest.m_categories.erase(it);
		}
	}
	if (ImGui::Checkbox("House", &m_house))
	{
		auto it = std::find(m_searchRequest.m_categories.begin(), m_searchRequest.m_categories.end(), Category_House);
		if (m_house)
		{
			if (it == m_searchRequest.m_categories.end())
				m_searchRequest.m_categories.push_back(Category_House);
		}
		else
		{
			if (it != m_searchRequest.m_categories.end())
				m_searchRequest.m_categories.erase(it);
		}
	}

	ImGui::SliderInt("Nb rooms", &m_searchRequest.m_nbRooms, 1, 20);
	ImGui::SliderInt("Nb bedrooms", &m_searchRequest.m_nbBedRooms, 1, 20);
	ImGui::InputInt("Surface min", &m_searchRequest.m_surfaceMin);
	ImGui::InputInt("Surface max", &m_searchRequest.m_surfaceMax);
	ImGui::InputInt("Price min", &m_searchRequest.m_priceMin);
	ImGui::InputInt("Price max", &m_searchRequest.m_priceMax);

	if (ImGui::Button("Launch"))
		Launch();

	ImGui::SameLine();
	if (ImGui::Button("Cancel"))
		RequestManager::getSingleton()->AskForDeleteRequest(this);

	ImGui::EndChild();
	ImGui::SameLine();

	// right
	ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing())); // Leave room for 1 line below us

	// Request available
	if (m_requestID > -1)
	{
		ImGui::Separator();
		if (!m_available)
		{
			char buf[128];
			sprintf_s(buf, "Searching... %c", "|/-\\"[(int)(ImGui::GetTime() / 0.1f) & 3]);
			ImGui::Text(buf);
		}
		// Display results
		else
		{
			ImGui::Text("%u results available", m_result.size());
			for (auto ID = 0; ID < m_result.size(); ++ID)
			{
				auto request = m_result[ID];
				request->Display();
			}
		}
	}
	// No request launched yet
	else
		ImGui::Text("No request sent");

	ImGui::EndChild();

	ImGui::End();
}