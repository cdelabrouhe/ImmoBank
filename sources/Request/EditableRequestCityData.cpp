#include "EditableRequestCityData.h"
#include <windows.h>
#include <shellapi.h>

#include "RequestManager.h"
#include "Online/OnlineManager.h"

#include "extern/ImGui/imgui.h"
#include "extern/jsoncpp/reader.h"
#include "Tools/StringTools.h"
#include "Database/DatabaseManager.h"
#include <time.h>
#include "SearchResult.h"

void EditableRequestCityData::Init(SearchRequest* _request)
{
	
}

void EditableRequestCityData::Process()
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

void EditableRequestCityData::End()
{
	Reset();
}

bool EditableRequestCityData::IsAvailable() const
{
	return m_available;
}

void EditableRequestCityData::Reset()
{
	if (m_requestID > -1)
		OnlineManager::getSingleton()->DeleteRequest(m_requestID);

	m_requestID = -1;
	m_available = false;
	for (auto result : m_result)
		delete result;
	m_result.clear(); 
}

void EditableRequestCityData::Launch()
{
	Reset();
	m_requestID = OnlineManager::getSingleton()->SendRequest(&m_searchRequest);
}

void EditableRequestCityData::Display(unsigned int _ID)
{
	std::string name = "Request##" + std::to_string(_ID);

	ImGui::SetNextWindowSize(ImVec2(900, 500), ImGuiCond_FirstUseEver);
	ImGui::Begin(name.c_str());

	// City selector process
	ImGui::BeginChild("Search request", ImVec2(300, 0), true);
	if (m_citySelector.Display())
		m_searchRequest.m_city = *m_citySelector.GetSelectedCity();
	
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
			for (size_t ID = 0; ID < m_result.size(); ++ID)
			{
				auto result = m_result[ID];
				result->Display();
			}
		}
	}
	// No request launched yet
	else
		ImGui::Text("No request sent");

	ImGui::EndChild();

	ImGui::End();
}