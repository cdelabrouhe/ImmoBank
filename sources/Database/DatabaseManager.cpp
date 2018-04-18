#include "DatabaseManager.h"
#include "Request/SearchRequest/SearchRequest.h"
#include "Tools/StringTools.h"
#include "SQLDatabase.h"
#include "Online/OnlineManager.h"
#include <time.h>
#include "extern/ImGui/imgui.h"
#include "Request/SearchRequest/SearchRequestCityBoroughs.h"
#include "Request/SearchRequest/SearchRequestResulCityBorough.h"

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

	if (m_displayCityData)
		DisplayCityInformation();
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::End()
{
	CloseTables();
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::AddBoroughData(const sBoroughData& _data)
{
	RemoveBoroughData(_data.m_cityName, _data.m_name);

	if (SQLExecute(m_tables[DataTables_Boroughs], "INSERT OR REPLACE INTO Boroughs (CITY, BOROUGH, TIMEUPDATE, KEY, APARTMENTBUYMIN, APARTMENTBUYMAX, HOUSEBUYMIN, HOUSEBUYMAX, RENTMIN, RENTMAX) VALUES('%s', '%s', %u, %u, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f)",
		_data.m_cityName.c_str(),
		_data.m_name.c_str(),
		_data.m_timeUpdate.GetData(),
		_data.m_key,
		_data.m_priceApartmentBuyMin,
		_data.m_priceApartmentBuyMax,
		_data.m_priceHouseBuyMin,
		_data.m_priceHouseBuyMax,
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
		borough.m_priceApartmentBuyMin = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceApartmentBuyMax = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceHouseBuyMin = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceHouseBuyMax = (float)sqlite3_column_double(_stmt, index++);
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
bool DatabaseManager::RemoveBoroughData(const std::string& _cityName, const std::string& _name)
{
	if (m_tables[DataTables_Boroughs] == nullptr)
		return false;

	std::vector<sCityData> cities;
	Str128f sql("DELETE FROM Boroughs WHERE CITY='%s' AND BOROUGH='%s'", _cityName.c_str(), _name.c_str());

	return SQLExecute(m_tables[DataTables_Boroughs], sql.c_str());
}

//-------------------------------------------------------------------------------------------------
bool DatabaseManager::GetBoroughs(const std::string& _cityName, std::vector<sBoroughData>& _data)
{
	if (m_tables[DataTables_Boroughs] == nullptr)
		return false;

	_data.clear();
	Str128f sql("SELECT * FROM Boroughs WHERE CITY='%s'", _cityName.c_str());

	SQLExecuteSelect(m_tables[DataTables_Boroughs], sql.c_str(), [&_data](sqlite3_stmt* _stmt)
	{
		int index = 0;
		_data.resize(_data.size() + 1);
		auto& borough = _data.back();
		borough.m_cityName = (const char*)sqlite3_column_text(_stmt, index++);
		borough.m_name = (const char*)sqlite3_column_text(_stmt, index++);
		borough.m_timeUpdate.SetData(sqlite3_column_int(_stmt, index++));
		borough.m_key = sqlite3_column_int(_stmt, index++);
		borough.m_priceApartmentBuyMin = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceApartmentBuyMax = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceHouseBuyMin = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceHouseBuyMax = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentMin = (float)sqlite3_column_double(_stmt, index++);
		borough.m_priceRentMax = (float)sqlite3_column_double(_stmt, index++);
	});

	if (_data.size() >= 1)
		return true;

	return false;
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::AddCity(const sCityData& _data)
{
	RemoveCityData(_data.m_name);

	if (SQLExecute(m_tables[DataTables_Cities], "INSERT OR REPLACE INTO Cities (NAME, ZIPCODE, TIMEUPDATE) VALUES('%s', %d, %u)",
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
		GetBoroughs(_data.m_name, _data.m_boroughs);

		return true;
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
bool DatabaseManager::RemoveCityData(const std::string& _name)
{
	if (m_tables[DataTables_Cities] == nullptr)
		return false;

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
		city.m_name = (const char*)sqlite3_column_text(_stmt, index++);
		city.m_zipCode = sqlite3_column_int(_stmt, index++);
		city.m_timeUpdate.SetData(sqlite3_column_int(_stmt, index++));
	});

	if (cities.size() > 0)
	{
		for (auto& city : cities)
			_list.push_back(city.m_name);

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
		"`TIMEUPDATE` INTEGER"	// Last time the borough has been updated
		")"
	);

	SQLExecute(m_tables[DataTables_Boroughs],
		"CREATE TABLE IF NOT EXISTS 'Boroughs' (\n"
		"`CITY` TEXT,\n"			// Name of the city
		"`BOROUGH` TEXT,\n"			// Name of the borough
		"`TIMEUPDATE` INTEGER,\n"	// Last time the borough has been updated
		"`KEY` INTEGER,\n"			// Internal meilleursagents.com key
		"`APARTMENTBUYMIN` REAL,\n"	// Buy min price
		"`APARTMENTBUYMAX` REAL,\n"	// Buy max price
		"`HOUSEBUYMIN` REAL,\n"		// Buy min price
		"`HOUSEBUYMAX` REAL,\n"		// Buy max price
		"`RENTMIN` REAL,\n"			// Rent min price
		"`RENTMAX` REAL"			// Rent max price
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
	data.m_priceApartmentBuyMin = 23456.1f;
	data.m_priceApartmentBuyMax = 34567.1f;
	data.m_priceHouseBuyMin = 246.8f;
	data.m_priceHouseBuyMax = 357.9f;
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
void DatabaseManager::AskForDisplayCityInformation()
{
	m_displayCityData = !m_displayCityData;
	if (m_displayCityData)
		InitDisplayCityInformation();		
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::InitDisplayCityInformation()
{
	m_cityListRequested = false;
	m_selectedCityID = 0;
	m_cityListFull;
	m_hovered = -1;
	m_selected = -1;
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::DisplayCityInformation()
{
	if (!m_cityListRequested)
	{
		m_cityListFull.clear();
		ListAllCities(m_cityListFull);
		m_cityListRequested = true;
	}

	ImGui::SetNextWindowSize(ImVec2(900, 500), ImGuiCond_FirstUseEver);
	ImGui::Begin("City info display", &m_displayCityData);

	// Left panel (city selector process)
	bool listUpdated = false;
	std::vector<std::string> cityListFiltered;
	ImGui::BeginChild("City search", ImVec2(300, 0), true);
	ImGui::InputText("City name", (char*)m_inputTextCity, 256);
	if (strlen(m_inputTextCity) > 0)
	{
		std::string input = m_inputTextCity;
		StringTools::TransformToLower(input);
		for (auto city : m_cityListFull)
		{
			std::string tmp = city;
			StringTools::TransformToLower(tmp);
			auto findID = tmp.find(m_inputTextCity);
			if (findID != std::string::npos)
				cityListFiltered.push_back(city);
		}

		listUpdated = true;
	}
	ImGui::Separator();

	if (!listUpdated)
	{
		for (auto& city : m_cityListFull)
			cityListFiltered.push_back(city);
	}

	int localHovered = -1;
	int cpt = 0;
	const ImColor colSelected(1.0f, 0.0f, 0.0f);
	const ImColor colHovered(0.0f, .5f, .5f);
	for (auto& city : cityListFiltered)
	{
		const bool isHovered = (m_hovered == cpt);
		const bool isSelected = (m_selected == cpt);
		if (isSelected)
			ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)colSelected);
		else if (isHovered)
			ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)colHovered);
		
		ImGui::Text(city.c_str());
		if (ImGui::IsItemHovered())
			m_hovered = cpt;

		if (ImGui::IsItemClicked())
			m_selected = cpt;

		if (isSelected || isHovered)
			ImGui::PopStyleColor();

		++cpt;
	}

	if (m_selected >= cityListFiltered.size())
		m_selected = -1;

	sCityData selectedCity;
	if (m_selected > -1)
		GetCityData(cityListFiltered[m_selected], selectedCity);

	ImGui::EndChild();
	ImGui::SameLine();

	// Right panel (infos display)
	ImGui::BeginChild("Item view", ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing())); // Leave room for 1 line below us

	if (!selectedCity.m_name.empty())
	{
		ImGui::Text("Name: %s", selectedCity.m_name.c_str());
		ImGui::Text("ZipCode: %d", selectedCity.m_zipCode);
		if (ImGui::TreeNode("Boroughs"))
		{
			for (auto& borough : selectedCity.m_boroughs)
			{
				ImGui::Text("%s", borough.m_name.c_str());
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::Text("Key: %u", borough.m_key);
					ImGui::Text("App buy min: %.2f", borough.m_priceApartmentBuyMin);
					ImGui::Text("App buy max: %.2f", borough.m_priceApartmentBuyMax);
					ImGui::Text("House buy min: %.2f", borough.m_priceHouseBuyMin);
					ImGui::Text("House buy max: %.2f", borough.m_priceHouseBuyMax);
					ImGui::Text("Rent min: %.2f", borough.m_priceRentMin);
					ImGui::Text("Rent max: %.2f", borough.m_priceRentMax);
					ImGui::EndTooltip();
				}
			}
			ImGui::TreePop();
		}
	}
	
	ImGui::EndChild();

	ImGui::End();
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