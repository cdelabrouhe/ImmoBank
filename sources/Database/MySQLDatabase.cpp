#include "MySQLDatabase.h"

#include <../../jsoncpp/value.h>
#include "Tools/Tools.h"
#include "Tools/Thread/Thread.h"
#include <time.h>

#include "extern/jsoncpp/value.h"

#ifndef _DEBUG
#ifndef WIN32
#define MYSQL_ACTIVE
#endif
#endif

#ifdef MYSQL_ACTIVE
#include <mysql_connection.h>

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#endif

static const u64 s_timeoutQuery = 10;

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

//--------------------------------------------------------------------------------------
static void DisplayMySQLException(sql::SQLException& _e)
{
	std::string str = "WARNING: SQLException ! MySQL error code: " + std::to_string(_e.getErrorCode()) + ", SQLState: " + _e.getSQLState() + "\n";
	printf(str.c_str());
	NotifyMySQLEvent(str);
}

//--------------------------------------------------------------------------------------
static void DisplayMySQLRuntimeError(std::runtime_error& _e)
{
	std::string str = "WARNING: MySQL runtime_err: " + std::string(_e.what()) + "\n";
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
			query.Process(db);

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

	try {
		m_driver = get_driver_instance();
		const char* name = m_driver->getName().c_str();
		auto major = m_driver->getMajorVersion();
		auto minor = m_driver->getMinorVersion();
		auto patch = m_driver->getPatchVersion();

		m_connexion = m_driver->connect(m_server, m_user, m_password);
		// Connect to the MySQL test database
		m_connexion->setSchema(m_base);

		char buf[4096];
		sprintf(buf, "User '%s' connected to database '%s' on server '%s'", m_user.c_str(), m_base.c_str(), m_server.c_str());
		std::string message = buf;
		DisplayMySQLMessage(message);

		result = true;
	}
	catch (sql::SQLException &e)
	{
		DisplayMySQLException(e);
		return false;
	}
	catch (std::runtime_error &e)
	{
		DisplayMySQLRuntimeError(e);
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
		sql::Statement* stmt = nullptr;
		sql::ResultSet* result = _db->ExecuteQuery(str, stmt);

		if (result && result->next())
		{
			m_data.m_city.m_name = result->getString("CITY");
			m_data.m_name = result->getString("BOROUGH");
			m_data.m_timeUpdate.SetData(result->getUInt("TIMEUPDATE"));
			m_data.m_key = result->getUInt("BOROUGHKEY");
			m_data.m_priceBuyApartment.m_val = (float)result->getDouble("APARTMENTBUY");
			m_data.m_priceBuyApartment.m_min = (float)result->getDouble("APARTMENTBUYMIN");
			m_data.m_priceBuyApartment.m_max = (float)result->getDouble("APARTMENTBUYMAX");
			m_data.m_priceBuyHouse.m_val = (float)result->getDouble("HOUSEBUY");
			m_data.m_priceBuyHouse.m_min = (float)result->getDouble("HOUSEBUYMIN");
			m_data.m_priceBuyHouse.m_max = (float)result->getDouble("HOUSEBUYMAX");
			m_data.m_priceRentHouse.m_val = (float)result->getDouble("RENTHOUSE");
			m_data.m_priceRentHouse.m_min = (float)result->getDouble("RENTHOUSEMIN");
			m_data.m_priceRentHouse.m_max = (float)result->getDouble("RENTHOUSEMAX");
			m_data.m_priceRentApartmentT1.m_val = (float)result->getDouble("RENTT1");
			m_data.m_priceRentApartmentT1.m_min = (float)result->getDouble("RENTT1MIN");
			m_data.m_priceRentApartmentT1.m_max = (float)result->getDouble("RENTT1MAX");
			m_data.m_priceRentApartmentT2.m_val = (float)result->getDouble("RENTT2");
			m_data.m_priceRentApartmentT2.m_min = (float)result->getDouble("RENTT2MIN");
			m_data.m_priceRentApartmentT2.m_max = (float)result->getDouble("RENTT2MAX");
			m_data.m_priceRentApartmentT3.m_val = (float)result->getDouble("RENTT3");
			m_data.m_priceRentApartmentT3.m_min = (float)result->getDouble("RENTT3MIN");
			m_data.m_priceRentApartmentT3.m_max = (float)result->getDouble("RENTT3MAX");
			m_data.m_priceRentApartmentT4Plus.m_val = (float)result->getDouble("RENTT4");
			m_data.m_priceRentApartmentT4Plus.m_min = (float)result->getDouble("RENTT4MIN");
			m_data.m_priceRentApartmentT4Plus.m_max = (float)result->getDouble("RENTT4MAX");
}

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
		m_connexion->close();
		delete m_connexion;
		m_connexion = NULL;

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

	if (it != m_timeoutQueries.end())
		return -1;

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
	sprintf(buf, "DELETE FROM `BOROUGHS` WHERE BOROUGHKEY=%u", _data.m_key);
	std::string str = buf;
	sql::Statement* stmt = nullptr;
	ExecuteQuery(str, stmt);
}

//--------------------------------------------------------------------------------------
sql::ResultSet* MySQLDatabase::ExecuteQuery(const std::string& _query, sql::Statement* _statement) const
{
#ifdef MYSQL_ACTIVE
	NotifyMySQLEvent(_query);

	if (m_connexion)
	{
		try {
			/* create a statement object */
			_statement = m_connexion->createStatement();

			/* run a query which returns exactly one result set */
			return _statement->executeQuery(_query);
		}
		catch (sql::SQLException &e)
		{
			DisplayMySQLException(e);
			return NULL;
		}
		catch (std::runtime_error &e)
		{
			DisplayMySQLRuntimeError(e);
			return NULL;
		}
	}
#endif

	return NULL;
}

//--------------------------------------------------------------------------------------
int MySQLDatabase::ExecuteUpdate(const std::string& _query) const
{
#ifdef MYSQL_ACTIVE
	NotifyMySQLEvent(_query);

	if (m_connexion)
	{
		try {
			/* create a statement object */
			sql::Statement* stmt = m_connexion->createStatement();

			/* run a query which returns exactly one result set */
			return stmt->executeUpdate(_query);
		}
		catch (sql::SQLException &e)
		{
			DisplayMySQLException(e);
			return 0;
		}
		catch (std::runtime_error &e)
		{
			DisplayMySQLRuntimeError(e);
			return 0;
		}
	}
#endif

	return 0;
}

ENABLE_OPTIMIZE