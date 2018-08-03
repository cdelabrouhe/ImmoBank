#include "EditableRequestAnnounce.h"
#include <windows.h>
#include <shellapi.h>

#include "Request/RequestManager.h"
#include "Online/OnlineManager.h"

#include "extern/ImGui/imgui.h"
#include "extern/jsoncpp/reader.h"
#include "Tools/StringTools.h"
#include "Database/DatabaseManager.h"
#include <time.h>
#include "Request/SearchRequest/SearchRequestResult.h"
#include <algorithm>

void EditableRequestAnnounce::Init(SearchRequest* _request)
{
	if (_request)
	{
		switch (_request->m_requestType)
		{
		case SearchRequestType_Announce:
		{
			m_searchRequest = *(SearchRequestAnnounce*)_request;
		}
		break;

		default:
			//	Not supposed to manage anything else
			break;
		}
	}
}

void EditableRequestAnnounce::Process()
{
	if ((m_requestID > -1) && !m_available)
	{
		m_available = OnlineManager::getSingleton()->IsRequestAvailable(m_requestID);
		if (m_available)
		{
			OnlineManager::getSingleton()->GetRequestResult(m_requestID, m_result);

			for (auto result : m_result)
			{
				result->PostProcess();
			}
		}
	}
}

void EditableRequestAnnounce::End()
{
	Reset();
}

bool EditableRequestAnnounce::IsAvailable() const
{
	return m_available;
}

void EditableRequestAnnounce::Reset()
{
	if (m_requestID > -1)
		OnlineManager::getSingleton()->DeleteRequest(m_requestID);

	m_requestID = -1;
	m_available = false;
	for (auto result : m_result)
		delete result;
	m_result.clear();
}

void EditableRequestAnnounce::Launch()
{
	Reset();
	m_requestID = OnlineManager::getSingleton()->SendRequest(&m_searchRequest);
}

void EditableRequestAnnounce::Display(unsigned int _ID)
{
	std::string name = "Request##" + std::to_string(_ID);

	ImGui::SetNextWindowSize(ImVec2(900, 500), ImGuiCond_FirstUseEver);
	ImGui::Begin(name.c_str());

	// City selector process
	ImGui::BeginChild("Search request", ImVec2(300, 0), true);

	if (ImGui::InputText("Search city", (char*)m_inputTextCity, 256))
	{
		if (strlen(m_inputTextCity) > 0)
		{
			// Ask for a city list
			m_cities.clear();
			std::string str = m_inputTextCity;
			DatabaseManager::getSingleton()->ListAllCitiesWithFilter(m_cities, str);
		}
	}

	if (m_cities.size() > 100)
		m_cities.resize(100);

	const char* cities[100];
	for (size_t ID = 0; ID < m_cities.size(); ++ID)
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

		if ((m_selectedCityID > -1) && (m_selectedCityID < m_cities.size()))
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
			ImGui::SameLine();
			static ImGuiTextFilter filter;
			filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
			std::vector<SearchRequestResult*> toRemove;
			for (size_t ID = 0; ID < m_result.size(); ++ID)
			{
				auto request = m_result[ID];
				if (!request->Display(&filter))
					toRemove.push_back(request);
			}

			for (auto request : toRemove)
			{
				auto it = std::find_if(m_result.begin(), m_result.end(), [request](SearchRequestResult* _request)->bool
				{
					return _request == request;
				});

				if (it != m_result.end())
					m_result.erase(it);

				delete request;
			}
		}
	}
	// No request launched yet
	else
		ImGui::Text("No request sent");

	ImGui::EndChild();

	ImGui::End();
}