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

	// Only update city data when changed
	static std::string s_currentSelection;
	static BoroughData wholeCityData;
	static sCityData selectedCity;
	if (m_selected > -1)
	{
		if (s_currentSelection != cityListFiltered[m_selected])
		{
			s_currentSelection = cityListFiltered[m_selected];
			DatabaseManager::getSingleton()->GetCityData(s_currentSelection, selectedCity, &wholeCityData);
			wholeCityData.m_city = selectedCity.m_data;
			wholeCityData.SetWholeCity();
		}
	}

	ImGui::EndChild();
	ImGui::SameLine();

	// Right panel (infos display)
	ImGui::BeginChild("Item view", ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing())); // Leave room for 1 line below us

	bool hovered = false;
	if (!selectedCity.m_data.m_name.empty())
	{
		ImGui::Text("Name: %s    ZipCode: %d   Insee code : %d", selectedCity.m_data.m_name.c_str(), selectedCity.m_data.m_zipCode, selectedCity.m_data.m_inseeCode);
		
		wholeCityData.DisplayAsTooltip();

		if (!DatabaseManager::getSingleton()->IsCityUpdating(selectedCity.m_data.m_name))
		{
			if (ImGui::Button("Update boroughs list"))
				DatabaseManager::getSingleton()->ComputeCityData(selectedCity.m_data.m_name);

			ImGui::SameLine();
			if (ImGui::Button("Auto update city price"))
			{
				BoroughData data;
				data.m_city = selectedCity.m_data;
				data.m_name = s_wholeCityName;
				DatabaseManager::getSingleton()->ComputeBoroughData(data);
			}

			ImGui::SameLine();

			static BoroughData* s_selectedData = nullptr;
			if (ImGui::Button("Manual edit city price"))
			{
				ImGui::OpenPopup("ManualEdit");
				s_selectedData = &wholeCityData;
			}

			if (s_selectedData == &wholeCityData)
				wholeCityData.Edit();

			if (ImGui::TreeNode("Boroughs"))
			{
				int cpt = 0;
				for (auto& borough : selectedCity.m_boroughs)
				{
					bool updating = DatabaseManager::getSingleton()->IsBoroughUpdating(borough);
					bool update = false;
					bool manual = false;
					if (!updating)
					{
						ImGui::PushID(this + cpt);
						update = ImGui::Button("Auto update");
						ImGui::PopID();

						ImGui::SameLine();

						ImGui::PushID(this + cpt + 10000);
						manual = ImGui::Button("Manual update");
						ImGui::PopID();

						if (manual)
						{
							ImGui::OpenPopup("ManualEdit");
							s_selectedData = &borough;
						}

						if (s_selectedData == &borough)
							borough.Edit();
					}
					else
						ImGui::Text("Updating...");
					
					ImGui::SameLine();

					ImGui::Text("%s", borough.m_name.c_str());
					borough.DisplayAsTooltip();

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