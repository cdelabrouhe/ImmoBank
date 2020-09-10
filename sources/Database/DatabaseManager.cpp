#include "DatabaseManager.h"
#include "SQLDatabase.h"
#include "Online/OnlineManager.h"
#include <time.h>
#include "Request/SearchRequest/SearchRequestCityBoroughs.h"
#include "Request/SearchRequest/SearchRequestResulCityBorough.h"
#include "Request/SearchRequest/SearchRequestResulCityBoroughData.h"
#include "Request/SearchRequest/SearchRequestCityBoroughData.h"
#include <algorithm>
#include "MySQLDatabase.h"
#include "extern/ImGui/imgui.h"
#include "ImageDatabase.h"
#include <Online/OnlineDatabase.h>

using namespace ImmoBank;

DatabaseManager* s_singleton = nullptr;
const std::string ImmoBank::s_wholeCityName = "WholeCity";
static int s_externalDBUpdateInterval = 600;

//-------------------------------------------------------------------------------------------------
DatabaseManager* DatabaseManager::getSingleton()
{
	if (s_singleton == nullptr)
		s_singleton = new DatabaseManager();
	return s_singleton;
}

//-------------------------------------------------------------------------------------------------
// FUNCTIONS
//-------------------------------------------------------------------------------------------------
void DatabaseManager::Init()
{
	for (auto ID = 0; ID < DataTables_COUNT; ++ID)
		m_mainTables[ID] = nullptr;

	OpenTables();
	CreateTables();

	m_externalDB = new MySQLDatabase();
	m_connectionValid = m_externalDB->Init();
	
	m_externalTimer = time(0);

	if (Tools::IsDevMode())
		s_externalDBUpdateInterval = 1000000;
	else
		s_externalDBUpdateInterval = 600;

	//Test();
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::CreateTables()
{
	SQLExecute(m_mainTables[DataTables_Boroughs],
		"CREATE TABLE IF NOT EXISTS 'Boroughs' (\n"
		"`CITY` TEXT,\n"			// Name of the city
		"`BOROUGH` TEXT,\n"			// Name of the borough
		"`TIMEUPDATE` INTEGER,\n"	// Last time the borough has been updated
		"`KEY` INTEGER,\n"			// Internal meilleursagents.com key
		"`APARTMENTBUY` REAL,\n"	// Buy min price
		"`APARTMENTBUYMIN` REAL,\n"	// Buy min price
		"`APARTMENTBUYMAX` REAL,\n"	// Buy max price
		"`HOUSEBUY` REAL,\n"		// Buy min price
		"`HOUSEBUYMIN` REAL,\n"		// Buy max price
		"`HOUSEBUYMAX` REAL,\n"		// Buy min price
		"`RENTHOUSE` REAL,\n"		// Buy max price
		"`RENTHOUSEMIN` REAL,\n"	// Buy min price
		"`RENTHOUSEMAX` REAL,\n"	// Buy max price
		"`RENTT1` REAL,\n"			// Buy min price
		"`RENTT1MIN` REAL,\n"		// Buy max price
		"`RENTT1MAX` REAL,\n"		// Buy min price
		"`RENTT2` REAL,\n"			// Buy max price
		"`RENTT2MIN` REAL,\n"		// Buy min price

		"`RENTT2MAX` REAL,\n"		// Buy max price
		"`RENTT3` REAL,\n"			// Buy min price
		"`RENTT3MIN` REAL,\n"		// Buy max price
		"`RENTT3MAX` REAL,\n"		// Buy min price
		"`RENTT4` REAL,\n"			// Buy max price
		"`RENTT4MIN` REAL,\n"		// Buy min price
		"`RENTT4MAX` REAL,\n"		// Buy max price
		"`SELOGERKEY` INTEGER,\n"	// Internal SeLoger key
		"`ZIPCODE` INTEGER,\n"		// ZIP code
		"`INSEECODE` INTEGER"		// INSEE code
		")"
	);
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::OpenTables()
{
	// Find EXE path
	std::string exePath = Tools::GetExePath();

	// Load SQL databases
	if (!m_mainTables[DataTables_Boroughs])
	{
		std::string path = exePath + "boroughs.sqlite";
		m_mainTables[DataTables_Boroughs] = SQLOpenDB(path.c_str());
	}
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::CloseTables()
{
	for (int entry = 0; entry < DataTables_COUNT; ++entry)
	{
		SQLCloseDB(m_mainTables[entry]);
		m_mainTables[entry] = nullptr;
	}

	auto it = m_tables.begin();
	while (it != m_tables.end())
	{
		SQLCloseDB(it->second);
		++it;
	}
	m_tables.clear();
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::NotifyOnlineDatabaseCreation(OnlineDatabase* _db)
{
	if (_db->GetDatabaseName().empty())
		return;

	// Find EXE path
	std::string exePath = Tools::GetExePath();

	// Open secondary tables
	std::string path = exePath + _db->GetDatabaseName() + ".sqlite";
	m_tables[_db->GetDatabaseName()] = SQLOpenDB(path.c_str());

	// Create secondary tables
	std::string request;
	_db->CreateTableRequest(request);
	SQLExecute(m_tables[_db->GetDatabaseName()], request.c_str());

	_db->Load();
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::Process()
{
	m_modified = false;

	if ((time(0) - m_externalTimer) > s_externalDBUpdateInterval)
	{
		m_modified = true;
		m_externalTimer = time(0);
		printf("DatabaseManager: force external update");
	}

	// City updates (get borough list from external DB)
	auto itCityUpdate = m_cityUpdates.begin();
	while (itCityUpdate != m_cityUpdates.end())
	{
		CityUpdateData& data = *itCityUpdate;
		if (data.Process())
			itCityUpdate = m_cityUpdates.erase(itCityUpdate);
		else
			++itCityUpdate;
	}

	// Compute city borough list from MeilleursAgents
	auto itCity = m_cityComputes.begin();
	while (itCity != m_cityComputes.end())
	{
		CityComputeData& data = *itCity;
		if (data.Process())
			itCity = m_cityComputes.erase(itCity);
		else
			++itCity;
	}

	// Boroughs computes
	auto itBorough = m_boroughComputes.begin();
	while (itBorough != m_boroughComputes.end())
	{
		BoroughData& data = *itBorough;
		if (data.Process())
		{
			data.End();
			itBorough = m_boroughComputes.erase(itBorough);
		}
		else
			++itBorough;
	}

	// External process
	m_externalDB->Process();

	auto it = m_externalBoroughRequests.begin();
	while (it != m_externalBoroughRequests.end())
	{
		int queryID = it->second;
		if (m_externalDB->IsQueryAvailable(queryID))
		{
			BoroughData borough;
			m_externalDB->GetResultBoroughData(queryID, borough);
			AddBoroughData(borough, false);
			it = m_externalBoroughRequests.erase(it);
		}
		else
			++it;
	}

	if (Tools::IsDevMode())
	{
		DisplayDebug();

		if (m_generateZipCodesIndices)
			m_generateZipCodesIndices = m_externalDB->UpdateAllZipCodes();

		if (m_updateLocalBaseToServer)
			m_updateLocalBaseToServer = m_externalDB->UpdateLocalBaseToServer();

		if (m_updateServerToLocalBase)
			m_updateServerToLocalBase = m_externalDB->UpdateServerToLocalBase();
	}
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::End()
{
	CloseTables();

	m_externalDB->End();
	delete m_externalDB;
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::AddBoroughData(const BoroughData& _data, bool _saveExternal)
{
	m_modified = true;

	BoroughData localData = _data;
	localData.m_city.UnFixName();
	RemoveBoroughData(localData.m_city.m_name, localData.m_name, localData.m_city.m_zipCode);

	char buf[4096];
	sprintf(buf, "INSERT OR REPLACE INTO Boroughs (CITY, BOROUGH, TIMEUPDATE, KEY, APARTMENTBUY, APARTMENTBUYMIN, APARTMENTBUYMAX, HOUSEBUY, HOUSEBUYMIN, HOUSEBUYMAX, RENTHOUSE, RENTHOUSEMIN, RENTHOUSEMAX, RENTT1, RENTT1MIN, RENTT1MAX, RENTT2, RENTT2MIN, RENTT2MAX, RENTT3, RENTT3MIN, RENTT3MAX, RENTT4, RENTT4MIN, RENTT4MAX, SELOGERKEY, ZIPCODE, INSEECODE) VALUES('%s', '%s', %u, %u, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %u, %u, %u)",
		localData.m_city.m_name.c_str(),
		localData.m_name.c_str(),
		localData.m_timeUpdate.GetData(),
		localData.m_meilleursAgentsKey,
		localData.m_priceBuyApartment.m_val,
		localData.m_priceBuyApartment.m_min,
		localData.m_priceBuyApartment.m_max,
		localData.m_priceBuyHouse.m_val,
		localData.m_priceBuyHouse.m_min,
		localData.m_priceBuyHouse.m_max,
		localData.m_priceRentHouse.m_val,
		localData.m_priceRentHouse.m_min,
		localData.m_priceRentHouse.m_max,
		localData.m_priceRentApartmentT1.m_val,
		localData.m_priceRentApartmentT1.m_min,
		localData.m_priceRentApartmentT1.m_max,
		localData.m_priceRentApartmentT2.m_val,
		localData.m_priceRentApartmentT2.m_min,
		localData.m_priceRentApartmentT2.m_max,
		localData.m_priceRentApartmentT3.m_val,
		localData.m_priceRentApartmentT3.m_min,
		localData.m_priceRentApartmentT3.m_max,
		localData.m_priceRentApartmentT4Plus.m_val,
		localData.m_priceRentApartmentT4Plus.m_min,
		localData.m_priceRentApartmentT4Plus.m_max,
		localData.m_selogerKey,
		localData.m_city.m_zipCode,
		localData.m_city.m_inseeCode);

	/*static bool s_test = false;
	if (s_test)
	{
		FILE* f = fopen("error.txt", "wt");
		if (f)
		{
			std::string str = buf;
			fwrite(str.data(), sizeof(char), (size_t)str.size(), f);
			fclose(f);
		}
	}*/

	if (SQLExecute(m_mainTables[DataTables_Boroughs], buf))
		printf("Add borough %s to database Boroughs\n", localData.m_name.c_str());

	if (_saveExternal)
		m_externalDB->WriteBoroughData(localData);
}

//-------------------------------------------------------------------------------------------------
void ExtractBoroughDataFromSQL(BoroughData& _borough, sqlite3_stmt* _stmt)
{
	int index = 0;
	_borough.m_city.m_name = (const char*)sqlite3_column_text(_stmt, index++);
	_borough.m_name = (const char*)sqlite3_column_text(_stmt, index++);
	_borough.m_timeUpdate.SetData((unsigned int)sqlite3_column_int64(_stmt, index++));
	_borough.m_meilleursAgentsKey = (unsigned int)sqlite3_column_int64(_stmt, index++);
	_borough.m_priceBuyApartment.m_val = (float)sqlite3_column_double(_stmt, index++);
	_borough.m_priceBuyApartment.m_min = (float)sqlite3_column_double(_stmt, index++);
	_borough.m_priceBuyApartment.m_max = (float)sqlite3_column_double(_stmt, index++);
	_borough.m_priceBuyHouse.m_val = (float)sqlite3_column_double(_stmt, index++);
	_borough.m_priceBuyHouse.m_min = (float)sqlite3_column_double(_stmt, index++);
	_borough.m_priceBuyHouse.m_max = (float)sqlite3_column_double(_stmt, index++);
	_borough.m_priceRentHouse.m_val = (float)sqlite3_column_double(_stmt, index++);
	_borough.m_priceRentHouse.m_min = (float)sqlite3_column_double(_stmt, index++);
	_borough.m_priceRentHouse.m_max = (float)sqlite3_column_double(_stmt, index++);
	_borough.m_priceRentApartmentT1.m_val = (float)sqlite3_column_double(_stmt, index++);
	_borough.m_priceRentApartmentT1.m_min = (float)sqlite3_column_double(_stmt, index++);
	_borough.m_priceRentApartmentT1.m_max = (float)sqlite3_column_double(_stmt, index++);
	_borough.m_priceRentApartmentT2.m_val = (float)sqlite3_column_double(_stmt, index++);
	_borough.m_priceRentApartmentT2.m_min = (float)sqlite3_column_double(_stmt, index++);
	_borough.m_priceRentApartmentT2.m_max = (float)sqlite3_column_double(_stmt, index++);
	_borough.m_priceRentApartmentT3.m_val = (float)sqlite3_column_double(_stmt, index++);
	_borough.m_priceRentApartmentT3.m_min = (float)sqlite3_column_double(_stmt, index++);
	_borough.m_priceRentApartmentT3.m_max = (float)sqlite3_column_double(_stmt, index++);
	_borough.m_priceRentApartmentT4Plus.m_val = (float)sqlite3_column_double(_stmt, index++);
	_borough.m_priceRentApartmentT4Plus.m_min = (float)sqlite3_column_double(_stmt, index++);
	_borough.m_priceRentApartmentT4Plus.m_max = (float)sqlite3_column_double(_stmt, index++);
	_borough.m_selogerKey = (unsigned int)sqlite3_column_int64(_stmt, index++);
	_borough.m_city.m_zipCode = (int)sqlite3_column_int64(_stmt, index++);
	_borough.m_city.m_inseeCode = (int)sqlite3_column_int64(_stmt, index++);
}

//-------------------------------------------------------------------------------------------------
bool DatabaseManager::GetBoroughData(const std::string& _cityName, const int _zipCode, const std::string& _name, BoroughData& _data)
{
	if (m_mainTables[DataTables_Boroughs] == nullptr)
		return false;

	std::vector<BoroughData> boroughs;
	Str128f sql("SELECT * FROM Boroughs WHERE CITY='%s' AND BOROUGH='%s' AND ZIPCODE=%d", _cityName.c_str(), _name.c_str(), _zipCode);

	SQLExecuteSelect(m_mainTables[DataTables_Boroughs], sql.c_str(), [&boroughs](sqlite3_stmt* _stmt)
	{
		BoroughData borough;
		ExtractBoroughDataFromSQL(borough, _stmt);
		boroughs.push_back(borough);
	});

	if (boroughs.size() == 1)
	{
		_data = boroughs.back();
		_data.m_city.FixName();
		return true;
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
bool DatabaseManager::RemoveBoroughData(const std::string& _cityName, const std::string& _name, const int _zipCode)
{
	if (m_mainTables[DataTables_Boroughs] == nullptr)
		return false;

	m_modified = true;

	bool result = false;
	std::vector<sCityData> cities;
	Str128f sql("DELETE FROM Boroughs WHERE CITY='%s' AND BOROUGH='%s' AND ZIPCODE=%d", _cityName.c_str(), _name.c_str(), _zipCode);
	result = SQLExecute(m_mainTables[DataTables_Boroughs], sql.c_str());

	return result;
}

//-------------------------------------------------------------------------------------------------
bool DatabaseManager::AddQuery(const std::string& _query, std::vector<BoroughData>& _data)
{
	if (m_mainTables[DataTables_Boroughs] == nullptr)
		return false;

	_data.clear();

	SQLExecuteSelect(m_mainTables[DataTables_Boroughs], _query.c_str(), [&_data](sqlite3_stmt* _stmt)
	{
		BoroughData borough;
		ExtractBoroughDataFromSQL(borough, _stmt);
		_data.push_back(borough);
	});
	
	if (_data.size() >= 1)
		return true;

	return false;
}

//-------------------------------------------------------------------------------------------------
bool DatabaseManager::GetBoroughs(sCity& _city, std::vector<BoroughData>& _data, bool _checkZipCode)
{
	if (m_mainTables[DataTables_Boroughs] == nullptr)
		return false;

	_data.clear();
	
	char buf[4096];
	do 
	{
		if (!_city.m_name.empty())
		{
			if (_checkZipCode)
			{
				_checkZipCode = false;
				sprintf(buf, "SELECT * FROM Boroughs WHERE CITY='%s' AND ZIPCODE=%d", _city.m_name.c_str(), _city.m_zipCode);
			}
			else
			{
				_checkZipCode = true;
				sprintf(buf, "SELECT * FROM Boroughs WHERE CITY='%s'", _city.m_name.c_str());
			}
		}
		else
			sprintf(buf, "SELECT * FROM Boroughs");

		std::string sql = buf;

		AddQuery(sql, _data);

	} while (!_checkZipCode && (_data.size() == 0));

	for (auto& borough : _data)
	{
		int zipCode = borough.m_city.m_zipCode;
		int inseeCode = borough.m_city.m_inseeCode;
		borough.m_city = !_city.m_name.empty() ? _city : borough.m_city;
		borough.m_city.FixName();
		if (zipCode != 0)
			borough.m_city.m_zipCode = zipCode;
		if (inseeCode != 0)
			borough.m_city.m_inseeCode = inseeCode;

		// Ask data to external database
		if (!borough.IsValid())
		{
			unsigned int key = borough.m_meilleursAgentsKey;
			auto it = std::find_if(m_externalBoroughRequests.begin(), m_externalBoroughRequests.end(), [key](std::pair<unsigned int, int>& _pair)->bool
			{
				return (_pair.first == key);
			});
			if (it == m_externalBoroughRequests.end())
			{
				int queryID = m_externalDB->AskForBoroughData(borough);
				if (queryID != -1)
					m_externalBoroughRequests.push_back(std::make_pair(key, queryID));
			}
		}
	}

	if (_data.size() >= 1)
		return true;

	return false;
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::GetAllBoroughs(std::vector<BoroughData>& _data)
{
	sCity city;
	GetBoroughs(city, _data);
}

//-------------------------------------------------------------------------------------------------
sqlite3* DatabaseManager::GetTable(const std::string& _name)
{
	auto it = m_tables.find(_name);
	if (it != m_tables.end())
		return it->second;
	return nullptr;
}

//-------------------------------------------------------------------------------------------------
bool DatabaseManager::IsCityUpdating(const std::string& _cityName)
{
	bool result = true;
	auto itCompute = std::find_if(m_cityComputes.begin(), m_cityComputes.end(), [_cityName](CityComputeData& _cityData)->bool
	{
		return (_cityData.m_city.m_name == _cityName) && (_cityData.m_city.m_name == _cityName);
	});
	result &= itCompute != m_cityComputes.end();

	if (result)
	{
		auto itUpdate = std::find_if(m_cityUpdates.begin(), m_cityUpdates.end(), [_cityName](CityUpdateData& _cityData)->bool
		{
			return (_cityData.m_city.m_name == _cityName) && (_cityData.m_city.m_name == _cityName);
		});
		result &= itUpdate != m_cityUpdates.end();
	}

	return result;
}

//-------------------------------------------------------------------------------------------------
bool DatabaseManager::IsBoroughUpdating(const BoroughData& _data)
{
	auto it = std::find_if(m_boroughComputes.begin(), m_boroughComputes.end(), [_data](BoroughData& _boroughData)->bool
	{
		return (_boroughData.m_city.m_name == _data.m_city.m_name) && (_boroughData.m_name == _data.m_name);
	});
	return it != m_boroughComputes.end();
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::AddCity(sCityData& _data)
{
	m_modified = true;

	BoroughData data;
	if (GetBoroughData(_data.m_data.m_name, _data.m_data.m_zipCode, s_wholeCityName, data))
		return;

	data.m_name = s_wholeCityName;
	data.m_city = _data.m_data;
	AddBoroughData(data);
}

//-------------------------------------------------------------------------------------------------
bool DatabaseManager::GetCityData(const std::string& _name, const int _zipCode, sCityData& _data, BoroughData* _wholeCity)
{
	if (m_mainTables[DataTables_Boroughs] == nullptr)
		return false;

	std::vector<sCityData> cities;
	BoroughData data;
	if (!GetBoroughData(_name, _zipCode, s_wholeCityName, data))
		return false;

	_data.m_data = data.m_city;
	_data.m_timeUpdate = data.m_timeUpdate;
	GetBoroughs(_data.m_data, _data.m_boroughs, false);
	_data.m_data.FixName();

	// Look for a WholeCity borough
	std::string lookFor = s_wholeCityName;
	auto itCity = std::find_if(_data.m_boroughs.begin(), _data.m_boroughs.end(), [lookFor, _zipCode](BoroughData& _borough)->bool
	{	return (_borough.m_name == lookFor) && (_borough.m_city.m_zipCode == _zipCode);	});
	if (itCity == _data.m_boroughs.end())
	{
		BoroughData data;
		data.m_city = _data.m_data;
		data.m_name = s_wholeCityName;
		_data.m_boroughs.push_back(data);
	}

	std::sort(_data.m_boroughs.begin(), _data.m_boroughs.end(), BoroughData::compare);

	bool found = false;
	auto it = _data.m_boroughs.begin();
	while (it != _data.m_boroughs.end())
	{
		BoroughData& data = *it;
		const bool isWholeCity = (_wholeCity && data.IsWholeCity());
		if (isWholeCity)
		{
			if (!found)
			{
				found = true;
				*_wholeCity = data;
			}
			else if (data.m_city.m_zipCode == _zipCode)
				*_wholeCity = data;
		}

		// Ask data to external database
		if (!data.IsValid())
		{
			unsigned int key = data.m_meilleursAgentsKey;
			auto itExternal = std::find_if(m_externalBoroughRequests.begin(), m_externalBoroughRequests.end(), [key](std::pair<unsigned int, int>& _pair)->bool
			{
				return (_pair.first == key);
			});
			if (itExternal == m_externalBoroughRequests.end())
			{
				int queryID = m_externalDB->AskForBoroughData(data);
				if (queryID != -1)
					m_externalBoroughRequests.push_back(std::make_pair(key, queryID));
			}
		}

		// next
		if (_wholeCity && data.IsWholeCity())
			it = _data.m_boroughs.erase(it);
		else
			++it;
	}

	return true;
}

//-------------------------------------------------------------------------------------------------
bool DatabaseManager::RemoveCityData(const std::string& _name, const int _zipCode)
{
	return RemoveBoroughData(_name, s_wholeCityName, _zipCode);
}

//-------------------------------------------------------------------------------------------------
void FillCity(sqlite3_stmt* _stmt, std::vector<sCityData>& _cities)
{
	BoroughData borough;
	ExtractBoroughDataFromSQL(borough, _stmt);
	sCityData data;
	data.m_data = borough.m_city;
	data.m_timeUpdate = borough.m_timeUpdate;
	_cities.push_back(data);
}

//-------------------------------------------------------------------------------------------------
bool DatabaseManager::ListAllCities(std::vector<sCity>& _list)
{
	if (m_mainTables[DataTables_Boroughs] == nullptr)
		return false;

	std::vector<sCityData> cities;
	Str128f sql("SELECT * FROM Boroughs WHERE BOROUGH='%s'", s_wholeCityName);

	SQLExecuteSelect(m_mainTables[DataTables_Boroughs], sql.c_str(), [&cities](sqlite3_stmt* _stmt)
	{
		FillCity(_stmt, cities);
	});

	if (cities.size() > 0)
	{
		for (auto& city : cities)
		{
			city.m_data.FixName();
			_list.push_back(city.m_data);
		}

		return true;
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::ListAllCitiesWithName(std::vector<sCity>& _list, std::string _name)
{
	std::vector<sCity> list;
	ListAllCities(list);

	std::sort(list.begin(), list.end(), sCity::compare);
	std::transform(_name.begin(), _name.end(), _name.begin(), ::tolower);

	auto it = list.begin();
	while (it != list.end())
	{
		std::string str = (*it).m_name;
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);
		auto itFind = str.find(_name);
		if (itFind != std::string::npos)
			_list.push_back(*it);
		++it;
	}
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::Test()
{
	sDate date;
	date.SetDate(2020, 10, 30, 15, 58, 17);

	sCityData city;
	city.m_data.m_name = "Montpellier";
	city.m_data.m_zipCode = 34000;
	city.m_data.m_inseeCode = 340172;
	time_t t = time(0);   // get time now
	struct tm * now = localtime(&t);
	int year = 1900 + now->tm_year;
	city.m_timeUpdate.SetDate(year, now->tm_mon, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
	AddCity(city);

	BoroughData mainCityData;
	sCityData city2;
	GetCityData("Montpellier", 34000, city2);

	BoroughData data;
	data.m_name = "Antigone";
	data.m_city.m_name = "Montpellier";
	data.m_meilleursAgentsKey = 13245;
	data.m_timeUpdate = date;

	data.m_priceBuyApartment.m_val = 1;
	data.m_priceBuyApartment.m_min = 2;
	data.m_priceBuyApartment.m_max = 3;
	data.m_priceBuyHouse.m_val = 4;
	data.m_priceBuyHouse.m_min = 5;
	data.m_priceBuyHouse.m_max = 6;
	data.m_priceRentHouse.m_val = 7;
	data.m_priceRentHouse.m_min = 8;
	data.m_priceRentHouse.m_max = 9;
	data.m_priceRentApartmentT1.m_val = 10;
	data.m_priceRentApartmentT1.m_min = 11;
	data.m_priceRentApartmentT1.m_max = 12;
	data.m_priceRentApartmentT2.m_val = 13;
	data.m_priceRentApartmentT2.m_min = 14;
	data.m_priceRentApartmentT2.m_max = 15;
	data.m_priceRentApartmentT3.m_val = 16;
	data.m_priceRentApartmentT3.m_min = 17;
	data.m_priceRentApartmentT3.m_max = 18;
	data.m_priceRentApartmentT4Plus.m_val = 19;
	data.m_priceRentApartmentT4Plus.m_min = 20;
	data.m_priceRentApartmentT4Plus.m_max = 21;
	data.m_selogerKey = 0;
	data.m_city.m_zipCode = 0;

	AddBoroughData(data);

	BoroughData data2;
	GetBoroughData("Montpellier", 34000, "Antigone", data2);
	printf("");
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::UpdateCityData(const sCity& _city)
{
	CityUpdateData data;
	data.m_city = _city;
	m_cityUpdates.push_back(data);
	m_cityUpdates.back().Init();
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::ComputeCityData(const sCity& _city)
{
	CityComputeData data;
	data.m_city = _city;
	m_cityComputes.push_back(data);
	m_cityComputes.back().Init();
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::ComputeBoroughData(BoroughData& _data)
{
	m_boroughComputes.push_back(_data);
	m_boroughComputes.back().Init();
}

//-------------------------------------------------------------------------------------------------
int DatabaseManager::AskForExternalDBCityBoroughs(const sCity& _city)
{
	return m_externalDB->AskForCityBoroughList(_city);
}

//-------------------------------------------------------------------------------------------------
bool DatabaseManager::IsExternalDBCityBoroughsAvailable(int _requestID, std::vector<BoroughData>& _boroughs)
{
	if (m_externalDB->IsQueryAvailable(_requestID))
	{
		m_externalDB->GetResultBoroughList(_requestID, _boroughs);
		return true;
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::ForceBoroughReset(BoroughData& _data)
{
	RemoveBoroughData(_data.m_city.m_name, _data.m_name, _data.m_city.m_zipCode);
	_data.Reset();
	AddBoroughData(_data, false);
	m_externalDB->RemoveBoroughData(_data);

	m_modified = true;
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::GetConnectionParameters(std::string& _server, std::string& _user)
{
	_server = m_externalDB->GetServer();
	_user = m_externalDB->GetUser();
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::TriggerDebugExternalSQLCommand(const std::string& _query)
{
	m_externalDB->DebugQuery(_query);
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::DisplayDebug()
{
	DisplaySQlite3Debug();
	DisplayMySQLRequestsPanel();
}

//-------------------------------------------------------------------------------------------------
bool DatabaseManager::TriggerSQLCommand(const std::string& _tableName, const std::string& _query, bool _affectExternal)
{
	bool result = SQLExecute(m_tables[_tableName], _query.c_str());
	if (_affectExternal)
	{
		m_externalDB->ExecuteQuery(_query);
	}
	return result;
}

//-------------------------------------------------------------------------------------------------
MYSQL_RES* DatabaseManager::TriggerExternalSQLCommand(const std::string& _query)
{
	return m_externalDB->ExecuteQuery(_query);
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::DisplaySQlite3Debug()
{
	if (!m_displayDebugSQLite3)
		return;

	ImGui::Begin("SQlite3 Debug panel", &m_displayDebugSQLite3);
	ImGui::BeginChild("Tools", ImVec2(ImGui::GetWindowContentRegionWidth(), 60), false, ImGuiWindowFlags_NoScrollbar);

	// Debug request
	bool callCommand = ImGui::Button("Call command") && strlen(m_SQlite3InputDebug) > 0;
	ImGui::SameLine();
	ImGui::InputText("Manual SQL command", (char*)m_SQlite3InputDebug, 2048);
	static std::vector<BoroughData> s_debugResults;
	if (callCommand)
	{
		std::string str = m_SQlite3InputDebug;
		DatabaseManager::getSingleton()->AddQuery(str, s_debugResults);

		m_SQlite3Requests.clear();
		for (auto& result : s_debugResults)
		{
			std::string mes = "City: " + result.m_city.m_name + ", Borough: " + result.m_name + ", MeilleursAgentsKEY: " + std::to_string(result.m_meilleursAgentsKey) + ", ZipCode: " + std::to_string(result.m_city.m_zipCode);
			m_SQlite3Requests.push_back(mes);
		}
	}

	if (ImGui::Button("Clear"))
		m_SQlite3Requests.clear();

	ImGui::Separator();

	ImGui::EndChild();

	ImGui::BeginChild("Requests");
	auto nbRequests = m_SQlite3Requests.size();
	for (auto ID = 0; ID < nbRequests; ++ID)
	{
		ImGui::TextWrapped("%s", m_SQlite3Requests[ID].c_str());
	}
	ImGui::EndChild();

	ImGui::End();
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::DisplayMySQLRequestsPanel()
{
	if (!m_displayDebugMySQL)
		return;

	ImGui::Begin("MySQL Debug panel", &m_displayDebugMySQL);
	ImGui::BeginChild("Tools", ImVec2(ImGui::GetWindowContentRegionWidth(), 60), false, ImGuiWindowFlags_NoScrollbar);

	// Debug request
	bool callCommand = ImGui::Button("Call command") && strlen(m_MySQLInputDebug) > 0;
	ImGui::SameLine();
	ImGui::InputText("Manual SQL command", (char*)m_MySQLInputDebug, 2048);
	if (callCommand)
	{
		std::string str = m_MySQLInputDebug;
		DatabaseManager::getSingleton()->TriggerDebugExternalSQLCommand(str);
	}

	if (ImGui::Button("Clear"))
		m_MySQLRequests.clear();

	ImGui::Separator();

	ImGui::EndChild();

	ImGui::BeginChild("Requests");
	auto nbRequests = m_MySQLRequests.size();
	for (auto ID = 0; ID < nbRequests; ++ID)
	{
		ImGui::TextWrapped("%s", m_MySQLRequests[ID].c_str());
	}
	ImGui::EndChild();

	ImGui::End();
}

void DatabaseManager::NotifyMySQLEvent(const std::string& _request)
{
	m_MySQLRequests.push_back(_request);
}