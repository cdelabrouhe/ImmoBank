#include "Tools.h"
#include <Tools/StringTools.h>
#include "extern/jsoncpp/reader.h"
#include <Database/BoroughData.h>
#include <extern/jsoncpp/reader.h>
#include <windows.h>

using namespace ImmoBank;

bool Tools::m_devMode = false;
bool Tools::m_viewMode = false;
bool Tools::m_editMode = false;
static float s_versionNumber = 1.01f;

bool Tools::IsDevMode()
{
	return m_devMode;
}

void Tools::SetDevMode(bool _state)
{
	m_devMode = _state;
}

void Tools::InvertDevMode()
{
	m_devMode = !m_devMode;
}

bool Tools::IsViewAllowed()
{
	return m_viewMode || IsDevMode();
}

void Tools::SetViewAllowed(bool _state)
{
	m_viewMode = _state;
}

bool Tools::IsEditAllowed()
{
	return m_editMode || IsDevMode();
}

void Tools::SetEditAllowed(bool _state)
{
	m_editMode = _state;
}

bool Tools::FileExists(const char* _path)
{
	return GetFileAttributesA(_path) != INVALID_FILE_ATTRIBUTES;
}

float Tools::GetVersionNumber()
{
	return s_versionNumber;
}

float Tools::ComputeRentabilityRate(float _rent, float _price)
{
	return _rent * 12.f * 100.f / _price;
}

bool Tools::ExtractPricesFromHTMLSource(const std::string& _source, sPrice& _rent, sPrice& _buyApartment, sPrice& _buyHouse, unsigned int& _meilleursAgentsKey, int& _zipCode)
{
	std::string searchStr("MA.Context.placePrices = ");
	_zipCode = -1;
	auto findID = _source.find(searchStr);
	if (findID == std::string::npos)
	{
		// Bot behavior detected ?
		findID = _source.find("behavior");
		if (findID != std::string::npos)
		{
			printf("ERROR: can't retrieve borough information, too many requests and website banned us\n");
			/*static bool s_test = false;
			if (s_test)
			{
				FILE* f = fopen("error.html", "wt");
				if (f)
				{
					fwrite(_source.data(), sizeof(char), (size_t)_source.size(), f);
					fclose(f);
				}
			}
			return false;*/
			return true;
		}
		else
		{
			findID = _source.find("Ooops");
			if (findID != std::string::npos)
			{
				printf("ERROR: bad request, maybe a bad city name ?\n");
				return false;
			}
			else
			{
				printf("ERROR: bad request, bad copy / paste ?\n");
				return false;
			}
		}
	}

	std::string tmp = _source.substr(findID + searchStr.size(), _source.size());
	findID = tmp.find_first_of(";");
	tmp = tmp.substr(0, findID);

	StringTools::RemoveEOL(tmp);

	Json::Reader reader;
	Json::Value root;
	reader.parse(tmp, root);
	Json::Value& rental = root["rental"]["apartment"];
	_rent = sPrice((float)rental["value"].asDouble(), (float)rental["low"].asDouble(), (float)rental["high"].asDouble());
	Json::Value& sellApartment = root["sell"]["apartment"];
	Json::Value& sellHouse = root["sell"]["house"];
	_buyApartment = sPrice((float)sellApartment["value"].asDouble(), (float)sellApartment["low"].asDouble(), (float)sellApartment["high"].asDouble());
	_buyHouse = sPrice((float)sellHouse["value"].asDouble(), (float)sellHouse["low"].asDouble(), (float)sellHouse["high"].asDouble());
	_meilleursAgentsKey = root["rental"]["place"]["id"].asUInt();



	searchStr = "MA.Context.mapPlace = ";
	findID = _source.find(searchStr);
	if (findID != std::string::npos)
	{
		tmp = _source.substr(findID + searchStr.size(), _source.size());
		findID = tmp.find_first_of(";");
		tmp = tmp.substr(0, findID);

		StringTools::RemoveEOL(tmp);

		Json::Reader reader;
		Json::Value root;
		reader.parse(tmp, root);
		Json::Value& zipData = root["zip"];
		if (zipData.isString())
			_zipCode = stoi(zipData.asString());
	}

	return true;
}

std::string Tools::GetExePath()
{
	char exe_path[2048];
	GetModuleFileNameA(NULL, exe_path, 2048);
	std::string exePath = exe_path;
	auto delimiter = exePath.find_first_of("\\");
	while (delimiter != std::string::npos)
	{
		exePath.replace(delimiter, 1, "/");
		delimiter = exePath.find_first_of("\\");
	}
	delimiter = exePath.find_last_of("/");
	exePath = exePath.substr(0, delimiter + 1);
	return exePath;
}

bool Tools::ReadJSON(const char* _path, Json::Value& _data)
{
	FILE* file = fopen(_path, "rt");
	if (file)
	{
		char* test_data = (char*)malloc(1000000);
		fread(test_data, sizeof(char), 1000000, file);
		fclose(file);
		std::string str = test_data;
		free(test_data);

		Json::Reader reader;
		reader.parse(str, _data);

		return true;
	}
	return false;
}

bool Tools::WriteJSON(const char* _path, Json::Value& _data)
{
	std::string str = _data.toStyledString();
	FILE* f = fopen(_path, "wt");
	if (f)
	{
		fwrite(str.data(), sizeof(char), (size_t)str.size(), f);
		fclose(f);
		return true;
	}
	return false;
}

void Tools::DeleteFileOnDisk(const char* _path)
{
	DeleteFileA(_path);
}

unsigned char* Tools::Read(const char* _path, int& _size)
{
	FILE* file = fopen(_path, "rt");
	if (file)
	{
		fseek(file, 0, SEEK_END); // seek to end of file
		_size = ftell(file); // get current file pointer
		fseek(file, 0, SEEK_SET); // seek back to beginning of file

		unsigned char* test_data = (unsigned char*)malloc(1000000);
		fread(test_data, sizeof(char), 1000000, file);
		fclose(file);
		return test_data;
	}
	return nullptr;
}

bool Tools::Write(const char* _path, unsigned char* _buffer, int _size)
{
	FILE* f = fopen(_path, "wt");
	if (f)
	{
		fwrite(_buffer, sizeof(char), (size_t)_size, f);
		fclose(f);
		return true;
	}
	return false;
}