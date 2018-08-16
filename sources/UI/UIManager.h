#pragma once

#include <string>
#include <vector>
#include "Tools\Types.h"

//-------------------------------------------------------------------------------------------------
// DATA
//-------------------------------------------------------------------------------------------------
struct ImFont;
class UIManager
{
public:
	static UIManager* getSingleton();

	ImFont*		FontDefault;

	UIManager();

	bool	Draw();

	void	Process();

	void	AskForDisplayCityInformation();
	void	InitDisplayCityInformation();
	void	DisplayCityInformation();
	void	DisplayComputeRateTool();
	bool	DisplayConnectionError();

#ifdef DEV_MODE
	bool	IsDisplayMySQLDebug();
	void	NotifyMySQLEvent(const std::string& _request);
	void	DisplayMySQLRequestsPanel();
#endif

private:
	// Display panel
	int								m_selectedCityID = 0;
	std::vector<sCity>				m_cityListFull;
	int								m_hovered = -1;
	int								m_selected = -1;

	bool							m_displayCityData = false;
	bool							m_cityListRequested = false;

	bool							m_connectionError = false;

#ifdef DEV_MODE
	std::vector<std::string>		m_MySQLRequests;
#endif
};