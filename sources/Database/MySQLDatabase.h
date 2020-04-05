#pragma once

#include <string>
#include "BoroughData.h"
#include <map>
#include <mutex>
#include <vector>

typedef struct st_mysql MYSQL;
typedef struct st_mysql_res MYSQL_RES;

namespace sql
{
	class Driver;
	class Connection;
	class Statement;
	class ResultSet;
	class ResultSet;
}

namespace ImmoBank
{
	class Thread;

	class MySQLDatabase;

	struct MySQLQuery
	{
		enum IO
		{
			Read,
			Write
		};

		enum Type
		{
			NONE,
			Borough,
			BoroughList,
			COUNT
		};

	public:
		MySQLQuery() {}
		MySQLQuery(Type _type, int _requestID, IO _IO) : m_type(_type), m_queryID(_requestID), m_IO(_IO) {}

		virtual void Process(MySQLDatabase* _db) = 0;
		inline Type GetType() const { return m_type; }

	public:
		int	m_queryID = -1;
		Type	m_type = Type::NONE;
		IO		m_IO = IO::Read;
		bool	m_finished = false;
		bool	m_canceled = false;
	};

	struct MySQLBoroughQuery : public MySQLQuery
	{
	public:
		MySQLBoroughQuery() {}
		MySQLBoroughQuery(int _requestID, IO _type) : MySQLQuery(Type::Borough, _requestID, _type) {}

		virtual void Process(MySQLDatabase* _db) override;

	public:
		BoroughData	m_data;
	};

	struct MySQLBoroughListQuery : public MySQLQuery
	{
	public:
		MySQLBoroughListQuery() {}
		MySQLBoroughListQuery(int _requestID, IO _type) : MySQLQuery(Type::BoroughList, _requestID, _type) {}

		virtual void Process(MySQLDatabase* _db) override;

	public:
		sCity						m_city;
		std::vector<BoroughData>	m_list;
	};

	class MySQLDatabase
	{
		friend struct MySQLQuery;
		friend struct MySQLBoroughQuery;
		friend struct MySQLBoroughListQuery;

	public:
		bool Init();
		void LoadConfigFile();
		void Process();
		void End();

		void UpdatePrivileges();

		int AskForBoroughData(BoroughData& _data);
		int AskForCityBoroughList(const sCity& _city);
		bool IsQueryAvailable(int _queryID) const;
		bool GetResultBoroughData(int _queryID, BoroughData& _data);
		bool GetResultBoroughList(int _queryID, std::vector<BoroughData>& _list);
		void WriteBoroughData(BoroughData& _data);
		MySQLQuery* GetNextQuery();
		void CancelQuery(const int _queryID);
		void Validate(const int _queryID);
		void RemoveBoroughData(BoroughData& _data);
		inline std::string GetServer() { return m_server; }
		inline std::string GetUser() { return m_user; }

		void DebugQuery(const std::string& _query);

		bool UpdateAllSeLogerKeys();
		bool UpdateAllLogicImmoKeys();
		bool UpdateAllPapKeys();
		bool UpdateLocalBaseToServer();
		bool UpdateServerToLocalBase();

	protected:
		MYSQL_RES* MySQLDatabase::ExecuteQuery(const std::string& _query) const;
		int ExecuteUpdate(const std::string& _query) const;

	private:
		int GetNextAvailableRequestID();
		MySQLQuery* AddQuery(MySQLQuery::Type _type, MySQLQuery::IO _IO, unsigned int _key, int& _requestID);

	private:
		MYSQL*  m_connexion = nullptr;

		std::map<int, MySQLQuery*>					m_queries;
		std::vector<std::pair<unsigned int, u64>>	m_timeoutQueries;

		std::mutex*	m_mutex = nullptr;
		Thread*		m_thread = nullptr;

		std::string		m_server;
		int				m_port = -1;
		std::string		m_user;
		std::string		m_password;
		std::string		m_base;

		struct sBoroughData
		{
			sBoroughData() {}
			sBoroughData(BoroughData& _data, const std::string& _request, int _requestID = -1) : m_data(_data), m_request(_request), m_requestID(_requestID) {}
			BoroughData	m_data;
			std::string m_request;
			int m_requestID = -1;
		};
		std::vector<sBoroughData>	m_boroughData;
		bool			m_updateSelogerInProgress = false;
		bool			m_updateLogicImmoInProgress = false;
		bool			m_updatePapInProgress = false;
	};
}