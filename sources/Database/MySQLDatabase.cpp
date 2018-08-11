#include "MySQLDatabase.h"

#include <../../jsoncpp/value.h>
#include "Tools/Tools.h"
#include "Tools/Thread/Thread.h"
#include <time.h>

#ifndef _DEBUG
#define MYSQL_ACTIVE
#endif

//#define XDEVAPI

#ifndef XDEVAPI
#define MYSQL_CONNECTION
#endif

#ifdef XDEVAPI
#include <mysqlx/xdevapi.h>
using namespace mysqlx;
#endif

#ifdef MYSQL_CONNECTION
#include <jdbc/mysql_connection.h>

#include <jdbc/cppconn/driver.h>
#include <jdbc/cppconn/exception.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>

#pragma comment(lib, "mysqlcppconn.lib")
#pragma comment(lib, "mysqlcppconn8.lib")
#endif

static const u64 s_timeoutQuery = 10;

#ifdef MYSQL_ACTIVE
//--------------------------------------------------------------------------------------
static void DisplayMySQLException(sql::SQLException& _e)
{
	std::string str = "WARNING: SQLException ! MySQL error code: " + std::to_string(_e.getErrorCode()) + ", SQLState: " + _e.getSQLState() + "\n";
	printf(str.c_str());
}

//--------------------------------------------------------------------------------------
static void DisplayMySQLRuntimeError(std::runtime_error& _e)
{
	std::string str = "WARNING: MySQL runtime_err: " + std::string(_e.what()) + "\n";
	printf(str.c_str());
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

//--------------------------------------------------------------------------------------
void MySQLBoroughQuery::Process(MySQLDatabase* _db)
{
	switch (m_type)
	{
	case Type_Read:
		{
			char buf[512];
			sprintf(buf, "SELECT * FROM BOROUGHS WHERE CITY='%s' AND BOROUGH='%s'", m_data.m_city.m_name.c_str(), m_data.m_name.c_str());
			std::string str = buf;
			sql::Statement* stmt = nullptr;
			sql::ResultSet* result = _db->ExecuteQuery(str, stmt);

			if (result->next())
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
			char buf[4096];
			sprintf(buf, "DELETE FROM `BOROUGHS` WHERE BOROUGHKEY=%u", m_data.m_key);
			std::string str = buf;
			sql::Statement* stmt = nullptr;
			_db->ExecuteQuery(str, stmt);
			
			// Insert new borough data
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

			str = buf;
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
}

//--------------------------------------------------------------------------------------
void MySQLDatabase::Init()
{
#ifdef MYSQL_ACTIVE
	m_mutex = new std::mutex();

	// Init thread
	m_thread = new Thread();
	m_thread->start(MySQLThreadStart, this, "MySQLRequestsThread");

#ifdef MYSQL_CONNECTION
	try {
		m_driver = get_driver_instance();
		const char* name = m_driver->getName().c_str();
		auto major = m_driver->getMajorVersion();
		auto minor = m_driver->getMinorVersion();
		auto patch = m_driver->getPatchVersion();

		m_connexion = m_driver->connect("192.168.0.26:3307", "basicuser", "chouchou");
		// Connect to the MySQL test database
		m_connexion->setSchema("mysql");
	}
	catch (sql::SQLException &e)
	{
		DisplayMySQLException(e);
		return;
	}
	catch (std::runtime_error &e)
	{
		DisplayMySQLRuntimeError(e);
		return;
	}
#endif

#ifdef XDEVAPI
	//----------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------------
	// EXAMPLE IN HERE: https://dev.mysql.com/doc/x-devapi-userguide/en/database-connection-example.html
	//----------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------------
	// Scope controls life-time of objects such as session or schema
	{
		Session sess("192.168.0.26", 3307, "basicuser", "chouchou");
		Schema db = sess.getSchema("mysql");
		// or Schema db(sess, "test");

		Collection myColl = db.getCollection("CITIES");
		// or Collection myColl(db, "my_collection");

		DocResult myDocs = myColl.find("name like :param")
			.limit(1)
			.bind("param", "S%").execute();

		Json::Value str = myDocs.fetchOne();
		printf("");
	}
#endif
#endif
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
		u64 result = curTime - itTimeout->second;
		if (result > s_timeoutQuery)
			itTimeout = m_timeoutQueries.erase(itTimeout);
		else
			++itTimeout;
	}
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
	auto it = std::find_if(m_timeoutQueries.begin(), m_timeoutQueries.end(), [key](std::pair<unsigned int, u64>& _pair)->bool
	{
		return (_pair.first == key);
	});
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
sql::ResultSet* MySQLDatabase::ExecuteQuery(const std::string& _query, sql::Statement* _statement) const
{
#ifdef MYSQL_ACTIVE
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