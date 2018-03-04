#include "DatabaseManager.h"
#include "Request/SearchRequest.h"
#include "Tools/StringTools.h"
#include "SQLDatabase.h"
#include "Online/OnlineManager.h"
#include <time.h>

DatabaseManager* s_singleton = nullptr;

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
	for (auto& compute : m_cityComputes)
		compute.Process();
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::End()
{
	CloseTables();
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::AddBoroughData(const sBoroughData& _data)
{
	if (SQLExecute(m_tables[DataTables_Boroughs], "INSERT OR REPLACE INTO Boroughs VALUES('%s', '%s', %lld, %d, %f, %f, %f, %f)",
		_data.m_cityName.c_str(),
		_data.m_name.c_str(),
		_data.m_timeUpdate.GetData(),
		_data.m_key,
		_data.m_priceBuyMin,
		_data.m_priceBuyMax,
		_data.m_priceRentMin,
		_data.m_priceRentMax))
		printf("Add borough %s to database Boroughs\n", _data.m_name.c_str());
}

//-------------------------------------------------------------------------------------------------
bool DatabaseManager::GetBoroughData(const std::string& _cityName, const std::string& _name, sBoroughData& _data)
{
	if (m_tables[DataTables_Boroughs] == nullptr)
		return false;

	std::vector<sBoroughData> boroughs;
	Str128f sql("SELECT * FROM Boroughs WHERE CITY='%s' AND BOROUGH='%s'", _cityName.c_str(), _name.c_str());

	SQLExecuteSelect(m_tables[DataTables_Boroughs], sql.c_str(), [&boroughs](sqlite3_stmt* _stmt)
	{
		int index = 0;
		boroughs.resize(boroughs.size() + 1);
		auto& borough = boroughs.back();
		borough.m_cityName = (const char*)sqlite3_column_text(_stmt, index++);
		borough.m_name = (const char*)sqlite3_column_text(_stmt, index++);
		borough.m_timeUpdate.SetData(sqlite3_column_int(_stmt, index++));
		borough.m_key = sqlite3_column_int(_stmt, index++);
		borough.m_priceBuyMin = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceBuyMax = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentMin = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentMax = (float)sqlite3_column_double(_stmt, index++);
	});

	if (boroughs.size() == 1)
	{
		_data = boroughs.back();
		return true;
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::AddCity(const sCityData& _data)
{
	if (SQLExecute(m_tables[DataTables_Cities], "INSERT OR REPLACE INTO Cities VALUES('%s', %d, %lld)",
		_data.m_name.c_str(),
		_data.m_zipCode,
		_data.m_timeUpdate.GetData()))
		printf("Add city %s to database Cities\n", _data.m_name.c_str());
}

//-------------------------------------------------------------------------------------------------
bool DatabaseManager::GetCityData(const std::string& _name, sCityData& _data)
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
		city.m_name = (const char*)sqlite3_column_text(_stmt, index++);
		city.m_zipCode = sqlite3_column_int(_stmt, index++);
		city.m_timeUpdate.SetData(sqlite3_column_int(_stmt, index++));
	});

	if (cities.size() == 1)
	{
		_data = cities.back();
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
		"`TIMEUPDATE` INTEGER,\n"	// Last time the borough has been updated
		"PRIMARY KEY(`NAME`)"
		")"
	);

	SQLExecute(m_tables[DataTables_Boroughs],
		"CREATE TABLE IF NOT EXISTS 'Boroughs' (\n"
		"`CITY` TEXT,\n"			// Name of the city
		"`BOROUGH` TEXT,\n"			// Name of the borough
		"`TIMEUPDATE` INTEGER,\n"	// Last time the borough has been updated
		"`KEY` INTEGER,\n"			// Internal meilleursagents.com key
		"`BUYMIN` REAL,\n"			// Buy min price
		"`BUYMAX` REAL,\n"			// Buy max price
		"`RENTMIN` REAL,\n"			// Rent min price
		"`RENTMAX` REAL,\n"			// Rent max price
		"PRIMARY KEY(`CITY`)"
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
	city.m_name = "Montpellier";
	city.m_zipCode = 34000;
	time_t t = time(0);   // get time now
	struct tm * now = localtime(&t);
	int year = 1900 + now->tm_year;
	city.m_timeUpdate.SetDate(year, now->tm_mon, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
	AddCity(city);

	sCityData city2;
	GetCityData("Montpellier", city2);

	sBoroughData data;
	data.m_name = "Antigone";
	data.m_cityName = "Montpellier";
	data.m_key = 13245;
	data.m_timeUpdate = date;
	data.m_priceBuyMin = 23456.1f;
	data.m_priceBuyMax = 34567.1f;
	data.m_priceRentMin = 4568.1f;
	data.m_priceRentMax = 56789.1f;
	AddBoroughData(data);

	sBoroughData data2;
	GetBoroughData("Montpellier", "Antigone", data2);
	printf("");
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::ComputeCityData(const std::string& _cityName)
{
	sCityComputeData data;
	data.m_city = _cityName;
	m_cityComputes.push_back(data);
	m_cityComputes.back().Init();
}

//-------------------------------------------------------------------------------------------------
void sCityComputeData::Init()
{
	m_state = UpdateStep_GetCityData;

	sCityData data;
	if (DatabaseManager::getSingleton()->GetCityData(m_city, data))
	{
		SearchRequestCityBoroughs boroughs;
		boroughs.m_city = m_city;
		m_boroughsListID = OnlineManager::getSingleton()->SendRequest(&boroughs);

		m_state = UpdateStep_GetBoroughList;
	}	
}

//-------------------------------------------------------------------------------------------------
bool sCityComputeData::Process()
{
	switch (m_state)
	{
	// Borough list
	case UpdateStep_GetBoroughList:
		if ((m_boroughsListID > -1) && OnlineManager::getSingleton()->IsRequestAvailable(m_boroughsListID))
		{
			std::vector<SearchRequestResult*> list;
			OnlineManager::getSingleton()->GetRequestResult(m_boroughsListID, list);
			m_boroughsListID = -1;

			for (auto result : list)
			{
				if (result->m_resultType == SearchRequestType_CityBoroughs)
				{
					SearchRequestResulCityBorough* borough = static_cast<SearchRequestResulCityBorough*>(result);
					sBoroughData data;
					data.m_cityName = m_city;
					data.m_name = borough->m_name;
					m_boroughs.push_back(data);
					delete borough;
				}
			}

			//std::string request = "https://www.meilleursagents.com/prix-immobilier/montpellier-34000/quartier_antigone-170492247/"

			IncreaseStep();
		}
		break;

	// Boroughs prices
	case UpdateStep_ComputeBoroughsPrices:
		{
			if (true)
			{
				IncreaseStep();
			}
		}
		break;
	}
	
	return m_state == UpdateStep_COUNT;
}

//-------------------------------------------------------------------------------------------------
void sCityComputeData::End()
{

}