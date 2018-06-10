#include "UIManager.h"

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <algorithm>

#include "extern/ImGui/imgui.h"
#include "Request/RequestManager.h"
#include "Database/DatabaseManager.h"
#include "CitySelector.h"
#include "Tools/StringTools.h"

//-------------------------------------------------------------------------------------------------
// FORWARD DECLARATIONS
//-------------------------------------------------------------------------------------------------
UIManager* s_singleton = nullptr;

//-------------------------------------------------------------------------------------------------
UIManager* UIManager::getSingleton()
{
	if (s_singleton == nullptr)
		s_singleton = new UIManager();
	return s_singleton;
}

//-------------------------------------------------------------------------------------------------
// FUNCTIONS
//-------------------------------------------------------------------------------------------------
UIManager::UIManager()
{
	FontDefault = NULL;
}

bool UIManager::Draw()
{
	bool quit = false;

	// Fullscreen window
	const float BORDER_WIDTH = 0.0f;
	ImGui::SetNextWindowPos(ImVec2(BORDER_WIDTH, BORDER_WIDTH));
	ImVec2 vec(ImGui::GetIO().DisplaySize.x - BORDER_WIDTH * 2, ImGui::GetIO().DisplaySize.y - BORDER_WIDTH * 2);
	ImGui::SetNextWindowSize(vec);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::Begin("ProdTool", NULL, ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoFocusOnAppearing|ImGuiWindowFlags_NoBringToFrontOnFocus|ImGuiWindowFlags_MenuBar);

	static bool showTestWindow = false;
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New request"))
				RequestManager::getSingleton()->CreateRequestAnnounceDefault();

			ImGui::Separator();
			if (ImGui::MenuItem("Exit", "ALT+F4"))
				quit = true;

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Database"))
		{
			if (ImGui::MenuItem("View city data"))
				UIManager::getSingleton()->AskForDisplayCityInformation();

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Window"))
		{
			if (ImGui::BeginMenu("Style"))
			{
				if (ImGui::MenuItem("Classic"))
					ImGui::StyleColorsClassic();

				if (ImGui::MenuItem("Dark"))
					ImGui::StyleColorsDark();

				if (ImGui::MenuItem("Light"))
					ImGui::StyleColorsLight();

				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("?"))
		{
			if (ImGui::MenuItem("Show test window"))
				showTestWindow = !showTestWindow;
			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	RequestManager::getSingleton()->DisplayRequests();

	if (showTestWindow)
		ImGui::ShowTestWindow();

	ImGui::End();
	ImGui::PopStyleVar();

	return quit;
}

//-------------------------------------------------------------------------------------------------
void UIManager::Process()
{
	if (m_displayCityData)
		DisplayCityInformation();

}

//-------------------------------------------------------------------------------------------------
void UIManager::AskForDisplayCityInformation()
{
	m_displayCityData = !m_displayCityData;
	if (m_displayCityData)
		InitDisplayCityInformation();
}

//-------------------------------------------------------------------------------------------------
void UIManager::InitDisplayCityInformation()
{
	m_cityListRequested = false;
	m_selectedCityID = 0;
	m_hovered = -1;
	m_selected = -1;
}

//-------------------------------------------------------------------------------------------------
void DisplayPricesTooltip(sBoroughData& _borough)
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

	ImGui::BeginTooltip();
	ImGui::SetWindowFontScale(1.0f);
	//ImGui::Text("Key: %u", _borough.m_key);
	ImGui::Text("Prices (per m2) ");
	ImGui::SetWindowFontScale(s_sizeMin); ImGui::SameLine(); ImGui::PushStyleColor(ImGuiCol_Text, s_colorMin); ImGui::Text("min"); ImGui::PopStyleColor();
	ImGui::SetWindowFontScale(s_size); ImGui::SameLine(); ImGui::PushStyleColor(ImGuiCol_Text, s_color); ImGui::Text(" medium "); ImGui::PopStyleColor();
	ImGui::SetWindowFontScale(s_sizeMax); ImGui::SameLine(); ImGui::PushStyleColor(ImGuiCol_Text, s_colorMax); ImGui::Text("max"); ImGui::PopStyleColor();

	ImGui::Separator();

	ImGui::SetWindowFontScale(1.f);
	ImGui::Text("BUY");
	DISPLAY_INFO(App, _borough.m_priceBuyApartment);
	DISPLAY_INFO(House, _borough.m_priceBuyHouse);
	ImGui::Separator();
	ImGui::SetWindowFontScale(1.f);
	ImGui::Text("RENT");
	DISPLAY_INFO(T1, _borough.m_priceRentApartmentT1);
	DISPLAY_INFO(T2, _borough.m_priceRentApartmentT2);
	DISPLAY_INFO(T3, _borough.m_priceRentApartmentT3);
	DISPLAY_INFO(T4 + , _borough.m_priceRentApartmentT4Plus);
	DISPLAY_INFO(House, _borough.m_priceRentHouse);

	ImGui::EndTooltip();
}

//-------------------------------------------------------------------------------------------------
void UIManager::DisplayCityInformation()
{
	static CitySelector s_citySelector;
	s_citySelector.SetDisplayAllResults(false);

	if (!m_cityListRequested || s_citySelector.HasChanged())
	{
		m_cityListFull.clear();
		DatabaseManager::getSingleton()->ListAllCities(m_cityListFull);
		m_cityListRequested = true;
	}

	ImGui::SetNextWindowSize(ImVec2(900, 500), ImGuiCond_FirstUseEver);
	ImGui::Begin("City info display", &m_displayCityData);

	// Left panel (city selector process)
	bool listUpdated = false;
	std::vector<std::string> cityListFiltered;
	ImGui::BeginChild("City search", ImVec2(300, 0), true);

	sCity result;
	if (s_citySelector.Display())
		result = *s_citySelector.GetSelectedCity();

	if (strlen(s_citySelector.GetText()) > 0)
	{
		std::string input = s_citySelector.GetText();
		StringTools::TransformToLower(input);
		for (auto city : m_cityListFull)
		{
			std::string tmp = city;
			StringTools::TransformToLower(tmp);
			auto findID = tmp.find(s_citySelector.GetText());
			if (findID != std::string::npos)
				cityListFiltered.push_back(city);
		}

		listUpdated = true;
	}
	ImGui::Separator();

	if (!listUpdated)
	{
		for (auto& city : m_cityListFull)
			cityListFiltered.push_back(city);
	}

	int localHovered = -1;
	int cpt = 0;
	const ImColor colSelected(1.0f, 0.0f, 0.0f);
	const ImColor colHovered(0.0f, .5f, .5f);
	for (auto& city : cityListFiltered)
	{
		const bool isHovered = (m_hovered == cpt);
		const bool isSelected = (m_selected == cpt);
		if (isSelected)
			ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)colSelected);
		else if (isHovered)
			ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)colHovered);

		ImGui::Text(city.c_str());
		if (ImGui::IsItemHovered())
			m_hovered = cpt;

		if (ImGui::IsItemClicked())
			m_selected = cpt;

		if (isSelected || isHovered)
			ImGui::PopStyleColor();

		++cpt;
	}

	if (m_selected >= (int)cityListFiltered.size())
		m_selected = -1;

	sBoroughData wholeCityData;
	sCityData selectedCity;
	if (m_selected > -1)
		DatabaseManager::getSingleton()->GetCityData(cityListFiltered[m_selected], selectedCity, &wholeCityData);

	ImGui::EndChild();
	ImGui::SameLine();

	// Right panel (infos display)
	ImGui::BeginChild("Item view", ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing())); // Leave room for 1 line below us

	if (!selectedCity.m_data.m_name.empty())
	{
		ImGui::Text("Name: %s    ZipCode: %d   Insee code : %d", selectedCity.m_data.m_name.c_str(), selectedCity.m_data.m_zipCode, selectedCity.m_data.m_inseeCode);
		bool hovered = ImGui::IsItemHovered();
		if (hovered)
			DisplayPricesTooltip(wholeCityData);

		if (!DatabaseManager::getSingleton()->IsCityUpdating(selectedCity.m_data.m_name))
		{
			ImGui::SameLine();
			if (ImGui::Button("Update boroughs list"))
				DatabaseManager::getSingleton()->ComputeCityData(selectedCity.m_data.m_name);

			ImGui::SameLine();
			if (ImGui::Button("Update city average price"))
			{
				sBoroughData data;
				data.m_city = selectedCity.m_data;
				data.m_name = s_wholeCityName;
				DatabaseManager::getSingleton()->ComputeBoroughData(data);
			}

			if (ImGui::TreeNode("Boroughs"))
			{
				int cpt = 0;
				for (auto& borough : selectedCity.m_boroughs)
				{
					ImGui::PushID(this + cpt);
					bool updating = DatabaseManager::getSingleton()->IsBoroughUpdating(borough);
					bool update = false;
					if (!updating)
						update = ImGui::Button("Update");
					else
						ImGui::Text("Updating...");

					ImGui::PopID();

					ImGui::SameLine();

					ImGui::Text("%s", borough.m_name.c_str());
					hovered = ImGui::IsItemHovered();
					if (hovered)
						DisplayPricesTooltip(borough);

					if (update)
						DatabaseManager::getSingleton()->ComputeBoroughData(borough);

					++cpt;
				}
				ImGui::TreePop();
			}
		}
		else
		{
			char buf[128];
			sprintf_s(buf, "Updating boroughs list... %c", "|/-\\"[(int)(ImGui::GetTime() / 0.1f) & 3]);
			ImGui::Text(buf);
		}
	}

	ImGui::EndChild();

	ImGui::End();
}