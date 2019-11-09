#include "ImageDatabase.h"
#include "Tools\Tools.h"

using namespace ImmoBank;

const std::string s_imageDatabaseFileName = "images.dat";

void ImageDatabase::Init()
{
	_Read();
}

void ImageDatabase::Process()
{
	if (!m_modified)
		return;

	m_modified = false;

	_Write();
}

bool ImageDatabase::HasImage(const std::string& _URL) const
{
	auto it = m_data.find(_URL);
	return it != m_data.end();
}

void ImageDatabase::StoreImage(const std::string& _URL, const std::string& _path)
{
	if (HasImage(_URL))
		return;

	m_data[_URL] = _path;

	m_modified = true;
}

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

void ImmoBank::ImageDatabase::_Write()
{
	std::string path = Tools::GetExePath();
	path += s_imageDatabaseFileName;
	Json::Value root;
	for (auto entry : m_data)
	{
		root[entry.first] = m_data[entry.first];
	}
}
