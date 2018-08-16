#include <windows.h>
#include <stdio.h>

#include "GL/ProdToolGL.h"

#include "Request/RequestManager.h"
#include "Online/OnlineManager.h"
#include "Database/DatabaseManager.h"
#include "UI/UIManager.h"
#include "Tools/Types.h"

const int	CONFIG_WINDOW_WIDTH = 1280;
const int	CONFIG_WINDOW_HEIGHT = 768;

int main(int argc, char** argv)
{
#ifndef DEV_MODE
	FreeConsole();
#endif
		
	// Setup window
	HWND hwnd = 0;
	if (!ProdToolGL_InitCreateWindow(CONFIG_WINDOW_WIDTH, CONFIG_WINDOW_HEIGHT))
		return 1;

	ProdToolGL_InitImGui();
	ProdToolGL_GetHwnd(&hwnd);

	// Init Online
	OnlineManager::getSingleton()->Init();

	// Init DB
	DatabaseManager::getSingleton()->Init();

	// Init RequestManager
	RequestManager::getSingleton()->Init();

	// Main loop
	bool quit = false;
	while (!quit && !ProdToolGL_ShouldClose())
	{
		ProdToolGL_NewFrame();

		OnlineManager::getSingleton()->Process();
		RequestManager::getSingleton()->Process();
		DatabaseManager::getSingleton()->Process();
		UIManager::getSingleton()->Process();

		const bool is_minimized = ProdToolGL_IsMinimized();
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

	ProdToolGL_Shutdown();

	return 0;
}