#pragma once

#include <map>

namespace ImmoBank
{
	class ImageDatabase
	{
	public:
		static ImageDatabase* getSingleton();

		void Init();
		void Process();

		bool HasImage(const std::string& _URL) const;
		unsigned char* GetImage(const std::string& _URL, int& _size) const;
		void StoreImage(const std::string& _URL, unsigned char* _buffer, int _bufferSize);
		void RemoveImage(const std::string& _URL);

	private:
		void _Check();
		void _Read();
		void _Write();
		std::string GeneratePath();

	private:
		std::map<std::string, std::string>	m_data;
		bool m_modified = false;
	};
}