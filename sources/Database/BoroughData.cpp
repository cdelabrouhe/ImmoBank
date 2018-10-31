#include "BoroughData.h"
#include <vector>
#include "Online\OnlineManager.h"
#include "Request\SearchRequest\SearchRequestCityBoroughData.h"
#include "Request\SearchRequest\SearchRequestResulCityBoroughData.h"
#include "extern/ImGui/imgui.h"
#include <GLFW\glfw3.h>
#include <Tools\Tools.h>
#include "Tools\StringTools.h"
#include <windows.h>
#include <shellapi.h>
#include <ctime>
#include "Text\TextManager.h"

BoroughData::BoroughData()
{
	SetTimeUpdateToNow();
}

//-------------------------------------------------------------------------------------------------
void BoroughData::Init()
{
	SearchRequestCityBoroughData request;
	request.m_data = *this;
	request.m_city = m_city;
	m_httpRequestID = OnlineManager::getSingleton()->SendRequest(&request);
}

//-------------------------------------------------------------------------------------------------
bool BoroughData::Process()
{
	if (OnlineManager::getSingleton()->IsRequestAvailable(m_httpRequestID))
	{
		std::vector<SearchRequestResult*> list;
		if (OnlineManager::getSingleton()->GetRequestResult(m_httpRequestID, list))
		{
			m_httpRequestID = -1;

			for (auto result : list)
			{
				if (result->m_resultType == SearchRequestType_CityBoroughData)
				{
					SearchRequestResulCityBoroughData* borough = static_cast<SearchRequestResulCityBoroughData*>(result);
					DatabaseManager::getSingleton()->AddBoroughData(borough->m_data);
				}
			}
		}

		return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
void BoroughData::End()
{

}

//-------------------------------------------------------------------------------------------------
void BoroughData::Reset(bool _resetDB)
{
	m_priceRentApartmentT1.Reset();
	m_priceRentApartmentT2.Reset();
	m_priceRentApartmentT3.Reset();
	m_priceRentApartmentT4Plus.Reset();
	m_priceBuyApartment.Reset();
	m_priceBuyHouse.Reset();
	m_priceRentHouse.Reset();

	// Reset all data in DBs
	if (_resetDB)
	{
		DatabaseManager::getSingleton()->ForceBoroughReset(*this);
	}
}

//-------------------------------------------------------------------------------------------------
void BoroughData::SetWholeCity()
{
	m_name = s_wholeCityName;
}

//-------------------------------------------------------------------------------------------------
bool BoroughData::IsWholeCity() const
{
	return m_name == s_wholeCityName;
}

//-------------------------------------------------------------------------------------------------
void BoroughData::OpenInBrowser() const
{
	std::string request = ComputeRequestURL();
	ShellExecuteA(NULL, "open", request.c_str(), NULL, NULL, SW_SHOWDEFAULT);
}

//-------------------------------------------------------------------------------------------------
bool BoroughData::IsValid() const
{
	return ((m_priceBuyApartment.m_val > 0.f) || (m_priceBuyHouse.m_val > 0.f));
}

//-------------------------------------------------------------------------------------------------
void BoroughData::SetTimeUpdateToNow()
{
	std::time_t t = std::time(0);   // get time now
	std::tm* now = std::localtime(&t);
	m_timeUpdate.SetDate(now->tm_year + 1900, now->tm_mon, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
}

//-------------------------------------------------------------------------------------------------
void BoroughData::SetSelogerKey(unsigned int _key, bool _isCity)
{
	m_selogerKey = ConvertSelogerKey(_key, _isCity);
}

//-------------------------------------------------------------------------------------------------
unsigned int BoroughData::ConvertSelogerKey(unsigned int _key, bool _isCity)
{
	if (_isCity)
		_key += 1 << 31;

	return _key;
}

//-------------------------------------------------------------------------------------------------
int GetBoroughNumber(const std::string& _boroughName)
{
	int maxBoroughNumber = 30;
	int result = -1;
	for (int ID = 0; ID < maxBoroughNumber; ++ID)
	{
		std::string val = std::to_string(ID) + "e";
		auto it = _boroughName.find(val);
		if (it != std::string::npos)
			result = ID;
	}

	return result;
}

//-------------------------------------------------------------------------------------------------
std::string BoroughData::ComputeRequestURL() const
{
	//std::string request = "https://www.meilleursagents.com/prix-immobilier/montpellier-34000/quartier_antigone-170492247/"
	std::string boroughName = m_name;
	StringTools::ReplaceBadSyntax(boroughName, " / ", "-");
	StringTools::ReplaceBadSyntax(boroughName, " \\ ", "-");
	StringTools::ReplaceBadSyntax(boroughName, " ", "-");
	StringTools::ReplaceBadSyntax(boroughName, "'", "-");
	StringTools::ReplaceBadSyntax(boroughName, "St.", "saint");
	StringTools::ReplaceBadSyntax(boroughName, "St-", "saint-");
	StringTools::ReplaceBadSyntax(boroughName, "Ste.", "sainte");
	StringTools::ReplaceBadSyntax(boroughName, "Ste-", "sainte-");
	StringTools::ReplaceBadSyntax(boroughName, "�", "e");
	StringTools::ReplaceBadSyntax(boroughName, "�", "e");
	std::string zipCode = std::to_string(m_city.m_zipCode);
	while (zipCode.size() < 5)
		zipCode = "0" + zipCode;

	std::string name = m_city.m_name;
	StringTools::ReplaceBadSyntax(name, " ", "-");
	StringTools::ConvertToImGuiText(name);
	std::string request = "https://www.meilleursagents.com/prix-immobilier/" + name;

	int boroughNumber = GetBoroughNumber(boroughName);
	if (boroughNumber == -1)
	{
		request = request + "-" + zipCode + "/";
		if (!IsWholeCity())
			request += "quartier_" + boroughName + "-" + std::to_string(m_meilleursAgentsKey);
		StringTools::TransformToLower(request);
	}
	else
	{
		StringTools::TransformToLower(request);
		int departement = m_city.m_zipCode / 1000;
		int zip = (departement * 1000) + boroughNumber;
		zipCode = std::to_string(zip);
		request = request + "-" + std::to_string(boroughNumber) + (boroughNumber == 1 ? "er" : "eme") + "-arrondissement-" + zipCode;
	}

	return request;
}

//-------------------------------------------------------------------------------------------------
std::string BoroughData::ComputeSeLogerKeyURL() const
{
	std::string str = m_name;
	auto delimiter = str.find("e (");
	if (delimiter != std::string::npos)
		str = str.substr(0, delimiter);

	delimiter = str.find("er (");
	if (delimiter != std::string::npos)
		str = str.substr(0, delimiter);

	std::string request = "https://autocomplete.svc.groupe-seloger.com/auto/complete/0/ALL/6?text=" + str;
	StringTools::ReplaceBadSyntax(request, " ", "+");

	return request;
}

//-------------------------------------------------------------------------------------------------
void BoroughData::Edit()
{
	// Popup ?
#define EDIT_INFO(name, data, decimal) \
			ImGui::SetWindowFontScale(1.f); \
			ImGui::Text(#name " : "); \
			ImGui::SameLine(); \
			ImGui::PushID(this + localID++); \
			ImGui::InputFloat3(GET_TEXT("BoroughManualEditMinAvgMax"), &data.m_min, decimal); \
			ImGui::PopID(); \

#define EDIT_INFO_CSTR(name, data, decimal) \
			ImGui::SetWindowFontScale(1.f); \
			ImGui::Text("%s : ", name); \
			ImGui::SameLine(); \
			ImGui::PushID(this + localID++); \
			ImGui::InputFloat3(GET_TEXT("BoroughManualEditMinAvgMax"), &data.m_min, decimal); \
			ImGui::PopID(); \

	if (ImGui::BeginPopupModal(GET_TEXT("BoroughManualEditPopup"), NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		bool isPaste = ImGui::IsKeyPressed(GLFW_KEY_LEFT_CONTROL) && ImGui::IsKeyDown(GLFW_KEY_V);
		if (isPaste)
		{
			const char* clipboard = ImGui::GetClipboardText();
			if (strlen(clipboard) > 0)
			{
				std::string str(clipboard);
				if (Tools::ExtractPricesFromHTMLSource(str, m_priceRentApartmentT1, m_priceRentApartmentT2, m_priceRentApartmentT3, m_priceRentApartmentT4Plus, m_priceBuyApartment, m_priceBuyHouse))
					DatabaseManager::getSingleton()->AddBoroughData(*this);
			}
		}

		ImGui::SetWindowFontScale(1.f);

		ImGui::Separator();
		ImGui::Text(GET_TEXT("BoroughManualEditBuy"));
		int localID = 0;
		EDIT_INFO_CSTR(GET_TEXT("BoroughManualEditAppartment"), m_priceBuyApartment, 0);
		EDIT_INFO_CSTR(GET_TEXT("BoroughManualEditHouse"), m_priceBuyHouse, 0);
		ImGui::Separator();
		ImGui::SetWindowFontScale(1.f);
		ImGui::Text(GET_TEXT("BoroughManualEditRent"));
		EDIT_INFO(T1, m_priceRentApartmentT1, 1);
		EDIT_INFO(T2, m_priceRentApartmentT2, 1);
		EDIT_INFO(T3, m_priceRentApartmentT3, 1);
		EDIT_INFO(T4+, m_priceRentApartmentT4Plus, 1);
		EDIT_INFO_CSTR(GET_TEXT("BoroughManualEditHouse"), m_priceRentHouse, 1);

		ImGui::Separator();

		if (ImGui::Button(GET_TEXT("BoroughManualEditSave")))
		{
			SetTimeUpdateToNow();
			DatabaseManager::getSingleton()->AddBoroughData(*this);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button(GET_TEXT("BoroughManualEditCancel")))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
}

static std::string To2DigitsString(int _value)
{
	std::string str = std::to_string(_value);
	if (str.size() == 1)
		str = "0" + str;
	return str;
}

void BoroughData::DisplayAsTooltip()
{
	bool hovered = ImGui::IsItemHovered();
	if (hovered)
	{
		ImGui::BeginTooltip();
		ImGui::SetWindowFontScale(1.f);

#ifdef DEV_MODE
		ImGui::Text("MeilleursAgentsKey: %u", m_meilleursAgentsKey);
		ImGui::Text("SeLogerKey: %u", m_selogerKey);
#endif

		if (IsValid())
		{
			static float s_sizeMin = 0.8f;
			static float s_size = 1.f;
			static float s_sizeMax = 0.8f;
			static ImVec4 s_colorMin(1.f, 1.f, 0.f, 1.f);
			static ImVec4 s_color(0.f, 1.f, 0.f, 1.f);
			static ImVec4 s_colorMax(1.f, 0.5f, 0.f, 1.f);

#define DISPLAY_INFO(name, data) \
			ImGui::SetWindowFontScale(1.f); \
			ImGui::Text(#name " : "); \
			ImGui::SetWindowFontScale(s_sizeMin); ImGui::SameLine(); ImGui::PushStyleColor(ImGuiCol_Text, s_colorMin); ImGui::Text("%.f", data.m_min); ImGui::PopStyleColor(); \
			ImGui::SetWindowFontScale(s_size); ImGui::SameLine(); ImGui::PushStyleColor(ImGuiCol_Text, s_color); ImGui::Text("   %.f   ", data.m_val); ImGui::PopStyleColor(); \
			ImGui::SetWindowFontScale(s_sizeMax); ImGui::SameLine(); ImGui::PushStyleColor(ImGuiCol_Text, s_colorMax); ImGui::Text("%.f", data.m_max); ImGui::PopStyleColor(); \

#define DISPLAY_INFO_CSTR(name, data) \
			ImGui::SetWindowFontScale(1.f); \
			ImGui::Text("%s : ", name); \
			ImGui::SetWindowFontScale(s_sizeMin); ImGui::SameLine(); ImGui::PushStyleColor(ImGuiCol_Text, s_colorMin); ImGui::Text("%.f", data.m_min); ImGui::PopStyleColor(); \
			ImGui::SetWindowFontScale(s_size); ImGui::SameLine(); ImGui::PushStyleColor(ImGuiCol_Text, s_color); ImGui::Text("   %.f   ", data.m_val); ImGui::PopStyleColor(); \
			ImGui::SetWindowFontScale(s_sizeMax); ImGui::SameLine(); ImGui::PushStyleColor(ImGuiCol_Text, s_colorMax); ImGui::Text("%.f", data.m_max); ImGui::PopStyleColor(); \

			ImGui::Text(GET_TEXT("PricePopupPricesM2"));
			ImGui::SetWindowFontScale(s_sizeMin); ImGui::SameLine(); ImGui::PushStyleColor(ImGuiCol_Text, s_colorMin); ImGui::Text("min"); ImGui::PopStyleColor();
			ImGui::SetWindowFontScale(s_size); ImGui::SameLine(); ImGui::PushStyleColor(ImGuiCol_Text, s_color); ImGui::Text(" medium "); ImGui::PopStyleColor();
			ImGui::SetWindowFontScale(s_sizeMax); ImGui::SameLine(); ImGui::PushStyleColor(ImGuiCol_Text, s_colorMax); ImGui::Text("max"); ImGui::PopStyleColor();

			ImGui::Separator();

			ImGui::SetWindowFontScale(1.f);
			ImGui::Text(GET_TEXT("PricePopupBuy"));
			DISPLAY_INFO_CSTR(GET_TEXT("PricePopupAppartment"), m_priceBuyApartment);
			DISPLAY_INFO_CSTR(GET_TEXT("PricePopupHouse"), m_priceBuyHouse);
			ImGui::Separator();
			ImGui::SetWindowFontScale(1.f);
			ImGui::Text(GET_TEXT("PricePopupRent"));
			DISPLAY_INFO(T1, m_priceRentApartmentT1);
			DISPLAY_INFO(T2, m_priceRentApartmentT2);
			DISPLAY_INFO(T3, m_priceRentApartmentT3);
			DISPLAY_INFO(T4 + , m_priceRentApartmentT4Plus);
			DISPLAY_INFO_CSTR(GET_TEXT("PricePopupHouse"), m_priceRentHouse);

			ImGui::Separator();

			ImGui::SetWindowFontScale(1.f);
			std::string day = To2DigitsString(m_timeUpdate.GetDay());
			std::string month = To2DigitsString(m_timeUpdate.GetMonth() + 1);
			std::string year = To2DigitsString(m_timeUpdate.GetYear());
			std::string hour = To2DigitsString(m_timeUpdate.GetHour());
			std::string minute = To2DigitsString(m_timeUpdate.GetMinute());
			std::string second = To2DigitsString(m_timeUpdate.GetSecond());
			ImGui::Text("%s: %s-%s-%s at %s:%s:%s" 
							, GET_TEXT("GeneralUpdate")
							, day.c_str()
							, month.c_str()
							, year.c_str()
							, hour.c_str()
							, minute.c_str()
							, second.c_str());
		}
		else
		{
#ifdef DEV_MODE
			ImGui::Separator();
#endif

			static ImVec4 s_colorNoData(1.f, 0.f, 0.f, 1.f);
			ImGui::PushStyleColor(ImGuiCol_Text, s_colorNoData);
			ImGui::Text(GET_TEXT("PricePopupNoValidData"));
			ImGui::PopStyleColor();
		}
		ImGui::EndTooltip();
	}
}