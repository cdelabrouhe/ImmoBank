#include "TextManager.h"
#include <extern/jsoncpp/value.h>
#include <Tools/Tools.h>
#include <Tools/SearchFile.h>
#include "Tools/StringTools.h"
#include <Config/ConfigManager.h>

using namespace ImmoBank;

TextManager* s_singleton = nullptr;

static const char* s_textUnknownEntry = "UNKNOWN ENTRY";

//-------------------------------------------------------------------------------------------------
TextManager* TextManager::getSingleton()
{
	if (s_singleton == nullptr)
		s_singleton = new TextManager();
	return s_singleton;
}

//-------------------------------------------------------------------------------------------------
void TextManager::Init()
{
	// Load databases
	std::string path = Tools::GetExePath();
	path += "lang/";
	SearchFile search;
	std::string phoSearchPattern = "*.json";
	search("", phoSearchPattern, path);

	for (unsigned int i = 0, n = search.count(); i < n; ++i)
	{
		std::string language = search[i];
		language = language.substr(0, language.find_first_of("."));
		m_languages.push_back(language);
		unsigned int languageIndex = StringTools::GenerateHash(language);
		std::map<unsigned int, std::string>& languageEntry = m_database[languageIndex];
		std::string file = path + search[i];
		Json::Value root;
		if (Tools::ReadJSON(file.c_str(), root))
		{
			auto list = root.getMemberNames();
			for (auto name : list)
			{
				unsigned int nameHash = StringTools::GenerateHash(name);
				languageEntry[nameHash] = root[name].asString();
			}
		}
	}

	auto data = ConfigManager::getSingleton()->GetEntry("lang");
	if (data)
		ChangeLanguage(data->asString());
	else
	{
		// Auto select first language
		if (m_database.size() > 0)
			m_languageIndex = m_database.begin()->first;
	}
}

//-------------------------------------------------------------------------------------------------
void TextManager::End()
{

}

//-------------------------------------------------------------------------------------------------
void TextManager::ChangeLanguage(const std::string& _newLanguage)
{
	ConfigManager::getSingleton()->SetEntryData("lang", _newLanguage);

	unsigned int hashLang = StringTools::GenerateHash(_newLanguage);
	auto it = m_database.find(hashLang);
	if (it != m_database.end())
		m_languageIndex = it->first;
}

//-------------------------------------------------------------------------------------------------
const char* TextManager::GetEntryText(const char* _entryName) const
{
	unsigned int hashName = StringTools::GenerateHash(_entryName);
	return GetEntryText(hashName);
}

//-------------------------------------------------------------------------------------------------
const char* TextManager::GetEntryText(const std::string& _entryName) const
{
	unsigned int hashName = StringTools::GenerateHash(_entryName);
	return GetEntryText(hashName);
}

//-------------------------------------------------------------------------------------------------
const char* TextManager::GetEntryText(unsigned int _entryHashName) const
{
	auto itLang = m_database.find(m_languageIndex);
	if(itLang != m_database.end())
	{
		const std::map<unsigned int, std::string>& languageEntry = itLang->second;
		auto itList = languageEntry.find(_entryHashName);
		if (itList != languageEntry.end())
		{
			return itList->second.c_str();
		}
	}
	return s_textUnknownEntry;
}