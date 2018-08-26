#include "MySQLDatabase.h"

#include <../../jsoncpp/value.h>
#include "Tools/Tools.h"
#include "Tools/Thread/Thread.h"
#include <time.h>

#include "extern/jsoncpp/value.h"

#define MYSQL_ACTIVE
#include <mysql.h>

#ifdef WIN32
#ifdef _DEBUG
#pragma comment(lib, "extern/libmariadb/bin/Win32/Debug/libmariadb.lib")
#else
#pragma comment(lib, "extern/libmariadb/bin/Win32/Release/libmariadb.lib")
#endif

#else

#ifdef _DEBUG
#pragma comment(lib, "extern/libmariadb/bin/x64/Debug/libmariadb.lib")
#else
#pragma comment(lib, "extern/libmariadb/bin/x64/Release/libmariadb.lib")
#endif
#endif

#ifdef MYSQL_ACTIVE
#define SQL_NO_ERROR 0
#endif

static const u64 s_timeoutQuery = 30;

#ifdef DEV_MODE
#include "UI/UIManager.h"
#endif

void NotifyMySQLEvent(const std::string& _str)
{
#ifdef DEV_MODE
	UIManager::getSingleton()->NotifyMySQLEvent(_str);
#endif
}

#ifdef MYSQL_ACTIVE
//--------------------------------------------------------------------------------------
static void DisplayMySQLMessage(const std::string& _message)
{
	std::string str = "MESSAGE: " + _message + "\n";
	printf(str.c_str());
	NotifyMySQLEvent(str);
}
#endif

//------------------------------------------------------------------------------------------------
unsigned int MySQLThreadStart(void* arg)
{
	MySQLDatabase* db = (MySQLDatabase*)arg;
	while (true)
	{
		MySQLBoroughQuery query;
		if (db->GetNextQuery(query))
		{
			query.Process(db);
			std::this_thread::sleep_for(std::chrono::milliseconds(25));
		}
		else
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
void MySQLDatabase::LoadConfigFile()
{
#ifdef MYSQL_ACTIVE
	// Load config file
	std::string path = Tools::GetExePath();
	path += "database.cfg";

	Json::Value root;
	if (Tools::ReadJSON(path.c_str(), root))
	{
		m_server = root["Server"].asString();
		m_port = root["Port"].asUInt();
		m_user = root["User"].asString();
		m_password = root["Password"].asString();
		m_base = root["Base"].asString();
	}
#endif
}

//--------------------------------------------------------------------------------------
bool MySQLDatabase::Init()
{
	bool result = true;

#ifdef MYSQL_ACTIVE
	m_mutex = new std::mutex();

	result = false;

	LoadConfigFile();

	// Init thread
	m_thread = new Thread();
	m_thread->start(MySQLThreadStart, this, "MySQLRequestsThread");

	MYSQL *mysql = mysql_init(NULL);
	
	m_connexion = mysql_real_connect(mysql
		, m_server.c_str()
		, m_user.c_str()
		, m_password.c_str()
		, m_base.c_str()
		, m_port, nullptr, 0);

	char buf[4096];
	if (m_connexion != nullptr)
	{
		sprintf(buf, "User '%s' connected to database '%s' on server %s:%d", m_user.c_str(), m_base.c_str(), m_server.c_str(), m_port);
		std::string message = buf;
		DisplayMySQLMessage(message);

		return true;
	}
	else
	{
		sprintf(buf, "CAN'T CONNECT User '%s' to database '%s' on server %s:%d", m_user.c_str(), m_base.c_str(), m_server.c_str(), m_port);
		std::string message = buf;
		DisplayMySQLMessage(message);

		return false;
	}
#endif

	return result;
}

//--------------------------------------------------------------------------------------
void MySQLDatabase::Process()
{
#ifdef MYSQL_ACTIVE
	m_mutex->lock();

	// Search for a valid request ID
	auto it = m_queries.begin();
	while (it != m_queries.end())
	{
		if (it->second.m_canceled)
			it = m_queries.erase(it);
		else
			++it;
	}
	m_mutex->unlock();

	// Check timeout queries
	auto curTime = time(0);
	auto itTimeout = m_timeoutQueries.begin();
	while (itTimeout != m_timeoutQueries.end())
	{
		int result = curTime - itTimeout->second;
		if (result > s_timeoutQuery)
			itTimeout = m_timeoutQueries.erase(itTimeout);
		else
			++itTimeout;
	}
#endif
}

//--------------------------------------------------------------------------------------
void MySQLBoroughQuery::Process(MySQLDatabase* _db)
{
#ifdef MYSQL_ACTIVE
	switch (m_type)
	{
	case Type_Read:
	{
		char buf[512];
		sprintf(buf, "SELECT * FROM BOROUGHS WHERE CITY='%s' AND BOROUGH='%s'", m_data.m_city.m_name.c_str(), m_data.m_name.c_str());
		std::string str = buf;
		MYSQL_RES* result = _db->ExecuteQuery(str);

		while (MYSQL_ROW row = mysql_fetch_row(result))
		{
			int rowID = 0;
			m_data.m_city.m_name = row[rowID++];
			m_data.m_name = row[rowID++];
			m_data.m_timeUpdate.SetData(strtoul(row[rowID++], nullptr, 10));
			m_data.m_key = strtoul(row[rowID++], nullptr, 10);
			m_data.m_priceBuyApartment.m_val = strtod(row[rowID++], nullptr);
			m_data.m_priceBuyApartment.m_min = strtod(row[rowID++], nullptr);
			m_data.m_priceBuyApartment.m_max = strtod(row[rowID++], nullptr);
			m_data.m_priceBuyHouse.m_val = strtod(row[rowID++], nullptr);
			m_data.m_priceBuyHouse.m_min = strtod(row[rowID++], nullptr);
			m_data.m_priceBuyHouse.m_max = strtod(row[rowID++], nullptr);
			m_data.m_priceRentHouse.m_val = strtod(row[rowID++], nullptr);
			m_data.m_priceRentHouse.m_min = strtod(row[rowID++], nullptr);
			m_data.m_priceRentHouse.m_max = strtod(row[rowID++], nullptr);
			m_data.m_priceRentApartmentT1.m_val = strtod(row[rowID++], nullptr);
			m_data.m_priceRentApartmentT1.m_min = strtod(row[rowID++], nullptr);
			m_data.m_priceRentApartmentT1.m_max = strtod(row[rowID++], nullptr);
			m_data.m_priceRentApartmentT2.m_val = strtod(row[rowID++], nullptr);
			m_data.m_priceRentApartmentT2.m_min = strtod(row[rowID++], nullptr);
			m_data.m_priceRentApartmentT2.m_max = strtod(row[rowID++], nullptr);
			m_data.m_priceRentApartmentT3.m_val = strtod(row[rowID++], nullptr);
			m_data.m_priceRentApartmentT3.m_min = strtod(row[rowID++], nullptr);
			m_data.m_priceRentApartmentT3.m_max = strtod(row[rowID++], nullptr);
			m_data.m_priceRentApartmentT4Plus.m_val = strtod(row[rowID++], nullptr);
			m_data.m_priceRentApartmentT4Plus.m_min = strtod(row[rowID++], nullptr);
			m_data.m_priceRentApartmentT4Plus.m_max = strtod(row[rowID++], nullptr);
		}

		mysql_free_result(result);

			/*static bool s_test = false;
			if (s_test)
			{
				FILE* f = fopen("error.txt", "wt");
				if (f)
				{
					fwrite(str.data(), sizeof(char), (size_t)str.size(), f);
					fclose(f);
				}
			}*/
	}
	break;
	case Type_Write:
	{
		// Remove current borough data
		_db->RemoveBoroughData(m_data);

		// Insert new borough data
		char buf[4096];
		memset(buf, 0, 4096);
		sprintf(buf, "INSERT INTO BOROUGHS (CITY, BOROUGH, TIMEUPDATE, BOROUGHKEY, APARTMENTBUY, APARTMENTBUYMIN, APARTMENTBUYMAX, HOUSEBUY, HOUSEBUYMIN, HOUSEBUYMAX, RENTHOUSE, RENTHOUSEMIN, RENTHOUSEMAX, RENTT1, RENTT1MIN, RENTT1MAX, RENTT2, RENTT2MIN, RENTT2MAX, RENTT3, RENTT3MIN, RENTT3MAX, RENTT4, RENTT4MIN, RENTT4MAX) VALUES('%s', '%s', %u, %u, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f)",
			m_data.m_city.m_name.c_str(),
			m_data.m_name.c_str(),
			m_data.m_timeUpdate.GetData(),
			m_data.m_key,
			m_data.m_priceBuyApartment.m_val,
			m_data.m_priceBuyApartment.m_min,
			m_data.m_priceBuyApartment.m_max,
			m_data.m_priceBuyHouse.m_val,
			m_data.m_priceBuyHouse.m_min,
			m_data.m_priceBuyHouse.m_max,
			m_data.m_priceRentHouse.m_val,
			m_data.m_priceRentHouse.m_min,
			m_data.m_priceRentHouse.m_max,
			m_data.m_priceRentApartmentT1.m_val,
			m_data.m_priceRentApartmentT1.m_min,
			m_data.m_priceRentApartmentT1.m_max,
			m_data.m_priceRentApartmentT2.m_val,
			m_data.m_priceRentApartmentT2.m_min,
			m_data.m_priceRentApartmentT2.m_max,
			m_data.m_priceRentApartmentT3.m_val,
			m_data.m_priceRentApartmentT3.m_min,
			m_data.m_priceRentApartmentT3.m_max,
			m_data.m_priceRentApartmentT4Plus.m_val,
			m_data.m_priceRentApartmentT4Plus.m_min,
			m_data.m_priceRentApartmentT4Plus.m_max);

		std::string str = buf;
		_db->ExecuteUpdate(str);

		/*static bool s_test = false;
		if (s_test)
		{
			FILE* f = fopen("error.txt", "wt");
			if (f)
				{
				fwrite(str.data(), sizeof(char), (size_t)str.size(), f);
				fclose(f);
			}
		}*/
	}
	break;
	}

	_db->Validate(m_queryID, m_data);
#endif
}


//--------------------------------------------------------------------------------------
void MySQLDatabase::End()
{
#ifdef MYSQL_ACTIVE
	// Stop thread
	m_thread->stop();
	delete m_thread;

	// Stop MySQL connection
	if (m_connexion)
	{
		mysql_close(m_connexion);
		m_connexion = nullptr;

		char buf[4096];
		sprintf(buf, "Deconnection to server '%s'", m_server.c_str());
		std::string message = buf;
		DisplayMySQLMessage(message);
	}

	delete m_mutex;
#endif
}

//--------------------------------------------------------------------------------------
int MySQLDatabase::GetNextAvailableRequestID()
{
	int ID = 0;
	while (m_queries.find(ID) != m_queries.end())
		++ID;
	return ID;
}

//--------------------------------------------------------------------------------------
int MySQLDatabase::AddQuery(MySQLBoroughQuery::Type _type, BoroughData& _data)
{
#ifdef MYSQL_ACTIVE
	unsigned int key = _data.m_key;

#ifndef _WIN32
	auto it = std::find_if(m_timeoutQueries.begin(), m_timeoutQueries.end(), [key](std::pair<unsigned int, u64>& _pair)->bool
	{
		return (_pair.first == key);
	});
#else
	bool found = false;
	auto it = m_timeoutQueries.begin();
	while (!found && (it != m_timeoutQueries.end()))
	{
		auto& pair = *it;
		found = (pair.first == key);
		++it;
	}
#endif

	// Manage timeout
	if (it != m_timeoutQueries.end())
	{
		if (_type == MySQLBoroughQuery::Type_Read)
			return -1;
		else
			m_timeoutQueries.erase(it);
	}

	m_mutex->lock();
	int requestID = GetNextAvailableRequestID();
	m_queries[requestID] = MySQLBoroughQuery(requestID, _type, _data);
	m_mutex->unlock();

	// Store in timeout list
	m_timeoutQueries.push_back(std::make_pair(_data.m_key, time(0)));

	return requestID;
#else
	return -1;
#endif
}

//--------------------------------------------------------------------------------------
int MySQLDatabase::AskForBoroughData(BoroughData& _data)
{
	return AddQuery(MySQLBoroughQuery::Type_Read, _data);
}

//--------------------------------------------------------------------------------------
bool MySQLDatabase::IsQueryAvailable(int _queryID) const
{
	bool valReturn = false;
	m_mutex->lock();
	auto it = m_queries.find(_queryID);
	if (it != m_queries.end())
		valReturn = !it->second.m_canceled && it->second.m_finished;
	m_mutex->unlock();

	return valReturn;
}

//--------------------------------------------------------------------------------------
bool MySQLDatabase::GetResultBoroughData(int _queryID, BoroughData& _data)
{
	bool valid = false;

	m_mutex->lock();
	auto it = m_queries.find(_queryID);
	if (it != m_queries.end())
	{
		if (it->second.m_finished)
		{
			_data = it->second.m_data;
			it->second.m_canceled = true;
			valid = true;
		}
	}
	m_mutex->unlock();
	return valid;
}

//------------------------------------------------------------------------------------------------
void MySQLDatabase::CancelQuery(const int _queryID)
{
	m_mutex->lock();
	auto it = m_queries.find(_queryID);
	if (it != m_queries.end())
		it->second.m_canceled = true;
	m_mutex->unlock();
}

//--------------------------------------------------------------------------------------
void MySQLDatabase::WriteBoroughData(BoroughData& _data)
{
	if (_data.IsValid())
		AddQuery(MySQLBoroughQuery::Type_Write, _data);
}

//--------------------------------------------------------------------------------------
bool MySQLDatabase::GetNextQuery(MySQLBoroughQuery& _request)
{
	m_mutex->lock();
	bool found = false;
	auto it = m_queries.begin();
	while (it != m_queries.end() && !found)
	{
		if (!it->second.m_canceled && !it->second.m_finished)
		{
			_request.m_queryID = it->first;
			_request.m_type = it->second.m_type;
			_request.m_data = it->second.m_data;
			found = true;
		}
		++it;
	}
	m_mutex->unlock();

	return found;
}

//------------------------------------------------------------------------------------------------
void MySQLDatabase::Validate(const int _queryID, BoroughData& _data)
{
	m_mutex->lock();
	auto it = m_queries.find(_queryID);
	if (it != m_queries.end())
	{
		it->second.m_data = _data;
		it->second.m_finished = true;
	}
	m_mutex->unlock();
}

//--------------------------------------------------------------------------------------
void MySQLDatabase::RemoveBoroughData(BoroughData& _data)
{
	char buf[4096];
	sprintf(buf, "DELETE FROM `BOROUGHS` WHERE CITY='%s' AND BOROUGH='%s'", _data.m_city.m_name.c_str(), _data.m_name.c_str());
	std::string str = buf;
	ExecuteUpdate(str);	
}

//--------------------------------------------------------------------------------------
MYSQL_RES* MySQLDatabase::ExecuteQuery(const std::string& _query) const
{
#ifdef MYSQL_ACTIVE
	NotifyMySQLEvent(_query);

	if (m_connexion)
	{
		if (mysql_query(m_connexion, _query.c_str()) == SQL_NO_ERROR)
		{
			MYSQL_RES* res = mysql_use_result(m_connexion);
			return res;
		}
	}
#endif

	return nullptr;
}

//--------------------------------------------------------------------------------------
int MySQLDatabase::ExecuteUpdate(const std::string& _query) const
{
#ifdef MYSQL_ACTIVE
	NotifyMySQLEvent(_query);

	if (m_connexion)
	{
		if (mysql_query(m_connexion, _query.c_str()) != SQL_NO_ERROR)
		{
			char buf[4096];
			sprintf(buf, "SQL ERROR sending update with request '%s'", _query.c_str());
			std::string message = buf;
			DisplayMySQLMessage(message);
			return 0;
		}
		return 1;
	}
#endif

	return 0;
}