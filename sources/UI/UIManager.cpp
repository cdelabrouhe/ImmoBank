#include "UIManager.h"

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <algorithm>

#include "extern/ImGui/imgui.h"
#include "Request/RequestManager.h"
#include "Database/DatabaseManager.h"
#include "CitySelector.h"
#include "Tools/StringTools.h"
#include "Tools/Tools.h"
#include "Text/TextManager.h"

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
		if (ImGui::BeginMenu("Request"))
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
			if (ImGui::MenuItem("Explore database"))
				UIManager::getSingleton()->AskForDisplayCityInformation();

			ImGui::EndMenu();
		}

		bool openRate = false;
		if (ImGui::BeginMenu("Tools"))
		{
			if (ImGui::MenuItem("Compute rentability rate"))
				openRate = true;

			ImGui::EndMenu();
		}

		if (openRate)
			ImGui::OpenPopup("Compute rate");

		DisplayComputeRateTool();

		if (ImGui::BeginMenu("Window"))
		{
			if (ImGui::BeginMenu("Language"))
			{
				std::vector<std::string> languages;
				TextManager::getSingleton()->GetLanguagesList(languages);

				for (auto lang : languages)
				{
					if (ImGui::MenuItem(lang.c_str()))
						TextManager::getSingleton()->ChangeLanguage(lang);
				}
				ImGui::EndMenu();
			}

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

#ifdef DEV_MODE
		if (ImGui::BeginMenu("DEBUG"))
		{
			ImGui::MenuItem("Display MySQL Debug", nullptr, &DatabaseManager::getSingleton()->m_displayDebug);

			ImGui::EndMenu();
		}
#endif

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
		std::sort(m_cityListFull.begin(), m_cityListFull.end(), sCity::compare);
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
		for (auto& city : m_cityListFull)
		{
			std::string tmp = city.m_name;
			StringTools::TransformToLower(tmp);
			auto findID = tmp.find(s_citySelector.GetText());
			if (findID != std::string::npos)
				cityListFiltered.push_back(city.m_name);
		}

		listUpdated = true;
	}
	ImGui::Separator();

	if (!listUpdated)
	{
		for (auto& city : m_cityListFull)
			cityListFiltered.push_back(city.m_name);
	}

	int localHovered = -1;
	int cpt = 0;
	const ImColor colSelected(1.0f, 0.0f, 0.0f);
	const ImColor colHovered(0.0f, .5f, .5f);
	for (auto city : cityListFiltered)
	{
		const bool isHovered = (m_hovered == cpt);
		const bool isSelected = (m_selected == cpt);
		if (isSelected)
			ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)colSelected);
		else if (isHovered)
			ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)colHovered);

		StringTools::ConvertToImGuiText(city);
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
	bool forceUpdate = DatabaseManager::getSingleton()->IsModified();
	static std::string s_currentSelection;
	static BoroughData wholeCityData;
	static sCityData selectedCity;
	if (m_selected > -1)
	{
		if (forceUpdate || (s_currentSelection != cityListFiltered[m_selected]))
		{
			s_currentSelection = cityListFiltered[m_selected];
			wholeCityData.Reset();
			DatabaseManager::getSingleton()->GetCityData(s_currentSelection, selectedCity, &wholeCityData);
			selectedCity.m_data.FixName();
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
		std::string name = selectedCity.m_data.m_name;
		StringTools::ConvertToImGuiText(name);
		ImGui::Text("Name: %s    ZipCode: %d   Insee code : %d", name.c_str(), selectedCity.m_data.m_zipCode, selectedCity.m_data.m_inseeCode);
		
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

			ImGui::SameLine();

			if (ImGui::Button("Link"))
				wholeCityData.OpenInBrowser();;

#ifdef DEV_MODE
			ImGui::SameLine();

			if (ImGui::Button("Del Data"))
				wholeCityData.Reset(true);
#endif

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
						manual = ImGui::Button("Edit");
						ImGui::PopID();

						ImGui::SameLine();

						ImGui::PushID(this + cpt + 20000);
						if (ImGui::Button("Link"))
							borough.OpenInBrowser();
						ImGui::PopID();

#ifdef DEV_MODE
						ImGui::SameLine();

						ImGui::PushID(this + cpt + 30000);
						if (ImGui::Button("DelData"))
							borough.Reset(true);
						ImGui::PopID();
#endif

						if (manual)
						{
							ImGui::OpenPopup("ManualEdit");
							s_selectedData = &borough;
						}

						if (s_selectedData == &borough)
							borough.Edit();

						ImGui::SameLine();

						ImGui::Text("%s", borough.m_name.c_str());
						borough.DisplayAsTooltip();
					}
					else
					{
						char buf[128];
						sprintf_s(buf, "Updating %s... %c ", borough.m_name.c_str(), "|/-\\"[(int)(ImGui::GetTime() / 0.1f) & 3]);
						ImGui::Text(buf);
					}
					
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

//-------------------------------------------------------------------------------------------------
void UIManager::DisplayComputeRateTool()
{
	if (ImGui::BeginPopupModal("Compute rate", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::SetWindowFontScale(1.f);

		static int s_rent = 700;
		static int s_price = 150000;
		static float s_result = 0.f;
		ImGui::InputInt("Rent", &s_rent);
		ImGui::InputInt("Price", &s_price);

		ImGui::Separator();
		ImGui::Text("Rate: %.2f", s_result);
		ImGui::Separator();

		if (ImGui::Button("Compute"))
			s_result = Tools::ComputeRentabilityRate((float)s_rent, (float)s_price);

		ImGui::SameLine();
		if (ImGui::Button("Exit"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
}

bool UIManager::DisplayConnectionError()
{
	bool result = false;
	if (!m_connectionError)
	{
		m_connectionError = true;
		ImGui::OpenPopup("Connection error");
	}

	if (ImGui::BeginPopupModal("Connection error", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		std::string server, user;
		DatabaseManager::getSingleton()->GetConnectionParameters(server, user);
		ImGui::Text("Can't connect to server %s with user %s", server.c_str(), user.c_str());
		if (ImGui::Button("Retry"))
		{
			ImGui::CloseCurrentPopup();
			result = true;
		}
		ImGui::EndPopup();
	}

	return result;
}