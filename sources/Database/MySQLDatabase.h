#pragma once

#include <string>
#include "BoroughData.h"
#include <map>
#include <mutex>
#include <vector>

class Thread;

namespace sql
{
	class Driver;
	class Connection;
	class Statement;
	class ResultSet;
	class ResultSet;
}

class MySQLDatabase;
struct MySQLBoroughQuery
{
	enum Type
	{
		Type_Read,
		Type_Write
	};

public:
	MySQLBoroughQuery()	{}
	MySQLBoroughQuery(int _requestID, Type _type, BoroughData& _data) : m_queryID(_requestID), m_type(_type), m_data(_data)	{}

	void Process(MySQLDatabase* _db);

public:
	int	m_queryID = -1;
	Type m_type = Type::Type_Read;
	BoroughData	m_data;
	bool	m_finished = false;
	bool	m_canceled = false;
};

class MySQLDatabase
{
	friend struct MySQLBoroughQuery;

public:
	void Init();
	void Process();
	void End();

	int AskForBoroughData(BoroughData& _data);
	bool IsQueryAvailable(int _queryID) const;
	bool GetResultBoroughData(int _queryID, BoroughData& _data);
	void WriteBoroughData(BoroughData& _data);
	bool GetNextQuery(MySQLBoroughQuery& _request);
	void CancelQuery(const int _queryID);
	void Validate(const int _queryID, BoroughData& _data);

protected:
	sql::ResultSet* MySQLDatabase::ExecuteQuery(const std::string& _query, sql::Statement* _statement) const;
	int ExecuteUpdate(const std::string& _query) const;

private:
	int GetNextAvailableRequestID();
	int AddQuery(MySQLBoroughQuery::Type _type, BoroughData& _data);

private:
	sql::Driver* m_driver = nullptr;
	sql::Connection* m_connexion = nullptr;

	std::map<int, MySQLBoroughQuery>	m_queries;
	std::vector<std::pair<unsigned int, u64>>		m_timeoutQueries;

	std::mutex*	m_mutex = nullptr;
	Thread*		m_thread = nullptr;
};