#include "SearchRequestResultAnnounce.h"
#include "Tools\StringTools.h"
#include "extern/ImGui/imgui.h"
#include <windows.h>
#include <shellapi.h>
#include <algorithm>
#include "Tools\Tools.h"
#include "Text\TextManager.h"
#include <Online\OnlineManager.h>
#include <GL\ProdToolGL.h>

using namespace ImmoBank;

void SearchRequestResultAnnounce::Init()
{
	if (!m_imageURL.empty())
		m_imageDownloadRequestID = OnlineManager::getSingleton()->SendBinaryHTTPRequest(m_imageURL);
}

void SearchRequestResultAnnounce::End()
{
	if (m_imageTextureID > 0)
		ProdToolGL_DeleteTexture(&m_imageTextureID);
	m_imageTextureID = 0;
}

void SearchRequestResultAnnounce::PostProcess()
{
	StringTools::RemoveSpecialCharacters(m_name);
	StringTools::RemoveSpecialCharacters(m_description);

	UpdateBoroughs();
}

void SearchRequestResultAnnounce::UpdateBoroughs()
{
	DatabaseManager::getSingleton()->GetBoroughs(m_city, m_boroughs);

	std::sort(m_boroughs.begin(), m_boroughs.end(), BoroughData::compare);

	std::string description = m_description;
	StringTools::TransformToLower(description);

	bool found = false;
	int ID = 0;
	for (auto& borough : m_boroughs)
	{
		int selogerKey = borough.GetSelogerKey();
		if (selogerKey == m_inseeCode)
		{
			m_selectedBorough = borough;
			m_selectedBoroughID = ID + 1;
			found = true;
			break;
		}

		++ID;
	}

	if (!found)
	{
		ID = 0;
		for (auto& borough : m_boroughs)
		{
			std::string name = borough.m_name;
			StringTools::TransformToLower(name);
			auto findIndex = description.find(name);
			if (findIndex != std::string::npos)
			{
				m_selectedBorough = borough;
				m_selectedBoroughID = ID + 1;
				break;
			}
			else
			{
				if ((m_selectedBoroughID == 0) && (borough.m_name == s_wholeCityName))
					m_selectedBoroughID = ID + 1;
			}

			++ID;
		}
	}
}

float SearchRequestResultAnnounce::GetRentabilityRate() const
{
	return Tools::ComputeRentabilityRate(GetEstimatedRent(), (float)m_price);
}

float SearchRequestResultAnnounce::GetEstimatedRent() const
{
	if (m_selectedBoroughID > 0)
	{
		const BoroughData& borough = m_boroughs[m_selectedBoroughID - 1];
		if (!m_waitingForDBUpdate && !DatabaseManager::getSingleton()->IsBoroughUpdating(borough))
		{
			float rent = 0.f;
			if (m_nbRooms == 1)
				rent = m_surface * borough.m_priceRentApartmentT1.m_val;
			else if (m_nbRooms == 2)
				rent = m_surface * borough.m_priceRentApartmentT2.m_val;
			else if (m_nbRooms == 3)
				rent = m_surface * borough.m_priceRentApartmentT3.m_val;
			else
				rent = m_surface * borough.m_priceRentApartmentT4Plus.m_val;

			return rent;
		}
	}
	return 0.f;
}

bool SearchRequestResultAnnounce::Display(ImGuiTextFilter* _filter)
{
	bool keep = true;
	if (_filter)
	{
		if (!_filter->PassFilter(m_name.c_str()) && !_filter->PassFilter(m_description.c_str()))
			return true;
	}

	// Download image
	if (!m_imageDownloaded && (m_imageDownloadRequestID > -1))
	{
		if (OnlineManager::getSingleton()->IsBasicHTTPRequestAvailable(m_imageDownloadRequestID))
		{
			unsigned char* buffer = nullptr;
			int size = 0;
			OnlineManager::getSingleton()->GetBinaryHTTPRequestResult(m_imageDownloadRequestID, buffer, size);
			if (buffer)
			{
				ProdToolGL_GenerateTextureFromBuffer(buffer, size, m_imageWidth, m_imageHeight, m_imageTextureID);
				free(buffer);
			}
		}
	}

	if (m_imageTextureID > 0)
	{
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, m_imageWidth + 15.f);
		ImGui::Image((void*)(intptr_t)m_imageTextureID, ImVec2(m_imageWidth, m_imageHeight));
		ImGui::NextColumn();
	}
	else
		ImGui::Columns(1);
	ImGui::Separator();
	ImGui::SetWindowFontScale(1.2f);
	std::string name = (m_category == Category_Apartment ? GET_TEXT("GeneralAppartment") : GET_TEXT("GeneralHouse"));
	name += ", " + m_name;
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), name.c_str());
	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
		ShellExecuteA(NULL, "open", m_URL.c_str(), NULL, NULL, SW_SHOWDEFAULT);

	ImGui::SameLine();
	ImGui::SetWindowFontScale(1.f);

	ImGui::PushID(this + 123456);
	if (ImGui::Button(GET_TEXT("GeneralRemove")))
		keep = false;
	ImGui::PopID();

	ImGui::TextWrapped(m_description.c_str());

	ImGui::Columns(7);

	ImGui::Separator();

	std::vector<const char*> data;
	data.resize(m_boroughs.size() + 1);
	data[0] = "";
	for (auto ID = 0; ID < m_boroughs.size(); ++ID)
		data[ID + 1] = m_boroughs[ID].m_name.c_str();
	ImGui::PushID(this + 12345);
	ImGui::Combo(GET_TEXT("GeneralBorough"), &m_selectedBoroughID, data.data(), (int)data.size());
	ImGui::PopID();

	// Compute rentability rate
	bool needNeighboorUpdate = false;
	std::string rate = std::string("<= ") + GET_TEXT("SearchRequestResultSelectBorough");
	BoroughData requestBorough;
	if (m_selectedBoroughID > 0)
	{
		const BoroughData& borough = m_boroughs[m_selectedBoroughID - 1];
		if (borough.m_priceRentApartmentT1.m_val < 0.1f)
		{
			needNeighboorUpdate = true;
			requestBorough = borough;
		}

		if (m_waitingForDBUpdate && !DatabaseManager::getSingleton()->IsBoroughUpdating(borough))
		{
			m_waitingForDBUpdate = false;
			UpdateBoroughs();
		}

		float tmp = GetRentabilityRate();
		std::round(tmp);
		char buf[128];
		sprintf(buf, "%.1f", tmp);
		rate = GET_TEXT("GeneralRate") +  std::string(": ") + std::string(buf);
	}

	ImGui::NextColumn();
	if (!needNeighboorUpdate)
		ImGui::Text("%s     ", rate.c_str());
	else if (m_waitingForDBUpdate)
	{
		char buf[128];
		sprintf_s(buf, "%s... %c", GET_TEXT("SearchRequestResultUpdatingBorough"), "|/-\\"[(int)(ImGui::GetTime() / 0.1f) & 3]);
		ImGui::Text(buf);
	}
	else if (ImGui::Button(GET_TEXT("SearchRequestResultUpdateBorough")))
	{
		DatabaseManager::getSingleton()->ComputeBoroughData(requestBorough);
		m_waitingForDBUpdate = true;
	}
	ImGui::NextColumn();
	float rent = GetEstimatedRent();
	if (rent > 0.f)
		ImGui::Text("%s: %.0f     ", GET_TEXT("SearchRequestResultEstimatedRent"),rent);
	else if(ImGui::Button(GET_TEXT("SearchRequestResultUpdateBorough")))
	{
		DatabaseManager::getSingleton()->ComputeBoroughData(requestBorough);
		m_waitingForDBUpdate = true;
	}
	ImGui::NextColumn();
	ImGui::Text("%s: %u     ", GET_TEXT("GeneralPrice"), m_price);
	ImGui::NextColumn();
	ImGui::Text("Surface: %.0f m2     ", m_surface);
	ImGui::NextColumn();
	ImGui::Text("%s: %u     ", GET_TEXT("SearchRequestResultNbRooms"), m_nbRooms);
	ImGui::NextColumn();
	ImGui::Text("%s: %u     ", GET_TEXT("SearchRequestResultNbBedRooms"), m_nbBedRooms);

	ImGui::Columns(1);
	ImGui::Separator();
	ImGui::Text(" ");

	return keep;
}