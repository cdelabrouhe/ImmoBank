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

struct sPrice
{
	sPrice(float _val = 0.f, float _min = 0.f, float _max = 0.f)
		: m_val(_val), m_min(_min), m_max(_max)	{}
	float m_min = 0.f;
	float m_val = 0.f;
	float m_max = 0.f;
};

struct sBoroughData
{
public:
	std::string			m_name;
	sCity				m_city;
	sDate				m_timeUpdate;
	unsigned int		m_key = 0xffffffff;
	sPrice				m_priceRentApartmentT1;
	sPrice				m_priceRentApartmentT2;
	sPrice				m_priceRentApartmentT3;
	sPrice				m_priceRentApartmentT4Plus;
	sPrice				m_priceBuyApartment;
	sPrice				m_priceBuyHouse;
	sPrice				m_priceRentHouse;

	void Init();
	bool Process();
	void End();

	static bool compare(const sBoroughData &_a, const sBoroughData &_b)
	{
		return _a.m_name < _b.m_name;
	}

private:
	int m_httpRequestID = -1;
};

struct sCityData
{
	sCity				m_data;
	sDate				m_timeUpdate;

	std::vector<sBoroughData>	m_boroughs;
};

struct sCityComputeData
{
	enum UpdateStep
	{
		UpdateStep_NONE = -1,
		UpdateStep_GetCityData,
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
	bool	RemoveBoroughData(const std::string& _cityName, const std::string& _name);
	bool	GetBoroughs(sCity& _city, std::vector<sBoroughData>& _data);
	bool	IsBoroughUpdating(sBoroughData& _data);

	void	AddCity(const sCityData& _data);
	bool	GetCityData(const std::string& _name, sCityData& _data);
	bool	RemoveCityData(const std::string& _name);
	bool	ListAllCities(std::vector<std::string>& _list);

	void	ComputeCityData(const std::string& _cityName);
	void	ComputeBoroughData(sBoroughData& _data);
	
	void	AskForDisplayCityInformation();
	void	InitDisplayCityInformation();
	void	DisplayCityInformation();

private:
	void	CreateTables();
	void	OpenTables();
	void	CloseTables();

	void	Test();

private:
	sqlite3*						m_tables[DataTables_COUNT];
	std::vector<sCityComputeData>	m_cityComputes;
	std::vector<sBoroughData>		m_boroughComputes;

	// Display panel
	char							m_inputTextCity[256];
	int								m_selectedCityID = 0;
	std::vector<std::string>		m_cityListFull;
	int								m_hovered = -1;
	int								m_selected = -1;
	bool							m_displayCityData = false;
	bool							m_cityListRequested = false;
};