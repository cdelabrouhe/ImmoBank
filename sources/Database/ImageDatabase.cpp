#include "ImageDatabase.h"
#include "Tools\Tools.h"
#include "Tools\Types.h"
#include "Tools\SearchFile.h"
#include <windows.h>
#include "GL\ProdToolGL.h"

using namespace ImmoBank;

const std::string s_imageDatabaseFolder = "Images/";
const std::string s_imageDatabaseFileName = "images.dat";
static ImageDatabase* s_imageDatabaseSingleton = nullptr;

//-------------------------------------------------------------------------------------------------
ImageDatabase* ImageDatabase::getSingleton()
{
	if (s_imageDatabaseSingleton == nullptr)
		s_imageDatabaseSingleton = new ImageDatabase();
	return s_imageDatabaseSingleton;
}

//-------------------------------------------------------------------------------------------------
void ImageDatabase::Init()
{
	std::string exePath = Tools::GetExePath();
	m_imagesPath = exePath + s_imageDatabaseFolder;
	CreateDirectoryA(m_imagesPath.c_str(), NULL);

	_ReadDataFile();
	_Check();

	//_Test();
}

//-------------------------------------------------------------------------------------------------
void ImageDatabase::Process()
{
	if (!m_modified)
		return;

	m_modified = false;

	_WriteDataFile();
}

//-------------------------------------------------------------------------------------------------
void ImageDatabase::End()
{

}

//-------------------------------------------------------------------------------------------------
bool ImageDatabase::HasImage(const std::string& _URL) const
{
	auto it = m_data.find(_URL);
	return it != m_data.end();
}

//-------------------------------------------------------------------------------------------------
std::string ImageDatabase::_GeneratePath()
{
	std::string path = std::to_string(rand()) + ".jpg";
	std::string test = m_imagesPath + path;
	while (Tools::FileExists(test.c_str()))
	{
		path = std::to_string(rand()) + ".jpg";
		test = m_imagesPath + path;
	}
	return path;
}

//-------------------------------------------------------------------------------------------------
void ImageDatabase::StoreImage(const std::string& _URL, unsigned char* _buffer, int _bufferSize)
{
	if (HasImage(_URL))
		return;

	std::string path = _GeneratePath();
	m_data[_URL] = path;

	path = m_imagesPath + path;
	Tools::Write(path.c_str(), _buffer, _bufferSize);

	m_modified = true;
}

//-------------------------------------------------------------------------------------------------
unsigned char* ImageDatabase::GetImage(const std::string& _URL, int& _size) const
{
	auto it = m_data.find(_URL);
	if (it == m_data.end())
	{
		_size = 0;
		return nullptr;
	}

	std::string path = m_imagesPath + it->second;
	return Tools::Read(path.c_str(), _size);
}

//-------------------------------------------------------------------------------------------------
void ImageDatabase::RemoveURL(const std::string& _URL)
{
	auto it = m_data.find(_URL);
	if (it == m_data.end())
		return;

	std::string path = it->second;
	m_data.erase(it);
	path = m_imagesPath + path;
	if (!Tools::FileExists(path.c_str()))
		return;

	Tools::DeleteFileOnDisk(path.c_str());
}

//-------------------------------------------------------------------------------------------------
void ImmoBank::ImageDatabase::RemoveFile(const std::string& _filePath)
{
	std::string path = m_imagesPath + _filePath;
	if (!Tools::FileExists(path.c_str()))
		return;

	Tools::DeleteFileOnDisk(path.c_str());
}

//-------------------------------------------------------------------------------------------------
void ImmoBank::ImageDatabase::_Test()
{
	https://static.pap.fr/photos/D09/D09A1199.thumb.jpg
	std::string path = m_imagesPath + "7627.jpg";
	int size = 0;
	unsigned char* buffer = Tools::Read(path.c_str(), size);
	int width, height;
	unsigned int textureID = -1;
	if (ProdToolGL_GenerateTextureFromJPEGBuffer(buffer, size, width, height, textureID))
	{
		free(buffer);
	}
}

//-------------------------------------------------------------------------------------------------
void ImageDatabase::ReferenceImage(const std::string& _URL, const std::string& _filePath)
{
	m_data[_URL] = _filePath;
}

//-------------------------------------------------------------------------------------------------
std::string ImageDatabase::GenerateNewImageFullPath(const std::string& _URL)
{
	std::string path = _GeneratePath();
	m_data[_URL] = path;
	m_modified = true;
	return m_imagesPath + path;
}

//-------------------------------------------------------------------------------------------------
void ImageDatabase::_Check()
{
	// Clean list from URL
	std::vector<std::string> URLs;
	for (auto& pair : m_data)
	{
		std::string path = m_imagesPath + pair.second;
		if (!Tools::FileExists(path.c_str()))
			URLs.push_back(pair.first);
	}

	for (auto URL : URLs)
	{
		RemoveURL(URL);
	}

	// Clean list from files
	SearchFile search;
	std::string searchPattern = "*.jpg";
	search("", searchPattern, m_imagesPath);

	for (unsigned int i = 0, n = search.count(); i < n; ++i)
	{
		std::string path = search[i];
		auto it = m_data.begin();
		bool found = false;
		while (it != m_data.end() && !found)
		{
			if (it->second == path)
				found = true;
			else
				++it;
		}

		if (!found)
			RemoveFile(path);
	}
}

//-------------------------------------------------------------------------------------------------
void ImageDatabase::_ReadDataFile()
{
	std::string path = m_imagesPath + s_imageDatabaseFileName;
	Json::Value root;
	Tools::ReadJSON(path.c_str(), root);

	m_data.clear();
	auto members = root.getMemberNames();
	for (auto& member : members)
	{
		m_data[member] = root[member].asString();
	}
}

//-------------------------------------------------------------------------------------------------
void ImageDatabase::_WriteDataFile()
{
	std::string path = m_imagesPath;
	path += s_imageDatabaseFileName;

	Json::Value root;
	for (auto entry : m_data)
	{
		root[entry.first] = m_data[entry.first];
	}

	Tools::WriteJSON(path.c_str(), root);
}