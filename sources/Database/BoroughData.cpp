#include "BoroughData.h"
#include <vector>
#include "Online\OnlineManager.h"
#include "Request\SearchRequest\SearchRequestCityBoroughData.h"
#include "Request\SearchRequest\SearchRequestResulCityBoroughData.h"
#include "extern/ImGui/imgui.h"
#include <GLFW\glfw3.h>
#include <Tools\Tools.h>

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
void BoroughData::Reset()
{
	m_priceRentApartmentT1.Reset();
	m_priceRentApartmentT2.Reset();
	m_priceRentApartmentT3.Reset();
	m_priceRentApartmentT4Plus.Reset();
	m_priceBuyApartment.Reset();
	m_priceBuyHouse.Reset();
	m_priceRentHouse.Reset();
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
void BoroughData::Edit()
{
	// Popup ?
#define EDIT_INFO(name, data, decimal) \
			ImGui::SetWindowFontScale(1.f); \
			ImGui::Text(#name " : "); \
			ImGui::SameLine(); \
			ImGui::PushID(this + localID++); \
			ImGui::InputFloat3("Min / Avg / Max",&data.m_min, decimal); \
			ImGui::PopID(); \

	if (ImGui::BeginPopupModal("ManualEdit", NULL, ImGuiWindowFlags_AlwaysAutoResize))
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
#ifdef _DEBUG
		static int s_key;
		s_key = m_key;
		if (ImGui::InputInt("Key", &s_key))
			m_key = s_key;
#endif
		ImGui::Separator();
		ImGui::Text("BUY");
		int localID = 0;
		EDIT_INFO(App, m_priceBuyApartment, 0);
		EDIT_INFO(House, m_priceBuyHouse, 0);
		ImGui::Separator();
		ImGui::SetWindowFontScale(1.f);
		ImGui::Text("RENT");
		EDIT_INFO(T1, m_priceRentApartmentT1, 1);
		EDIT_INFO(T2, m_priceRentApartmentT2, 1);
		EDIT_INFO(T3, m_priceRentApartmentT3, 1);
		EDIT_INFO(T4 + , m_priceRentApartmentT4Plus, 1);
		EDIT_INFO(House, m_priceRentHouse, 1);

		ImGui::Separator();

		if (ImGui::Button("Save"))
		{
			DatabaseManager::getSingleton()->AddBoroughData(*this);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
}

void BoroughData::DisplayAsTooltip()
{
	bool hovered = ImGui::IsItemHovered();
	if (hovered)
	{
		const bool validData = ((m_priceBuyApartment.m_val > 0.f) && (m_priceBuyHouse.m_val > 0.f));

		ImGui::BeginTooltip();
		ImGui::SetWindowFontScale(1.f);

		if (validData)
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

#ifdef _DEBUG
			ImGui::Text("Key: %u", m_key);
#endif
			ImGui::Text("Prices (per m2) ");
			ImGui::SetWindowFontScale(s_sizeMin); ImGui::SameLine(); ImGui::PushStyleColor(ImGuiCol_Text, s_colorMin); ImGui::Text("min"); ImGui::PopStyleColor();
			ImGui::SetWindowFontScale(s_size); ImGui::SameLine(); ImGui::PushStyleColor(ImGuiCol_Text, s_color); ImGui::Text(" medium "); ImGui::PopStyleColor();
			ImGui::SetWindowFontScale(s_sizeMax); ImGui::SameLine(); ImGui::PushStyleColor(ImGuiCol_Text, s_colorMax); ImGui::Text("max"); ImGui::PopStyleColor();

			ImGui::Separator();

			ImGui::SetWindowFontScale(1.f);
			ImGui::Text("BUY");
			DISPLAY_INFO(App, m_priceBuyApartment);
			DISPLAY_INFO(House, m_priceBuyHouse);
			ImGui::Separator();
			ImGui::SetWindowFontScale(1.f);
			ImGui::Text("RENT");
			DISPLAY_INFO(T1, m_priceRentApartmentT1);
			DISPLAY_INFO(T2, m_priceRentApartmentT2);
			DISPLAY_INFO(T3, m_priceRentApartmentT3);
			DISPLAY_INFO(T4 + , m_priceRentApartmentT4Plus);
			DISPLAY_INFO(House, m_priceRentHouse);

		}
		else
		{
			static ImVec4 s_colorNoData(1.f, 0.f, 0.f, 1.f);
			ImGui::PushStyleColor(ImGuiCol_Text, s_colorNoData);
			ImGui::Text("No valid data, update it !");
			ImGui::PopStyleColor();
		}
		ImGui::EndTooltip();
	}
}