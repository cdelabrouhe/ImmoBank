#include "ImageDatabase.h"
#include "Tools\Tools.h"
#include "Tools\Types.h"

using namespace ImmoBank;

const std::string s_imageDatabaseFileName = "images.dat";
ImageDatabase* s_singleton = nullptr;

//-------------------------------------------------------------------------------------------------
ImageDatabase* ImageDatabase::getSingleton()
{
	if (s_singleton == nullptr)
		s_singleton = new ImageDatabase();
	return s_singleton;
}

//-------------------------------------------------------------------------------------------------
void ImageDatabase::Init()
{
	_Read();
	_Check();
}

//-------------------------------------------------------------------------------------------------
void ImageDatabase::Process()
{
	if (!m_modified)
		return;

	m_modified = false;

	_Write();
}

//-------------------------------------------------------------------------------------------------
bool ImageDatabase::HasImage(const std::string& _URL) const
{
	auto it = m_data.find(_URL);
	return it != m_data.end();
}

//-------------------------------------------------------------------------------------------------
void ImageDatabase::StoreImage(const std::string& _URL, unsigned char* _buffer, int _bufferSize)
{
	if (HasImage(_URL))
		return;

	std::string path = GeneratePath();
	m_data[_URL] = path;

	Tools::Write(path.c_str(), _buffer, _bufferSize);

	m_modified = true;
}

//-------------------------------------------------------------------------------------------------
unsigned char* ImmoBank::ImageDatabase::GetImage(const std::string& _URL, int& _size) const
{
	auto it = m_data.find(_URL);
	if (it != m_data.end())
	{
		_size = 0;
		return nullptr;
	}

	std::string path = it->second;
	return Tools::Read(path.c_str(), _size);
}

//-------------------------------------------------------------------------------------------------
void ImmoBank::ImageDatabase::RemoveImage(const std::string& _URL)
{
	auto it = m_data.find(_URL);
	if (it == m_data.end())
		return;

	std::string path = it->second;
	m_data.erase(it);
	if (!Tools::FileExists(path.c_str()))
		return;

	Tools::DeleteFileOnDisk(path.c_str());
}

//-------------------------------------------------------------------------------------------------
void ImmoBank::ImageDatabase::_Check()
{
	std::vector<std::string> URLs;
	for (auto& pair : m_data)
	{
		if (!Tools::FileExists(pair.second.c_str()))
			URLs.push_back(pair.second);
	}

	for (auto URL : URLs)
	{
		RemoveImage(URL);
	}
}

//-------------------------------------------------------------------------------------------------
void ImmoBank::ImageDatabase::_Read()
{
	std::string path = Tools::GetExePath();
	path += s_imageDatabaseFileName;
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
void ImmoBank::ImageDatabase::_Write()
{
	std::string path = Tools::GetExePath();
	path += s_imageDatabaseFileName;
	Json::Value root;
	for (auto entry : m_data)
	{
		root[entry.first] = m_data[entry.first];
	}

	Tools::WriteJSON(path.c_str(), root);
}

//-------------------------------------------------------------------------------------------------
std::string ImmoBank::ImageDatabase::GeneratePath()
{
	std::string path = "Images/" + std::to_string(rand()) + ".jpg";
	while (Tools::FileExists(path.c_str()))
	{
		path = "Images/" + std::to_string(rand()) + ".jpg";
	}
	return path;
}
