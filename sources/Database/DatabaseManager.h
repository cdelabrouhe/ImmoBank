#pragma once

#include <vector>
#include <map>
#include "Tools\Types.h"

enum DataTables
{
	DataTables_NONE = -1,
	DataTables_Cities,
	DataTables_Boroughs,
	DataTables_COUNT
};

struct sqlite3;

struct sCityData
{
	std::string			m_name;
	int					m_zipCode = 0;
	sDate				m_timeUpdate;
};

struct sBoroughData
{
	std::string			m_name;
	std::string			m_cityName;
	sDate				m_timeUpdate;
	int					m_key;
	float				m_priceBuyMin;
	float				m_priceBuyMax;
	float				m_priceRentMin;
	float				m_priceRentMax;
};

struct sCityComputeData
{
	enum UpdateStep
	{
		UpdateStep_NONE = -1,
		UpdateStep_GetBoroughList,
		UpdateStep_ComputeBoroughsPrices,
		UpdateStep_COUNT
	};

	void Init();
	bool Process();
	void End();

	std::string					m_city;
	std::vector<sBoroughData>	m_boroughs;
	int							m_boroughsListID = -1;

private:
	void IncreaseStep()			{ m_state = (UpdateStep)((int)m_state + 1); }

private:
	UpdateStep		m_state = UpdateStep_NONE;
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

	void	AddBoroughData(const sBoroughData& _data);
	bool	GetBoroughData(const std::string& _cityName, const std::string& _name, sBoroughData& _data);

	void	AddCity(const sCityData& _data);
	bool	GetCityData(const std::string& _name, sCityData& _data);

	void	ComputeCityData(const std::string& _cityName);
	
private:
	void	CreateTables();
	void	OpenTables();
	void	CloseTables();

	void	Test();

private:
	sqlite3*						m_tables[DataTables_COUNT];
	std::vector<sCityComputeData>	m_cityComputes;
};