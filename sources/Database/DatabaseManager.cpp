#include "DatabaseManager.h"
#include "Request/SearchRequest/SearchRequest.h"
#include "Tools/StringTools.h"
#include "SQLDatabase.h"
#include "Online/OnlineManager.h"
#include <time.h>
#include "extern/ImGui/imgui.h"
#include "Request/SearchRequest/SearchRequestCityBoroughs.h"
#include "Request/SearchRequest/SearchRequestResulCityBorough.h"
#include "Request/SearchRequest/SearchRequestResulCityBoroughData.h"
#include "Request/SearchRequest/SearchRequestCityBoroughData.h"
#include <algorithm>

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
	auto itCity = m_cityComputes.begin();
	while (itCity != m_cityComputes.end())
	{
		sCityComputeData& data = *itCity;
		if (data.Process())
			itCity = m_cityComputes.erase(itCity);
		else
			m_cityComputes.erase(itCity);
	}

	auto itBorough = m_boroughComputes.begin();
	while (itBorough != m_boroughComputes.end())
	{
		sBoroughData& data = *itBorough;
		if (data.Process())
			itBorough = m_boroughComputes.erase(itBorough);
		else
			m_boroughComputes.erase(itBorough);
	}

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

	if (SQLExecute(m_tables[DataTables_Boroughs], "INSERT OR REPLACE INTO Boroughs (CITY, BOROUGH, TIMEUPDATE, KEY, APARTMENTBUY, APARTMENTBUYMIN, APARTMENTBUYMAX, HOUSEBUY, HOUSEBUYMIN, HOUSEBUYMAX, RENTHOUSE, RENTHOUSEMIN, RENTHOUSEMAX, RENTT1, RENTT1MIN, RENTT1MAX, RENTT2, RENTT2MIN, RENTT2MAX, RENTT3, RENTT3MIN, RENTT3MAX, RENTT4, RENTT4MIN, RENTT4MAX) VALUES('%s', '%s', %u, %u, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f)",
		_data.m_cityName.c_str(),
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

		std::sort(_data.m_boroughs.begin(), _data.m_boroughs.end(), sBoroughData::compare);

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
void DatabaseManager::ComputeBoroughData(sBoroughData& _data)
{
	m_boroughComputes.push_back(_data);
	m_boroughComputes.back().Init();
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

	if (m_selected >= (int)cityListFiltered.size())
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
			int cpt = 0;
			for (auto& borough : selectedCity.m_boroughs)
			{
				ImGui::PushID(this + cpt);
				bool update = ImGui::Button("Update");
				ImGui::PopID();

				ImGui::SameLine();

				ImGui::Text("%s", borough.m_name.c_str());
				bool hovered = ImGui::IsItemHovered();
				if (hovered)
				{
					ImGui::BeginTooltip();
					ImGui::Text("Key: %u", borough.m_key);
					ImGui::Text("App buy min: %.2f", borough.m_priceBuyApartment.m_min);
					ImGui::Text("App buy: %.2f", borough.m_priceBuyApartment.m_val);
					ImGui::Text("App buy max: %.2f", borough.m_priceBuyApartment.m_max);
					ImGui::Text("House buy min: %.2f", borough.m_priceBuyHouse.m_min);
					ImGui::Text("House buy: %.2f", borough.m_priceBuyHouse.m_val);
					ImGui::Text("House buy max: %.2f", borough.m_priceBuyHouse.m_max);
					ImGui::Text("T1 rent min: %.2f", borough.m_priceRentApartmentT1.m_min);
					ImGui::Text("T1 rent: %.2f", borough.m_priceRentApartmentT1.m_val);
					ImGui::Text("T1 rent max: %.2f", borough.m_priceRentApartmentT1.m_max);
					ImGui::Text("T2 rent min: %.2f", borough.m_priceRentApartmentT2.m_min);
					ImGui::Text("T2 rent: %.2f", borough.m_priceRentApartmentT2.m_val);
					ImGui::Text("T2 rent max: %.2f", borough.m_priceRentApartmentT2.m_max);
					ImGui::Text("T3 rent min: %.2f", borough.m_priceRentApartmentT3.m_min);
					ImGui::Text("T3 rent: %.2f", borough.m_priceRentApartmentT3.m_val);
					ImGui::Text("T3 rent max: %.2f", borough.m_priceRentApartmentT3.m_max);
					ImGui::Text("T4 rent min: %.2f", borough.m_priceRentApartmentT4Plus.m_min);
					ImGui::Text("T4 rent: %.2f", borough.m_priceRentApartmentT4Plus.m_val);
					ImGui::Text("T4 rent max: %.2f", borough.m_priceRentApartmentT4Plus.m_max);
					ImGui::Text("House rent min: %.2f", borough.m_priceRentHouse.m_min);
					ImGui::Text("House rent: %.2f", borough.m_priceRentHouse.m_val);
					ImGui::Text("House rent max: %.2f", borough.m_priceRentHouse.m_max);
					ImGui::EndTooltip();
				}

				if (update)
				{
					ComputeBoroughData(borough);
				}

				++cpt;
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

//-------------------------------------------------------------------------------------------------
void sBoroughData::Init()
{
	SearchRequestCityBoroughData request;
	request.m_data = *this;
	request.m_city = m_cityName;
	m_httpRequestID = OnlineManager::getSingleton()->SendRequest(&request);
}

//-------------------------------------------------------------------------------------------------
bool sBoroughData::Process()
{
	if (OnlineManager::getSingleton()->IsRequestAvailable(m_httpRequestID))
	{
		std::vector<SearchRequestResult*> list;
		if (OnlineManager::getSingleton()->GetRequestResult(m_httpRequestID, list))
		{
			m_httpRequestID = -1;

			for (auto result : list)
			{
				if (result->m_resultType == SearchRequestType_CityBoroughData)
				{
					SearchRequestResulCityBoroughData* borough = static_cast<SearchRequestResulCityBoroughData*>(result);
					DatabaseManager::getSingleton()->AddBoroughData(borough->m_data);
				}
			}
		}

		return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
void sBoroughData::End()
{

}
