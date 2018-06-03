#include "UIManager.h"

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <algorithm>

#include "extern/ImGui/imgui.h"
#include "Request/RequestManager.h"
#include "Database/DatabaseManager.h"

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
				DatabaseManager::getSingleton()->AskForDisplayCityInformation();

			ImGui::Separator();
			if (ImGui::MenuItem("Exit", "ALT+F4"))
				quit = true;

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
