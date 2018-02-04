#pragma once

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
};