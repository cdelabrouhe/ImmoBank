#pragma once

#include <string>
#include <vector>

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

private:
	// Display panel
	int								m_selectedCityID = 0;
	std::vector<std::string>		m_cityListFull;
	int								m_hovered = -1;
	int								m_selected = -1;

	bool							m_displayCityData = false;
	bool							m_cityListRequested = false;
};