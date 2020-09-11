#include "UIManager.h"

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <algorithm>

#include "extern/ImGui/imgui.h"
#include "Request/RequestManager.h"
#include "Database/DatabaseManager.h"
#include "Online/OnlineManager.h"
#include "CitySelector.h"
#include "Tools/StringTools.h"
#include "Tools/Tools.h"
#include "Text/TextManager.h"
#include <GL/ProdToolGL.h>
#include "GLFW/glfw3.h"
#include <shellapi.h>
#include <Online/OnlineDatabase.h>
#include <Online/PapOnlineDatabase.h>
#include <Online/LogicImmoOnlineDatabase.h>

using namespace ImmoBank;

//-------------------------------------------------------------------------------------------------
// FORWARD DECLARATIONS
//-------------------------------------------------------------------------------------------------
UIManager* s_singleton = nullptr;
static bool s_computeRentabilityRate = false;
static int s_rentabilityToolRent = 700;
static int s_rentabilityToolPrice = 150000;
static float s_rentabilityToolResult = 0.f;

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

	s_rentabilityToolResult = Tools::ComputeRentabilityRate((float)s_rentabilityToolRent, (float)s_rentabilityToolPrice);
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
	ImGui::Begin(GET_TEXT("MainWindow"), NULL, ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoFocusOnAppearing|ImGuiWindowFlags_NoBringToFrontOnFocus|ImGuiWindowFlags_MenuBar);

	static bool showTestWindow = false;
	static bool showAbout = false;
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu(GET_TEXT("MainMenuRequest")))
		{
			if (ImGui::MenuItem(GET_TEXT("MenuRequesetNewRequest")))
				RequestManager::getSingleton()->CreateRequestAnnounceDefault();

			ImGui::Separator();
			if (ImGui::MenuItem(GET_TEXT("GeneralExit"), "ALT+F4"))
				quit = true;

			ImGui::EndMenu();
		}

		if (Tools::IsViewAllowed() && ImGui::BeginMenu(GET_TEXT("MainMenuDatabase")))
		{
			if (ImGui::MenuItem(GET_TEXT("MenuDatabaseExploreDB")))
				UIManager::getSingleton()->AskForDisplayCityInformation();

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu(GET_TEXT("MainMenuTools")))
		{
			if (ImGui::MenuItem(GET_TEXT("MenuToolsComputeRentabilityRate")))
				s_computeRentabilityRate = true;

			ImGui::EndMenu();
		}

		if (s_computeRentabilityRate)
			DisplayComputeRateTool(true);

		if (ImGui::BeginMenu(GET_TEXT("MainMenuWindow")))
		{
			if (ImGui::BeginMenu(GET_TEXT("MenuWindowLanguage")))
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

			if (ImGui::BeginMenu(GET_TEXT("MenuWindowStyle")))
			{
				if (ImGui::MenuItem(GET_TEXT("MenuWindowStyleClassic")))
					ImGui::StyleColorsClassic();

				if (ImGui::MenuItem(GET_TEXT("MenuWindowStyleDark")))
					ImGui::StyleColorsDark();

				if (ImGui::MenuItem(GET_TEXT("MenuWindowStyleLight")))
					ImGui::StyleColorsLight();

				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("?"))
		{
			if (Tools::IsDevMode())
			{
				if (ImGui::MenuItem(GET_TEXT("MenuHelpShowTestWindow")))
					showTestWindow = !showTestWindow;
			}

			if (ImGui::MenuItem(GET_TEXT("MenuHelpShowAbout")))
				showAbout = !showAbout;
			ImGui::EndMenu();
		}

		if (Tools::IsDevMode())
		{
			if (ImGui::BeginMenu(GET_TEXT("MenuDebug")))
			{
				ImGui::MenuItem(GET_TEXT("MenuDebugDisplayMySQLDebug"), nullptr, &DatabaseManager::getSingleton()->m_displayDebugMySQL);
				ImGui::MenuItem("SQlite3 debug panel", nullptr, &DatabaseManager::getSingleton()->m_displayDebugSQLite3);
				ImGui::MenuItem("OnlineManager debug panel", nullptr, &OnlineManager::getSingleton()->m_displayDebug);
				ImGui::MenuItem("GenerateZipCodes", nullptr, &DatabaseManager::getSingleton()->m_generateZipCodesIndices);
				ImGui::MenuItem("UpdateLocalBaseToServer", nullptr, &DatabaseManager::getSingleton()->m_updateLocalBaseToServer);
				ImGui::MenuItem("UpdateServerToLocalBase", nullptr, &DatabaseManager::getSingleton()->m_updateServerToLocalBase);

				if (ImGui::BeginMenu("Databases"))
				{
					auto& dbs = OnlineManager::getSingleton()->GetOnlineDatabases();
					for (auto* db : dbs)
					{
						if (ImGui::BeginMenu(db->GetName().c_str()))
						{
							if (bool* update = db->ForceUpdate())
							{
								std::string name = "Force update internal data";
								ImGui::MenuItem(name.c_str(), nullptr, update);
							}

							if (ImGui::MenuItem("Update data from server"))
								db->UpdateFromExternalDatabase();

							ImGui::EndMenu();
						}
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}
		}

		ImGui::EndMenuBar();
	}

	RequestManager::getSingleton()->DisplayRequests();

	if (showTestWindow)
		ImGui::ShowTestWindow();

	if (showAbout)
	{
		ImGui::Begin("About ImmoBank", &showAbout, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("                     Immobank version %.2f", Tools::GetVersionNumber());
		ImGui::Separator();
		ImGui::Text("Copyright 2018-2019 - Christophe de Labrouhe");
		ImGui::TextColored(ImVec4(0.f,0.5f,1.f,1.f), "                    mastertof@hotmail.com");
		if (ImGui::IsItemClicked())
			ShellExecuteA(NULL, "open", "mailto:mastertof@hotmail.com", NULL, NULL, SW_SHOWDEFAULT);

		ImGui::End();
	}

	ImGui::End();
	ImGui::PopStyleVar();

	return quit;
}

//-------------------------------------------------------------------------------------------------
void UIManager::Process()
{
	if (m_displayCityData)
		DisplayCityInformation();

	// Activate dev mode
	static bool s_block = false;
	if (!s_block && ImGui::IsKeyPressed(GLFW_KEY_LEFT_CONTROL) && ImGui::IsKeyPressed(GLFW_KEY_LEFT_SHIFT) && ImGui::IsKeyPressed(GLFW_KEY_D))
	{
		s_block = true;
		Tools::InvertDevMode();
	}
	else if (s_block)
	{
		if (!ImGui::IsKeyPressed(GLFW_KEY_D))
			s_block = false;
	}
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
	ImGui::Begin(GET_TEXT("DatabaseWindowName"), &m_displayCityData);

	// Left panel (city selector process)
	bool listUpdated = false;
	std::vector<sCity> cityListFiltered;
	ImGui::BeginChild(GET_TEXT("DatabaseWindowSearchCity"), ImVec2(300, 0), true);

	s_citySelector.Display();

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
	for (auto city : cityListFiltered)
	{
		const bool isHovered = (m_hovered == cpt);
		const bool isSelected = (m_selected == cpt);
		if (isSelected)
			ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)colSelected);
		else if (isHovered)
			ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)colHovered);

		StringTools::ConvertToImGuiText(city.m_name);
		ImGui::Text("%s (%d)", city.m_name.c_str(), city.m_zipCode);
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
	static sCity s_currentSelection;
	static BoroughData wholeCityData;
	static sCityData selectedCity;
	if (m_selected > -1)
	{
		sCity cityFiletered = cityListFiltered[m_selected];
		if (forceUpdate || (s_currentSelection != cityFiletered))
		{
			s_currentSelection = cityFiletered;
			wholeCityData.Reset();
			DatabaseManager::getSingleton()->GetCityData(s_currentSelection.m_name, s_currentSelection.m_zipCode, selectedCity, &wholeCityData);
			selectedCity.m_data.FixName();
			wholeCityData.m_city = selectedCity.m_data;
			wholeCityData.SetWholeCity();

			DatabaseManager::getSingleton()->UpdateCityData(selectedCity.m_data);
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
		if (Tools::IsDevMode())
		{
			char buf[2048];
			sprintf(buf, "%s: %s    %s: %d   %s: %d"
				, GET_TEXT("DatabaseWindowCityName")
				, name.c_str()
				, GET_TEXT("DatabaseWindowZipCode")
				, selectedCity.m_data.m_zipCode
				, GET_TEXT("DatabaseWindowInseeCode")
				, selectedCity.m_data.m_inseeCode);

			std::string text(buf);

			auto& dbs = OnlineManager::getSingleton()->GetOnlineDatabases();
			for (auto* db : dbs)
			{
				if (db->HasKey())
				{
					text += "  " + db->GetName() + " key: " + db->GetKeyAsString(wholeCityData);
				}
			}

			ImGui::Text("%s", text.c_str());
		}
		else
		{
			ImGui::Text("%s: %s    %s: %d   %s: %d", GET_TEXT("DatabaseWindowCityName")
				, name.c_str()
				, GET_TEXT("DatabaseWindowZipCode")
				, selectedCity.m_data.m_zipCode
				, GET_TEXT("DatabaseWindowInseeCode")
				, selectedCity.m_data.m_inseeCode);
		}
		
		wholeCityData.DisplayAsTooltip();

		if (!DatabaseManager::getSingleton()->IsCityUpdating(selectedCity.m_data.m_name))
		{
			if (Tools::IsEditAllowed() && ImGui::Button(GET_TEXT("DatabaseWindowCityUpdateBoroughList")))
				DatabaseManager::getSingleton()->ComputeCityData(selectedCity.m_data);

			ImGui::SameLine();
			if (Tools::IsEditAllowed() && ImGui::Button(GET_TEXT("DatabaseWindowCityAutoUpdatePrice")))
			{
				BoroughData data;
				data.m_city = selectedCity.m_data;
				data.m_name = s_wholeCityName;
				DatabaseManager::getSingleton()->ComputeBoroughData(data);
			}

			ImGui::SameLine();

			static BoroughData* s_selectedData = nullptr;
			if (Tools::IsEditAllowed() && ImGui::Button(GET_TEXT("DatabaseWindowCityManualUpdatePrice")))
			{
				ImGui::OpenPopup(GET_TEXT("BoroughManualEditPopup"));
				s_selectedData = &wholeCityData;
			}

			ImGui::SameLine();

			if (ImGui::Button(GET_TEXT("DatabaseWindowCityLink")))
				wholeCityData.OpenInBrowser();;

			if (Tools::IsDevMode())
			{
				ImGui::SameLine();

				if (ImGui::Button(GET_TEXT("DatabaseWindowCityDeleteData")))
					wholeCityData.Reset(true);

				ImGui::SameLine();

				if (ImGui::Button(GET_TEXT("DatabaseWindowBoroughExternalUpdateList")))
					DatabaseManager::getSingleton()->UpdateCityData(selectedCity.m_data);
			}

			if (s_selectedData == &wholeCityData)
				wholeCityData.Edit();
			else
				wholeCityData.End();

			ImGui::Separator();

			int cpt = 0;
			for (auto& borough : selectedCity.m_boroughs)
			{
				bool updating = DatabaseManager::getSingleton()->IsBoroughUpdating(borough);
				bool update = false;
				bool manual = false;
				if (!updating)
				{
					if (Tools::IsEditAllowed())
					{
						ImGui::PushID(this + cpt);
						update = ImGui::Button(GET_TEXT("DatabaseWindowBoroughAutoUpdatePrice"));
						ImGui::PopID();

						ImGui::SameLine();

						ImGui::PushID(this + cpt + 10000);
						manual = ImGui::Button(GET_TEXT("DatabaseWindowBoroughManualUpdatePrice"));
						ImGui::PopID();

						ImGui::SameLine();
					}

					ImGui::PushID(this + cpt + 20000);
					if (ImGui::Button(GET_TEXT("DatabaseWindowBoroughLink")))
						borough.OpenInBrowser();
					ImGui::PopID();

					if (Tools::IsDevMode())
					{
						ImGui::SameLine();

						ImGui::PushID(this + cpt + 30000);
						if (ImGui::Button(GET_TEXT("DatabaseWindowBoroughDeleteData")))
							borough.Reset(true);
						ImGui::PopID();
					}

					if (manual)
					{
						ImGui::OpenPopup(GET_TEXT("BoroughManualEditPopup"));
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
					sprintf_s(buf, "%s %s... %c ", GET_TEXT("GeneralUpdating"), borough.m_name.c_str(), "|/-\\"[(int)(ImGui::GetTime() / 0.1f) & 3]);
					ImGui::Text(buf);
				}

				if (update)
					DatabaseManager::getSingleton()->ComputeBoroughData(borough);

				++cpt;
			}
		}
		else
		{
			char buf[128];
			sprintf_s(buf, "%s... %c", GET_TEXT("DatabaseWindowCityUpdatingBoroughList"), "|/-\\"[(int)(ImGui::GetTime() / 0.1f) & 3]);
			ImGui::Text(buf);
		}
	}

	ImGui::EndChild();

	ImGui::End();
}

//-------------------------------------------------------------------------------------------------
void UIManager::ForceRentabilityRateToolValues(int _price, int _rent)
{
	s_rentabilityToolRent = _rent;
	s_rentabilityToolPrice = _price;
	s_rentabilityToolResult = Tools::ComputeRentabilityRate((float)s_rentabilityToolRent, (float)s_rentabilityToolPrice);
}

//-------------------------------------------------------------------------------------------------
void UIManager::DisplayComputeRateTool(bool _independantWindow)
{
	if (_independantWindow ? ImGui::Begin(GET_TEXT("PopupComputeRentabilityRate"), &s_computeRentabilityRate, ImGuiWindowFlags_AlwaysAutoResize) : true)
	{
		ImGui::SetWindowFontScale(1.f);

		ImGui::InputInt(GET_TEXT("GeneralRent"), &s_rentabilityToolRent);
		ImGui::InputInt(GET_TEXT("GeneralPrice"), &s_rentabilityToolPrice);

		ImGui::Separator();
		ImGui::Text("%s: %.2f", GET_TEXT("GeneralRate"), s_rentabilityToolResult);
		ImGui::Separator();

		if (ImGui::Button(GET_TEXT("GeneralCompute")))
			s_rentabilityToolResult = Tools::ComputeRentabilityRate((float)s_rentabilityToolRent, (float)s_rentabilityToolPrice);

		if (_independantWindow)
		{
			ImGui::SameLine();
			if (ImGui::Button(GET_TEXT("GeneralExit")))
				s_computeRentabilityRate = false;
		}
	}

	if (_independantWindow)
		ImGui::End();
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