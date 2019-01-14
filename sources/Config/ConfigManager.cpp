#include "ConfigManager.h"
#include <Tools\Tools.h>

using namespace ImmoBank;

ConfigManager* s_singleton = nullptr;

static const char* s_textUnknownEntry = "UNKNOWN ENTRY";

//-------------------------------------------------------------------------------------------------
ConfigManager* ConfigManager::getSingleton()
{
	if (s_singleton == nullptr)
		s_singleton = new ConfigManager();
	return s_singleton;
}

//-------------------------------------------------------------------------------------------------
void ConfigManager::Init()
{
	std::string path = Tools::GetExePath();
	path += "config.cfg";
	Tools::ReadJSON(path.c_str(), m_data);
}

//-------------------------------------------------------------------------------------------------
void ConfigManager::End()
{

}

//-------------------------------------------------------------------------------------------------
void ImmoBank::ConfigManager::Save()
{
	std::string path = Tools::GetExePath();
	path += "config.cfg";
	Tools::WriteJSON(path.c_str(), m_data);
}

//-------------------------------------------------------------------------------------------------
const Json::Value* ImmoBank::ConfigManager::GetEntry(const std::string& _name)
{
	Json::Value val = m_data.get(_name.c_str(), Json::nullValue);
	if (val != Json::nullValue)
		return &m_data[_name];
	return nullptr;
}

//-------------------------------------------------------------------------------------------------
void ImmoBank::ConfigManager::SetEntryData(const std::string& _name, const std::string& _data)
{
	m_data[_name] = _data;

	Save();
}
