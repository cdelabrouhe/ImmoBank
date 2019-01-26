#include <windows.h>
#include <stdio.h>

#include "GL/ProdToolGL.h"

#include "Request/RequestManager.h"
#include "Online/OnlineManager.h"
#include "Database/DatabaseManager.h"
#include "UI/UIManager.h"
#include "Tools/Types.h"
#include "Text/TextManager.h"
#include <Config/ConfigManager.h>

const int	CONFIG_WINDOW_WIDTH = 1600;
const int	CONFIG_WINDOW_HEIGHT = 900;

#define FREE_CONSOLE

using namespace ImmoBank;

int main(int argc, char** argv)
{
	bool quit = false;

	ConfigManager::getSingleton()->Init();

	// Init DB
	DatabaseManager::getSingleton()->Init();
	bool connectionValid = DatabaseManager::getSingleton()->IsConnectionValid();
	if (!connectionValid)
	{
		printf("Error initializing DB\n");
		quit = true;
		system("pause");
	}

#ifdef FREE_CONSOLE
	FreeConsole();
#endif

	// Setup window
	HWND hwnd = 0;
	if (!ProdToolGL_InitCreateWindow(CONFIG_WINDOW_WIDTH, CONFIG_WINDOW_HEIGHT))
		return 1;

	ProdToolGL_InitImGui();
	ProdToolGL_GetHwnd(&hwnd);

	// Init Online
	TextManager::getSingleton()->Init();

	// Init Online
	OnlineManager::getSingleton()->Init();

	// Init RequestManager
	RequestManager::getSingleton()->Init();

	// Main loop
	while (!quit && !ProdToolGL_ShouldClose())
	{
		ProdToolGL_NewFrame();

		if (connectionValid)
		{
			OnlineManager::getSingleton()->Process();
			RequestManager::getSingleton()->Process();
			DatabaseManager::getSingleton()->Process();
			UIManager::getSingleton()->Process();
		}
		else
		{
			if (UIManager::getSingleton()->DisplayConnectionError())
			{
				DatabaseManager::getSingleton()->Init();
				connectionValid = DatabaseManager::getSingleton()->IsConnectionValid();
			}
		}		

		const bool is_minimized = false;// ProdToolGL_IsMinimized();
		bool want_refresh = true;
		if (is_minimized)
			want_refresh = false;

		// UI refresh
		if (want_refresh)
		{
			quit = UIManager::getSingleton()->Draw();
		}

		// GPU render and swap
		// Swap appears to be costly depending on drivers/settings, so we aim to avoid it when possible.
		if (want_refresh)
			ProdToolGL_Render();
	}

	// End Online
	OnlineManager::getSingleton()->End();

	// End DB
	DatabaseManager::getSingleton()->End();

	// End Request manager
	RequestManager::getSingleton()->End();

	// End Text manager
	TextManager::getSingleton()->End();

	// End Config manager
	ConfigManager::getSingleton()->End();

	ProdToolGL_Shutdown();

	return 0;
}