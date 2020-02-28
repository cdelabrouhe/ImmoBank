#pragma once

#include <map>
#include <sstream>

namespace ImmoBank
{
	class ImageDatabase
	{
	public:
		static ImageDatabase* getSingleton();

		void Init();
		void Process();
		void End();

		bool HasImage(const std::string& _URL) const;
		unsigned char* GetImage(const std::string& _URL, int& _size) const;
		void StoreImage(const std::string& _URL, unsigned char* _buffer, int _bufferSize);
		void RemoveURL(const std::string& _URL);
		void RemoveFile(const std::string& _filePath);
		void ReferenceImage(const std::string& _URL, const std::string& _filePath);
		std::string GenerateNewImageFullPath(const std::string& _URL);

	private:
		void _Check();
		void _ReadDataFile();
		void _WriteDataFile();
		void _Test();
		std::string _GeneratePath();

	private:
		std::map<std::string, std::string>	m_data;
		std::string m_imagesPath;
		bool m_modified = false;
	};
}