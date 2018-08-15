#pragma once

#include <vector>
#include <map>
#include "Tools\Types.h"
#include "BoroughData.h"
#include "CityComputeData.h"

enum DataTables
{
	DataTables_NONE = -1,
	DataTables_Cities,
	DataTables_Boroughs,
	DataTables_COUNT
};

struct sqlite3;
class MySQLDatabase;

extern const std::string	s_wholeCityName;

struct sCityData
{
	sCity				m_data;
	sDate				m_timeUpdate;

	std::vector<BoroughData>	m_boroughs;
};

//-------------------------------------------------------------------------------------------------
// DATA
//-------------------------------------------------------------------------------------------------
class DatabaseManager
{
public:
	static DatabaseManager* getSingleton();

	void	Init();
	void	Process();
	void	End();

	void	AddBoroughData(const BoroughData& _data, bool _saveExternal = true);
	bool	GetBoroughData(const std::string& _cityName, const std::string& _name, BoroughData& _data);
	bool	RemoveBoroughData(const std::string& _cityName, const std::string& _name);
	bool	GetBoroughs(sCity& _city, std::vector<BoroughData>& _data);
	bool	IsCityUpdating(const std::string& _cityName);
	bool	IsBoroughUpdating(const BoroughData& _data);

	void	AddCity(const sCityData& _data);
	bool	GetCityData(const std::string& _name, sCityData& _data, BoroughData* _wholeCity = nullptr);
	bool	RemoveCityData(const std::string& _name);
	bool	ListAllCities(std::vector<sCity>& _list);
	void	ListAllCitiesWithFilter(std::vector<sCity>& _list, std::string _filter);

	void	ComputeCityData(const std::string& _cityName);
	void	ComputeBoroughData(BoroughData& _data);

	inline bool IsModified() const		{ return m_modified; }

	void	ForceBoroughReset(BoroughData& _data);
	
private:
	void	CreateTables();
	void	OpenTables();
	void	CloseTables();

	void	Test();

private:
	sqlite3*						m_tables[DataTables_COUNT];
	std::vector<CityComputeData>	m_cityComputes;
	std::vector<BoroughData>		m_boroughComputes;
	MySQLDatabase*					m_externalDB = nullptr;
	std::vector<std::pair<unsigned int, int>>				m_externalBoroughRequests;
	time_t							m_externalTimer = 0;
	bool							m_modified = false;
};