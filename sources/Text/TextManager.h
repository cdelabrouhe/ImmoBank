#pragma once

#include <string>
#include <map>
#include <vector>

#define GET_TEXT(name)		TextManager::getSingleton()->GetEntryText(name)

//-------------------------------------------------------------------------------------------------
// DATA
//-------------------------------------------------------------------------------------------------
class TextManager
{
public:
	static TextManager* getSingleton();

	void	Init();
	void	End();

	inline void GetLanguagesList(std::vector<std::string>& _list) const	{ _list = m_languages;	}
	void ChangeLanguage(const std::string& _newLanguage);

	const char* GetEntryText(const char* _entryName) const;
	const char* GetEntryText(const std::string& _entryName) const;

private:
	const char* GetEntryText(unsigned int _entryHashName) const;

protected:
	std::string		m_language;
	unsigned int	m_languageIndex = 0xFFFFFFFF;
	std::map<int, std::map<unsigned int, std::string>>	m_database;
	std::vector<std::string> m_languages;
};