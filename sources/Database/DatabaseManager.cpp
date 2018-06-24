#include "DatabaseManager.h"
#include "SQLDatabase.h"
#include "Online/OnlineManager.h"
#include <time.h>
#include "Request/SearchRequest/SearchRequestCityBoroughs.h"
#include "Request/SearchRequest/SearchRequestResulCityBorough.h"
#include "Request/SearchRequest/SearchRequestResulCityBoroughData.h"
#include "Request/SearchRequest/SearchRequestCityBoroughData.h"
#include <algorithm>

DatabaseManager* s_singleton = nullptr;
const std::string s_wholeCityName = "WholeCity";

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
		m_tables[ID] = nullptr;

	OpenTables();
	CreateTables();

	//Test();
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::Process()
{
	m_modified = false;
	auto itCity = m_cityComputes.begin();
	while (itCity != m_cityComputes.end())
	{
		CityComputeData& data = *itCity;
		if (data.Process())
			itCity = m_cityComputes.erase(itCity);
		else
			++itCity;
	}

	auto itBorough = m_boroughComputes.begin();
	while (itBorough != m_boroughComputes.end())
	{
		BoroughData& data = *itBorough;
		if (data.Process())
			itBorough = m_boroughComputes.erase(itBorough);
		else
			++itBorough;
	}
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::End()
{
	CloseTables();
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::AddBoroughData(const BoroughData& _data)
{
	m_modified = true;

	RemoveBoroughData(_data.m_city.m_name, _data.m_name);

	if (SQLExecute(m_tables[DataTables_Boroughs], "INSERT OR REPLACE INTO Boroughs (CITY, BOROUGH, TIMEUPDATE, KEY, APARTMENTBUY, APARTMENTBUYMIN, APARTMENTBUYMAX, HOUSEBUY, HOUSEBUYMIN, HOUSEBUYMAX, RENTHOUSE, RENTHOUSEMIN, RENTHOUSEMAX, RENTT1, RENTT1MIN, RENTT1MAX, RENTT2, RENTT2MIN, RENTT2MAX, RENTT3, RENTT3MIN, RENTT3MAX, RENTT4, RENTT4MIN, RENTT4MAX) VALUES('%s', '%s', %u, %u, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f)",
		_data.m_city.m_name.c_str(),
		_data.m_name.c_str(),
		_data.m_timeUpdate.GetData(),
		_data.m_key,
		_data.m_priceBuyApartment.m_val,
		_data.m_priceBuyApartment.m_min,
		_data.m_priceBuyApartment.m_max,
		_data.m_priceBuyHouse.m_val,
		_data.m_priceBuyHouse.m_min,
		_data.m_priceBuyHouse.m_max,
		_data.m_priceRentHouse.m_val,
		_data.m_priceRentHouse.m_min,
		_data.m_priceRentHouse.m_max,
		_data.m_priceRentApartmentT1.m_val,
		_data.m_priceRentApartmentT1.m_min,
		_data.m_priceRentApartmentT1.m_max,
		_data.m_priceRentApartmentT2.m_val,
		_data.m_priceRentApartmentT2.m_min,
		_data.m_priceRentApartmentT2.m_max,
		_data.m_priceRentApartmentT3.m_val,
		_data.m_priceRentApartmentT3.m_min,
		_data.m_priceRentApartmentT3.m_max,
		_data.m_priceRentApartmentT4Plus.m_val,
		_data.m_priceRentApartmentT4Plus.m_min,
		_data.m_priceRentApartmentT4Plus.m_max))
		printf("Add borough %s to database Boroughs\n", _data.m_name.c_str());
}

//-------------------------------------------------------------------------------------------------
bool DatabaseManager::GetBoroughData(const std::string& _cityName, const std::string& _name, BoroughData& _data)
{
	if (m_tables[DataTables_Boroughs] == nullptr)
		return false;

	std::vector<BoroughData> boroughs;
	Str128f sql("SELECT * FROM Boroughs WHERE CITY='%s' AND BOROUGH='%s'", _cityName.c_str(), _name.c_str());

	SQLExecuteSelect(m_tables[DataTables_Boroughs], sql.c_str(), [&boroughs](sqlite3_stmt* _stmt)
	{
		int index = 0;
		boroughs.resize(boroughs.size() + 1);
		auto& borough = boroughs.back();
		borough.m_city.m_name = (const char*)sqlite3_column_text(_stmt, index++);
		borough.m_name = (const char*)sqlite3_column_text(_stmt, index++);
		borough.m_timeUpdate.SetData(sqlite3_column_int(_stmt, index++));
		borough.m_key = sqlite3_column_int(_stmt, index++);
		borough.m_priceBuyApartment.m_val = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceBuyApartment.m_min = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceBuyApartment.m_max = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceBuyHouse.m_val = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceBuyHouse.m_min = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceBuyHouse.m_max = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentHouse.m_val = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentHouse.m_min = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentHouse.m_max = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentApartmentT1.m_val = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentApartmentT1.m_min = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentApartmentT1.m_max = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentApartmentT2.m_val = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentApartmentT2.m_min = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentApartmentT2.m_max = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentApartmentT3.m_val = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentApartmentT3.m_min = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentApartmentT3.m_max = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentApartmentT4Plus.m_val = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentApartmentT4Plus.m_min = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentApartmentT4Plus.m_max = (float)sqlite3_column_double(_stmt, index++);
	});

	if (boroughs.size() == 1)
	{
		_data = boroughs.back();
		return true;
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
bool DatabaseManager::RemoveBoroughData(const std::string& _cityName, const std::string& _name)
{
	if (m_tables[DataTables_Boroughs] == nullptr)
		return false;

	m_modified = true;

	std::vector<sCityData> cities;
	Str128f sql("DELETE FROM Boroughs WHERE CITY='%s' AND BOROUGH='%s'", _cityName.c_str(), _name.c_str());

	return SQLExecute(m_tables[DataTables_Boroughs], sql.c_str());
}

//-------------------------------------------------------------------------------------------------
bool DatabaseManager::GetBoroughs(sCity& _city, std::vector<BoroughData>& _data)
{
	if (m_tables[DataTables_Boroughs] == nullptr)
		return false;

	_data.clear();
	Str128f sql("SELECT * FROM Boroughs WHERE CITY='%s'", _city.m_name.c_str());

	SQLExecuteSelect(m_tables[DataTables_Boroughs], sql.c_str(), [&_data](sqlite3_stmt* _stmt)
	{
		int index = 0;
		_data.resize(_data.size() + 1);
		auto& borough = _data.back();
		borough.m_city.m_name = (const char*)sqlite3_column_text(_stmt, index++);
		borough.m_name = (const char*)sqlite3_column_text(_stmt, index++);
		borough.m_timeUpdate.SetData(sqlite3_column_int(_stmt, index++));
		borough.m_key = sqlite3_column_int(_stmt, index++);
		borough.m_priceBuyApartment.m_val = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceBuyApartment.m_min = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceBuyApartment.m_max = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceBuyHouse.m_val = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceBuyHouse.m_min = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceBuyHouse.m_max = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentHouse.m_val = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentHouse.m_min = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentHouse.m_max = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentApartmentT1.m_val = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentApartmentT1.m_min = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentApartmentT1.m_max = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentApartmentT2.m_val = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentApartmentT2.m_min = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentApartmentT2.m_max = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentApartmentT3.m_val = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentApartmentT3.m_min = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentApartmentT3.m_max = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentApartmentT4Plus.m_val = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentApartmentT4Plus.m_min = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentApartmentT4Plus.m_max = (float)sqlite3_column_double(_stmt, index++);
	});

	for (auto& borough : _data)
		borough.m_city = _city;

	if (_data.size() >= 1)
		return true;

	return false;
}

bool DatabaseManager::IsCityUpdating(const std::string& _cityName)
{
	auto it = std::find_if(m_cityComputes.begin(), m_cityComputes.end(), [_cityName](CityComputeData& _cityData)->bool
	{
		return (_cityData.m_city == _cityName) && (_cityData.m_city == _cityName);
	});
	return it != m_cityComputes.end();
}

//-------------------------------------------------------------------------------------------------
bool DatabaseManager::IsBoroughUpdating(BoroughData& _data)
{
	auto it = std::find_if(m_boroughComputes.begin(), m_boroughComputes.end(), [_data](BoroughData& _boroughData)->bool
	{
		return (_boroughData.m_city.m_name == _data.m_city.m_name) && (_boroughData.m_name == _data.m_name);
	});
	return it != m_boroughComputes.end();
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::AddCity(const sCityData& _data)
{
	m_modified = true;

	RemoveCityData(_data.m_data.m_name);

	if (SQLExecute(m_tables[DataTables_Cities], "INSERT OR REPLACE INTO Cities (NAME, ZIPCODE, INSEECODE, TIMEUPDATE) VALUES('%s', %d, %d, %u)",
		_data.m_data.m_name.c_str(),
		_data.m_data.m_zipCode,
		_data.m_data.m_inseeCode,
		_data.m_timeUpdate.GetData()))
		printf("Add city %s to database Cities\n", _data.m_data.m_name.c_str());
}

//-------------------------------------------------------------------------------------------------
bool DatabaseManager::GetCityData(const std::string& _name, sCityData& _data, BoroughData* _wholeCity)
{
	if (m_tables[DataTables_Cities] == nullptr)
		return false;

	std::vector<sCityData> cities;
	Str128f sql("SELECT * FROM Cities WHERE NAME='%s'", _name.c_str());

	SQLExecuteSelect(m_tables[DataTables_Cities], sql.c_str(), [&cities](sqlite3_stmt* _stmt)
	{
		int index = 0;
		cities.resize(cities.size() + 1);
		auto& city = cities.back();
		city.m_data.m_name = (const char*)sqlite3_column_text(_stmt, index++);
		city.m_data.m_zipCode = sqlite3_column_int(_stmt, index++);
		city.m_data.m_inseeCode = sqlite3_column_int(_stmt, index++);
		city.m_timeUpdate.SetData(sqlite3_column_int(_stmt, index++));
	});

	if (cities.size() == 1)
	{
		_data = cities.back();
		GetBoroughs(_data.m_data, _data.m_boroughs);

		std::sort(_data.m_boroughs.begin(), _data.m_boroughs.end(), BoroughData::compare);

		auto it = _data.m_boroughs.begin();
		while (it != _data.m_boroughs.end())
		{
			BoroughData& data = *it;
			if (_wholeCity && data.IsWholeCity())
			{
				*_wholeCity = data;
				it = _data.m_boroughs.erase(it);
			}
			else
				++it;
		}

		return true;
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
bool DatabaseManager::RemoveCityData(const std::string& _name)
{
	if (m_tables[DataTables_Cities] == nullptr)
		return false;

	m_modified = true;

	std::vector<sCityData> cities;
	Str128f sql("DELETE FROM Cities WHERE NAME='%s'", _name.c_str());
	
	return SQLExecute(m_tables[DataTables_Cities], sql.c_str());
}

//-------------------------------------------------------------------------------------------------
bool DatabaseManager::ListAllCities(std::vector<std::string>& _list)
{
	if (m_tables[DataTables_Cities] == nullptr)
		return false;

	std::vector<sCityData> cities;
	Str128f sql("SELECT * FROM Cities");

	SQLExecuteSelect(m_tables[DataTables_Cities], sql.c_str(), [&cities](sqlite3_stmt* _stmt)
	{
		int index = 0;
		cities.resize(cities.size() + 1);
		auto& city = cities.back();
		city.m_data.m_name = (const char*)sqlite3_column_text(_stmt, index++);
		city.m_data.m_zipCode = sqlite3_column_int(_stmt, index++);
		city.m_data.m_inseeCode = sqlite3_column_int(_stmt, index++);
		city.m_timeUpdate.SetData(sqlite3_column_int(_stmt, index++));
	});

	if (cities.size() > 0)
	{
		for (auto& city : cities)
			_list.push_back(city.m_data.m_name);

		return true;
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::CreateTables()
{
	SQLExecute(m_tables[DataTables_Cities],
		"CREATE TABLE IF NOT EXISTS 'Cities' (\n"
		"`NAME` TEXT,\n"			// Name of the city
		"`ZIPCODE` INTEGER,\n"		// ZIP code
		"`INSEECODE` INTEGER,\n"		// ZIP code
		"`TIMEUPDATE` INTEGER"	// Last time the borough has been updated
		")"
	);

	SQLExecute(m_tables[DataTables_Boroughs],
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
		"`RENTT4MAX` REAL"			// Buy max price
		")"
	);
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::OpenTables()
{
	if (!m_tables[DataTables_Cities])
		m_tables[DataTables_Cities] = SQLOpenDB("C:/Tables/cities.sqlite");

	if (!m_tables[DataTables_Boroughs])
		m_tables[DataTables_Boroughs] = SQLOpenDB("C:/Tables/boroughs.sqlite");
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::CloseTables()
{
	for (int entry = 0; entry < DataTables_COUNT; ++entry)
	{
		SQLCloseDB(m_tables[entry]);
		m_tables[entry] = nullptr;
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
	GetCityData("Montpellier", city2);

	BoroughData data;
	data.m_name = "Antigone";
	data.m_city.m_name = "Montpellier";
	data.m_key = 13245;
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

	AddBoroughData(data);

	BoroughData data2;
	GetBoroughData("Montpellier", "Antigone", data2);
	printf("");
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::ComputeCityData(const std::string& _cityName)
{
	CityComputeData data;
	data.m_city = _cityName;
	m_cityComputes.push_back(data);
	m_cityComputes.back().Init();
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::ComputeBoroughData(BoroughData& _data)
{
	m_boroughComputes.push_back(_data);
	m_boroughComputes.back().Init();
}