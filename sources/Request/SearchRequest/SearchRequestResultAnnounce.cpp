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
#include <UI\UIManager.h>
#include <Database\ImageDatabase.h>

using namespace ImmoBank;

void SearchRequestResultAnnounce::Init()
{
	if (!m_imageTinyURL.empty())
		m_loadedImageURL = m_imageTinyURL;
	else if (!m_imageURL.empty())
		m_loadedImageURL = m_imageURL;

	_LoadImage(m_loadedImageURL, m_imageDownloadRequestID, m_imageTextureID, m_imageDownloaded);
	
	m_priceM2 = int((float)m_price / m_surface);
}

void SearchRequestResultAnnounce::End()
{
	if (m_imageTextureID > 0)
		ProdToolGL_DeleteTexture(&m_imageTextureID);
	m_imageTextureID = 0;

	if (m_imageDownloadRequestID > -1)
		OnlineManager::getSingleton()->CancelBasicHTTPRequest(m_imageDownloadRequestID);
}

void SearchRequestResultAnnounce::PostProcess()
{
	StringTools::RemoveSpecialCharacters(m_name);
	StringTools::RemoveSpecialCharacters(m_description);

	UpdateBoroughs(true);
}

void SearchRequestResultAnnounce::UpdateBoroughs(bool _lookForBorough)
{
	DatabaseManager::getSingleton()->GetBoroughs(m_city, m_boroughs);

	std::sort(m_boroughs.begin(), m_boroughs.end(), BoroughData::compare);

	if (_lookForBorough)
	{
		std::string description = m_description;
		StringTools::TransformToLower(description);

		bool found = false;
		int ID = 0;
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

	m_rent = (int)GetEstimatedRent();
}

float SearchRequestResultAnnounce::GetRentabilityRate() const
{
	return Tools::ComputeRentabilityRate((float)m_rent, (float)m_price);
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

bool SearchRequestResultAnnounce::_LoadImage(const std::string& _path, int& _requestID, unsigned int& _textureID, bool& _downloadStatus)
{
	if (!_path.empty())
	{
		if (!ImageDatabase::getSingleton()->HasImage(_path))
		{
			std::string filePath = ImageDatabase::getSingleton()->GenerateNewImageFullPath(_path);
			_requestID = OnlineManager::getSingleton()->SendBinaryHTTPRequest(_path, filePath);
		}
		else
		{
			int size = 0;
			unsigned char* buffer = ImageDatabase::getSingleton()->GetImage(_path, size);
			int width, height;
			if (ProdToolGL_GenerateTextureFromJPEGBuffer(buffer, size, width, height, _textureID))
			{
				free(buffer);
				_requestID = -1;
				_downloadStatus = true;
				return true;
			}
			_downloadStatus = true;
		}
	}
	return false;
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
	if (m_imageDownloadRequestID > -1)
	{
		if (!m_imageDownloaded)
		{
			if (OnlineManager::getSingleton()->IsHTTPRequestAvailable(m_imageDownloadRequestID))
			{
				if (!_LoadImage(m_loadedImageURL, m_imageDownloadRequestID, m_imageTextureID, m_imageDownloaded))
				{
					if (!m_loadedImageURL.empty())
						m_imageDownloadRequestID = OnlineManager::getSingleton()->SendBinaryHTTPRequest(m_loadedImageURL);
				}
			}
		}
	}

	ImGui::Columns(2);
	float width = 140.f;
	ImGui::SetColumnWidth(0, width + 15.f);
	if (m_imageTextureID > 0)
	{
		if (ImGui::ImageButton((void*)(intptr_t)m_imageTextureID, ImVec2(140.f, width / 1.333f)))
			ShellExecuteA(NULL, "open", m_URL.c_str(), NULL, NULL, SW_SHOWDEFAULT);
	}
	else
	{
		if (m_imageDownloadRequestID > -1)
			ImGui::Text("Loading...");
		else
			ImGui::Text("No image");
	}

	ImGui::NextColumn();
	ImGui::Separator();
	ImGui::SetWindowFontScale(1.2f);
	std::string name = m_database + " - " + (m_category == Category_Apartment ? GET_TEXT("GeneralAppartment") : GET_TEXT("GeneralHouse"));
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

	ImGui::TextWrapped("%s", m_description.c_str());

	ImGui::Columns(8);

	int columnID = 0;
	ImGui::SetColumnWidth(columnID++, 240.f);
	ImGui::SetColumnWidth(columnID++, 80.f);
	ImGui::SetColumnWidth(columnID++, 190.f);
	ImGui::SetColumnWidth(columnID++, 110.f);
	ImGui::SetColumnWidth(columnID++, 120.f);
	ImGui::SetColumnWidth(columnID++, 70.f);
	ImGui::SetColumnWidth(columnID++, 90.f);
	ImGui::SetColumnWidth(columnID++, 130.f);
	ImGui::Separator();

	std::vector<const char*> data;
	data.resize(m_boroughs.size() + 1);
	data[0] = "";
	for (auto ID = 0; ID < m_boroughs.size(); ++ID)
		data[ID + 1] = m_boroughs[ID].m_name.c_str();

	ImGui::PushID(this + 12345);
	bool forceChange = ImGui::Combo(GET_TEXT("GeneralBorough"), &m_selectedBoroughID, data.data(), (int)data.size());
	ImGui::PopID();

	// Compute rentability rate
	bool needNeighboorUpdate = false;
	std::string rate = std::string("<= ") + GET_TEXT("SearchRequestResultSelectBorough");
	BoroughData requestBorough;
	if (forceChange || (m_selectedBoroughID > 0))
	{
		const BoroughData& borough = m_boroughs[m_selectedBoroughID - 1];
		if (borough.m_priceRentApartmentT1.m_val < 0.1f)
		{
			needNeighboorUpdate = true;
			requestBorough = borough;
		}

		if (forceChange || (m_waitingForDBUpdate && !DatabaseManager::getSingleton()->IsBoroughUpdating(borough)))
		{
			m_waitingForDBUpdate = false;
			UpdateBoroughs(false);
		}

		float tmp = GetRentabilityRate();
		std::round(tmp);
		char buf[128];
		sprintf(buf, "%.1f", tmp);
		rate = GET_TEXT("GeneralRate") +  std::string(": ") + std::string(buf);
	}

	ImGui::NextColumn();
	if (!needNeighboorUpdate)
	{
		ImGui::Text("%s     ", rate.c_str());
	}
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
	if (m_rent > 0)
	{
		ImGui::PushID(this + 51384);
		ImGui::Text(GET_TEXT("SearchRequestResultEstimatedRent"));
		ImGui::SameLine();
		ImGui::InputInt("€", &m_rent, 10, min(m_rent * 2, 2000));
		ImGui::PopID();
	}
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
	ImGui::NextColumn();
	ImGui::Text("%s: %u     ", GET_TEXT("SearchRequestResultPriceM2"), m_priceM2);

	ImGui::Columns(1);
	ImGui::Separator();
	ImGui::Text(" ");

	return keep;
}

bool SearchRequestResultAnnounce::Compare(const SearchRequestResult* _target, Tools::SortType _sortType, bool _invert) const
{
	bool result = false;
	switch (_sortType)
	{
	case Tools::SortType::Rate:
		result = !_invert 
			? GetRentabilityRate() < ((SearchRequestResultAnnounce*)_target)->GetRentabilityRate()
			: GetRentabilityRate() > ((SearchRequestResultAnnounce*)_target)->GetRentabilityRate();
		break;
	case Tools::SortType::Price:
		result = !_invert
			? m_price < ((SearchRequestResultAnnounce*)_target)->m_price
			: m_price >((SearchRequestResultAnnounce*)_target)->m_price;
		break;
	case Tools::SortType::Surface:
		result = !_invert
			? m_surface < ((SearchRequestResultAnnounce*)_target)->m_surface
			: m_surface >((SearchRequestResultAnnounce*)_target)->m_surface;
		break;
	case Tools::SortType::PriceM2:
		result = !_invert
			? m_priceM2 < ((SearchRequestResultAnnounce*)_target)->m_priceM2
			: m_priceM2 > ((SearchRequestResultAnnounce*)_target)->m_priceM2;
		break;
	}

	return result;
}