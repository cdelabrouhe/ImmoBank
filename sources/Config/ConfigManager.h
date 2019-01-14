#pragma once

#include <extern/jsoncpp/value.h>

namespace ImmoBank
{
	//-------------------------------------------------------------------------------------------------
	// DATA
	//-------------------------------------------------------------------------------------------------
	class ConfigManager
	{
	public:
		static ConfigManager* getSingleton();

		void	Init();
		void	End();

		void	Save();

		const Json::Value* GetEntry(const std::string& _name);

		void SetEntryData(const std::string& _name, const std::string& _data);

	protected:
		Json::Value m_data;
	};
}