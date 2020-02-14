#pragma once

#include <vector>
#include <map>
#include "Tools\Types.h"
#include "BoroughData.h"
#include "CityComputeData.h"
#include "CityUpdateData.h"

struct sqlite3;

namespace ImmoBank
{
	enum DataTables
	{
		DataTables_NONE = -1,
		DataTables_Cities,
		DataTables_Boroughs,
		DataTables_COUNT
	};

	class MySQLDatabase;
	class ImageDatabase;

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
		void	GetAllBoroughs(std::vector<BoroughData>& _data);
		bool	IsCityUpdating(const std::string& _cityName);
		bool	IsBoroughUpdating(const BoroughData& _data);

		void	AddCity(const sCityData& _data);
		bool	GetCityData(const std::string& _name, sCityData& _data, BoroughData* _wholeCity = nullptr);
		bool	RemoveCityData(const std::string& _name);
		bool	ListAllCities(std::vector<sCity>& _list);
		void	ListAllCitiesWithFilter(std::vector<sCity>& _list, std::string _filter);

		void	UpdateCityData(const sCity& _city);
		void	ComputeCityData(const std::string& _cityName);
		void	ComputeBoroughData(BoroughData& _data);

		int		AskForExternalDBCityBoroughs(const sCity& _city);
		bool	IsExternalDBCityBoroughsAvailable(int _requestID, std::vector<BoroughData>& _boroughs);

		inline bool IsModified() const { return m_modified; }

		void	ForceBoroughReset(BoroughData& _data);

		inline bool IsConnectionValid() const { return m_connectionValid; }

		void GetConnectionParameters(std::string& _server, std::string& _user);

		void TriggerExternalSQLCommand(const std::string& _query);
		
		void	DisplayDebug();
		void	DisplayMySQLRequestsPanel();
		void	NotifyMySQLEvent(const std::string& _request);

	private:
		void	CreateTables();
		void	OpenTables();
		void	CloseTables();

		void	Test();

	public:
		bool							m_displayDebug = false;

	private:
		sqlite3*						m_tables[DataTables_COUNT];
		std::vector<CityComputeData>	m_cityComputes;
		std::vector<CityUpdateData>		m_cityUpdates;
		std::vector<BoroughData>		m_boroughComputes;
		MySQLDatabase*					m_externalDB = nullptr;
		std::vector<std::pair<unsigned int, int>>				m_externalBoroughRequests;
		time_t							m_externalTimer = 0;
		bool							m_modified = false;
		bool							m_connectionValid = false;

		// Debug panel
		char							m_MySQLInputDebug[2048];
		std::vector<std::string>		m_MySQLRequests;

	public:
		bool							m_generateSeLogerIndices = false;
		bool							m_generateLogicImmoIndices = false;
		bool							m_updateLocalBaseToServer = false;
	};
}