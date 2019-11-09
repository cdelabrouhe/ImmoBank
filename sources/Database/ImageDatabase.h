#pragma once

#include <map>

namespace ImmoBank
{
	class ImageDatabase
	{
	public:
		void Init();
		void Process();

		bool HasImage(const std::string& _URL) const;
		void StoreImage(const std::string& _URL, const std::string& _path);

	private:
		void _Read();
		void _Write();

	private:
		std::map<std::string, std::string>	m_data;
		bool m_modified = false;
	};
}