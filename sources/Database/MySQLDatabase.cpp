#include "MySQLDatabase.h"

#include "extern/jsoncpp/value.h"
#include "Tools/Tools.h"
#include "Tools/Thread/Thread.h"
#include <time.h>

#include "Tools/StringTools.h"
#include "DatabaseManager.h"
#include "Online/OnlineManager.h"
#include "extern/jsoncpp/reader.h"
#include <Config/ConfigManager.h>

using namespace ImmoBank;

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

#include "UI/UIManager.h"

void NotifyMySQLEvent(const std::string& _str)
{
	if (Tools::IsDevMode())
		DatabaseManager::getSingleton()->NotifyMySQLEvent(_str);
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
		if (MySQLQuery* query = db->GetNextQuery())
		{
			query->Process(db);
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
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
	const Json::Value* data = ConfigManager::getSingleton()->GetEntry("Database");
	if (data)
	{
		m_server = (*data)["Server"].asString();
		m_port = (*data)["Port"].asUInt();
		m_user = (*data)["User"].asString();
		m_password = (*data)["Password"].asString();
		m_base = (*data)["Base"].asString();
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

	char buf[4096];
	sprintf(buf, "Connecting to server %s:%d...", m_server.c_str(), m_port);
	std::string message = buf;
	DisplayMySQLMessage(message);

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

	if (m_connexion != nullptr)
	{
		memset(buf, 0, 4096);
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
		MySQLQuery* query = it->second;
		if (query->m_canceled)
		{
			delete query;
			it = m_queries.erase(it);
		}
		else
			++it;
	}
	m_mutex->unlock();

	// Check timeout queries
	auto curTime = time(0);
	auto itTimeout = m_timeoutQueries.begin();
	while (itTimeout != m_timeoutQueries.end())
	{
		time_t result = curTime - itTimeout->second;
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
	switch (m_IO)
	{
	case IO::Read:
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
			m_data.m_meilleursAgentsKey = strtoul(row[rowID++], nullptr, 10);
			m_data.m_priceBuyApartment.m_val = (float)strtod(row[rowID++], nullptr);
			m_data.m_priceBuyApartment.m_min = (float)strtod(row[rowID++], nullptr);
			m_data.m_priceBuyApartment.m_max = (float)strtod(row[rowID++], nullptr);
			m_data.m_priceBuyHouse.m_val = (float)strtod(row[rowID++], nullptr);
			m_data.m_priceBuyHouse.m_min = (float)strtod(row[rowID++], nullptr);
			m_data.m_priceBuyHouse.m_max = (float)strtod(row[rowID++], nullptr);
			m_data.m_priceRentHouse.m_val = (float)strtod(row[rowID++], nullptr);
			m_data.m_priceRentHouse.m_min = (float)strtod(row[rowID++], nullptr);
			m_data.m_priceRentHouse.m_max = (float)strtod(row[rowID++], nullptr);
			m_data.m_priceRentApartmentT1.m_val = (float)strtod(row[rowID++], nullptr);
			m_data.m_priceRentApartmentT1.m_min = (float)strtod(row[rowID++], nullptr);
			m_data.m_priceRentApartmentT1.m_max = (float)strtod(row[rowID++], nullptr);
			m_data.m_priceRentApartmentT2.m_val = (float)strtod(row[rowID++], nullptr);
			m_data.m_priceRentApartmentT2.m_min = (float)strtod(row[rowID++], nullptr);
			m_data.m_priceRentApartmentT2.m_max = (float)strtod(row[rowID++], nullptr);
			m_data.m_priceRentApartmentT3.m_val = (float)strtod(row[rowID++], nullptr);
			m_data.m_priceRentApartmentT3.m_min = (float)strtod(row[rowID++], nullptr);
			m_data.m_priceRentApartmentT3.m_max = (float)strtod(row[rowID++], nullptr);
			m_data.m_priceRentApartmentT4Plus.m_val = (float)strtod(row[rowID++], nullptr);
			m_data.m_priceRentApartmentT4Plus.m_min = (float)strtod(row[rowID++], nullptr);
			m_data.m_priceRentApartmentT4Plus.m_max = (float)strtod(row[rowID++], nullptr);
			m_data.m_selogerKey = strtoul(row[rowID++], nullptr, 10);
			if (const char* key = row[rowID++])	m_data.m_logicImmoKey = key;
			auto papKey = row[rowID++];
			m_data.m_papKey = papKey != nullptr ? strtoul(papKey, nullptr, 10) : 0;
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
	case IO::Write:
	{
		// Remove current borough data
		_db->RemoveBoroughData(m_data);

		// Insert new borough data
		char buf[4096];
		memset(buf, 0, 4096);
		sprintf(buf, "INSERT INTO BOROUGHS (CITY, BOROUGH, TIMEUPDATE, BOROUGHKEY, APARTMENTBUY, APARTMENTBUYMIN, APARTMENTBUYMAX, HOUSEBUY, HOUSEBUYMIN, HOUSEBUYMAX, RENTHOUSE, RENTHOUSEMIN, RENTHOUSEMAX, RENTT1, RENTT1MIN, RENTT1MAX, RENTT2, RENTT2MIN, RENTT2MAX, RENTT3, RENTT3MIN, RENTT3MAX, RENTT4, RENTT4MIN, RENTT4MAX, SELOGERKEY, LOGICIMMOKEY, PAPKEY) VALUES('%s', '%s', %u, %u, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %u, '%s', %u)",
			m_data.m_city.m_name.c_str(),
			m_data.m_name.c_str(),
			m_data.m_timeUpdate.GetData(),
			m_data.m_meilleursAgentsKey,
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
			m_data.m_priceRentApartmentT4Plus.m_max,
			m_data.m_selogerKey,
			m_data.m_logicImmoKey.empty() ? "(null)" : m_data.m_logicImmoKey.c_str(),
			m_data.m_papKey);

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

	_db->Validate(m_queryID);
#endif
}

//--------------------------------------------------------------------------------------
void MySQLBoroughListQuery::Process(MySQLDatabase* _db)
{
#ifdef MYSQL_ACTIVE
	switch (m_IO)
	{
	case IO::Read:
	{
		char buf[512];
		if (!m_city.m_name.empty())
			sprintf(buf, "SELECT * FROM BOROUGHS WHERE CITY='%s'", m_city.m_name.c_str());
		else
			sprintf(buf, "SELECT * FROM BOROUGHS");

		std::string str = buf;
		MYSQL_RES* result = _db->ExecuteQuery(str);

		while (MYSQL_ROW row = mysql_fetch_row(result))
		{
			BoroughData data;
			int rowID = 0;
			data.m_city.m_name = row[rowID++];
			data.m_name = row[rowID++];
			data.m_timeUpdate.SetData(strtoul(row[rowID++], nullptr, 10));
			data.m_meilleursAgentsKey = strtoul(row[rowID++], nullptr, 10);
			data.m_priceBuyApartment.m_val = (float)strtod(row[rowID++], nullptr);
			data.m_priceBuyApartment.m_min = (float)strtod(row[rowID++], nullptr);
			data.m_priceBuyApartment.m_max = (float)strtod(row[rowID++], nullptr);
			data.m_priceBuyHouse.m_val = (float)strtod(row[rowID++], nullptr);
			data.m_priceBuyHouse.m_min = (float)strtod(row[rowID++], nullptr);
			data.m_priceBuyHouse.m_max = (float)strtod(row[rowID++], nullptr);
			data.m_priceRentHouse.m_val = (float)strtod(row[rowID++], nullptr);
			data.m_priceRentHouse.m_min = (float)strtod(row[rowID++], nullptr);
			data.m_priceRentHouse.m_max = (float)strtod(row[rowID++], nullptr);
			data.m_priceRentApartmentT1.m_val = (float)strtod(row[rowID++], nullptr);
			data.m_priceRentApartmentT1.m_min = (float)strtod(row[rowID++], nullptr);
			data.m_priceRentApartmentT1.m_max = (float)strtod(row[rowID++], nullptr);
			data.m_priceRentApartmentT2.m_val = (float)strtod(row[rowID++], nullptr);
			data.m_priceRentApartmentT2.m_min = (float)strtod(row[rowID++], nullptr);
			data.m_priceRentApartmentT2.m_max = (float)strtod(row[rowID++], nullptr);
			data.m_priceRentApartmentT3.m_val = (float)strtod(row[rowID++], nullptr);
			data.m_priceRentApartmentT3.m_min = (float)strtod(row[rowID++], nullptr);
			data.m_priceRentApartmentT3.m_max = (float)strtod(row[rowID++], nullptr);
			data.m_priceRentApartmentT4Plus.m_val = (float)strtod(row[rowID++], nullptr);
			data.m_priceRentApartmentT4Plus.m_min = (float)strtod(row[rowID++], nullptr);
			data.m_priceRentApartmentT4Plus.m_max = (float)strtod(row[rowID++], nullptr);
			data.m_selogerKey = strtoul(row[rowID++], nullptr, 10);
			if (const char* key = row[rowID++])	data.m_logicImmoKey = key;
			auto papKey = row[rowID++];
			data.m_papKey = papKey != nullptr ? strtoul(papKey, nullptr, 10) : 0;

			m_list.push_back(data);
		}

		mysql_free_result(result);
	}
	break;
	case IO::Write:
	{
		// TODO (if relevant)
	}
	break;
	}

	_db->Validate(m_queryID);
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
MySQLQuery* MySQLDatabase::AddQuery(MySQLQuery::Type _type, MySQLQuery::IO _IO, unsigned int _key, int& _requestID)
{
#ifdef MYSQL_ACTIVE
	// Search for a recent similar request
	bool found = false;
	auto it = m_timeoutQueries.begin();
	while (!found && (it != m_timeoutQueries.end()))
	{
		auto& pair = *it;
		if (pair.first == _key)
			found = true;
		else
			++it;
	}

	// Manage timeout
	if (found)
	{
		if (_IO == MySQLQuery::IO::Read)
		{
			_requestID = -1;
			return nullptr;
		}
		else
			m_timeoutQueries.erase(it);
	}

	int requestID = GetNextAvailableRequestID();
	MySQLQuery* query = nullptr;
	switch (_type)
	{
	case MySQLQuery::Borough:
		query = new MySQLBoroughQuery(requestID, _IO);
		break;

	case MySQLQuery::BoroughList:
		query = new MySQLBoroughListQuery(requestID, _IO);
		break;
	}
	m_queries[requestID] = query;

	// Store in timeout list
	m_timeoutQueries.push_back(std::make_pair(_key, (u64)time(0)));

	_requestID = requestID;
	return query;
#else
	_requestID = -1;
	return nullptr;
#endif
}

//--------------------------------------------------------------------------------------
int MySQLDatabase::AskForBoroughData(BoroughData& _data)
{
	int requestID = -1;

	std::string str = _data.m_city.m_name + _data.m_name;
	unsigned int key = StringTools::GenerateHash(str);

	m_mutex->lock();

	MySQLQuery* query = AddQuery(MySQLQuery::Type::Borough, MySQLQuery::IO::Read, key, requestID);
	if (!query || (query->GetType() != MySQLQuery::Type::Borough))
	{
		m_mutex->unlock();
		return -1;
	}

	MySQLBoroughQuery* result = static_cast<MySQLBoroughQuery*>(query);
	result->m_data = _data;

	m_mutex->unlock();

	return requestID;
}

//--------------------------------------------------------------------------------------
int MySQLDatabase::AskForCityBoroughList(const sCity& _city)
{
	int requestID = -1;

	unsigned int key = StringTools::GenerateHash(_city.m_name);

	m_mutex->lock();

	MySQLQuery* query = AddQuery(MySQLQuery::Type::BoroughList, MySQLQuery::IO::Read, key, requestID);
	if (!query || (query->GetType() != MySQLQuery::Type::BoroughList))
	{
		m_mutex->unlock();
		return -1;
	}

	MySQLBoroughListQuery* result = static_cast<MySQLBoroughListQuery*>(query);
	result->m_city = _city;

	m_mutex->unlock();

	return requestID;
}

//--------------------------------------------------------------------------------------
bool MySQLDatabase::IsQueryAvailable(int _queryID) const
{
	bool valReturn = false;
	m_mutex->lock();
	auto it = m_queries.find(_queryID);
	if (it != m_queries.end())
		valReturn = !it->second->m_canceled && it->second->m_finished;
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
		MySQLQuery* query = it->second;
		if ((query->m_finished) && (query->GetType() == MySQLQuery::Type::Borough))
		{
			_data = ((MySQLBoroughQuery*)query)->m_data;
			query->m_canceled = true;
			valid = true;
		}
	}
	m_mutex->unlock();
	return valid;
}

//------------------------------------------------------------------------------------------------
bool MySQLDatabase::GetResultBoroughList(int _queryID, std::vector<BoroughData>& _list)
{
	bool valid = false;

	m_mutex->lock();
	auto it = m_queries.find(_queryID);
	if (it != m_queries.end())
	{
		MySQLQuery* query = it->second;
		if ((query->m_finished) && (query->GetType() == MySQLQuery::Type::BoroughList))
		{
			_list = ((MySQLBoroughListQuery*)query)->m_list;
			query->m_canceled = true;
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
		it->second->m_canceled = true;
	m_mutex->unlock();
}

//--------------------------------------------------------------------------------------
void MySQLDatabase::WriteBoroughData(BoroughData& _data)
{
	if (_data.IsValid())
	{
		int requestID = -1;
		std::string str = _data.m_city.m_name + _data.m_name;
		unsigned int key = StringTools::GenerateHash(str);

		m_mutex->lock();
		auto query = AddQuery(MySQLQuery::Type::Borough, MySQLQuery::IO::Write, key, requestID);
		MySQLBoroughQuery* result = static_cast<MySQLBoroughQuery*>(query);
		result->m_data = _data;
		m_mutex->unlock();
	}
}

//--------------------------------------------------------------------------------------
MySQLQuery* MySQLDatabase::GetNextQuery()
{
	MySQLQuery* result = nullptr;

	m_mutex->lock();
	bool found = false;
	auto it = m_queries.begin();
	while (it != m_queries.end() && !found)
	{
		MySQLQuery* query = it->second;
		if (!query->m_canceled && !query->m_finished)
		{
			result = query;
			found = true;
		}
		++it;
	}
	m_mutex->unlock();

	return result;
}

//------------------------------------------------------------------------------------------------
void MySQLDatabase::Validate(const int _queryID)
{
	m_mutex->lock();
	auto it = m_queries.find(_queryID);
	if (it != m_queries.end())
	{
		MySQLQuery* query = it->second;
		query->m_finished = true;
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
void MySQLDatabase::DebugQuery(const std::string& _query)
{
	MYSQL_RES* result = ExecuteQuery(_query);

	int cpt = 0;
	while (MYSQL_ROW row = mysql_fetch_row(result))
	{
		int rowID = 0;
		BoroughData data;
		data.m_city.m_name = row[rowID++];
		data.m_name = row[rowID++];
		data.m_timeUpdate.SetData(strtoul(row[rowID++], nullptr, 10));
		data.m_meilleursAgentsKey = strtoul(row[rowID++], nullptr, 10);
		data.m_priceBuyApartment.m_val = (float)strtod(row[rowID++], nullptr);
		data.m_priceBuyApartment.m_min = (float)strtod(row[rowID++], nullptr);
		data.m_priceBuyApartment.m_max = (float)strtod(row[rowID++], nullptr);
		data.m_priceBuyHouse.m_val = (float)strtod(row[rowID++], nullptr);
		data.m_priceBuyHouse.m_min = (float)strtod(row[rowID++], nullptr);
		data.m_priceBuyHouse.m_max = (float)strtod(row[rowID++], nullptr);
		data.m_priceRentHouse.m_val = (float)strtod(row[rowID++], nullptr);
		data.m_priceRentHouse.m_min = (float)strtod(row[rowID++], nullptr);
		data.m_priceRentHouse.m_max = (float)strtod(row[rowID++], nullptr);
		data.m_priceRentApartmentT1.m_val = (float)strtod(row[rowID++], nullptr);
		data.m_priceRentApartmentT1.m_min = (float)strtod(row[rowID++], nullptr);
		data.m_priceRentApartmentT1.m_max = (float)strtod(row[rowID++], nullptr);
		data.m_priceRentApartmentT2.m_val = (float)strtod(row[rowID++], nullptr);
		data.m_priceRentApartmentT2.m_min = (float)strtod(row[rowID++], nullptr);
		data.m_priceRentApartmentT2.m_max = (float)strtod(row[rowID++], nullptr);
		data.m_priceRentApartmentT3.m_val = (float)strtod(row[rowID++], nullptr);
		data.m_priceRentApartmentT3.m_min = (float)strtod(row[rowID++], nullptr);
		data.m_priceRentApartmentT3.m_max = (float)strtod(row[rowID++], nullptr);
		data.m_priceRentApartmentT4Plus.m_val = (float)strtod(row[rowID++], nullptr);
		data.m_priceRentApartmentT4Plus.m_min = (float)strtod(row[rowID++], nullptr);
		data.m_priceRentApartmentT4Plus.m_max = (float)strtod(row[rowID++], nullptr);
		data.m_selogerKey = strtoul(row[rowID++], nullptr, 10);
		if (const char* key = row[rowID++])	data.m_logicImmoKey = key;
		data.m_papKey = strtoul(row[rowID++], nullptr, 10);

		std::string mes = "City: " + data.m_city.m_name
			+ ", Borough: " + data.m_name
			+ ", MeilleursAgentsKEY: "
			+ std::to_string(data.m_meilleursAgentsKey)
			+ ", SeLogerKEY: "
			+ std::to_string(data.m_selogerKey)
			+ ", LogicImmoKEY: "
			+ data.m_logicImmoKey
			+ ", PapKEY: "
			+ std::to_string(data.m_papKey);

		DisplayMySQLMessage(mes);

		++cpt;
	}

	std::string mes = std::to_string(cpt) + " results";
	DisplayMySQLMessage(mes);

	mysql_free_result(result);
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

//--------------------------------------------------------------------------------------
bool MySQLDatabase::UpdateAllSeLogerKeys()
{
	bool result = true;
	if (Tools::IsDevMode())
	{
		if (!m_updateSelogerInProgress)
		{
			m_updateSelogerInProgress = true;
			std::string query = "SELECT * FROM BOROUGHS";
			MYSQL_RES* result = ExecuteQuery(query);

			while (MYSQL_ROW row = mysql_fetch_row(result))
			{
				int rowID = 0;
				BoroughData data;
				data.m_city.m_name = row[rowID++];
				data.m_name = row[rowID++];
				data.m_timeUpdate.SetData(strtoul(row[rowID++], nullptr, 10));
				data.m_meilleursAgentsKey = strtoul(row[rowID++], nullptr, 10);
				data.m_priceBuyApartment.m_val = (float)strtod(row[rowID++], nullptr);
				data.m_priceBuyApartment.m_min = (float)strtod(row[rowID++], nullptr);
				data.m_priceBuyApartment.m_max = (float)strtod(row[rowID++], nullptr);
				data.m_priceBuyHouse.m_val = (float)strtod(row[rowID++], nullptr);
				data.m_priceBuyHouse.m_min = (float)strtod(row[rowID++], nullptr);
				data.m_priceBuyHouse.m_max = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentHouse.m_val = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentHouse.m_min = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentHouse.m_max = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT1.m_val = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT1.m_min = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT1.m_max = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT2.m_val = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT2.m_min = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT2.m_max = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT3.m_val = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT3.m_min = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT3.m_max = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT4Plus.m_val = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT4Plus.m_min = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT4Plus.m_max = (float)strtod(row[rowID++], nullptr);
				data.m_selogerKey = strtoul(row[rowID++], nullptr, 10);
				if (const char* key = row[rowID++])	data.m_logicImmoKey = key;
				data.m_papKey = strtoul(row[rowID++], nullptr, 10);

				if (data.m_selogerKey != 0)
					continue;

				if (data.m_name == s_wholeCityName)
					continue;

				std::string request = data.ComputeSeLogerKeyURL();
				int requestID = OnlineManager::getSingleton()->SendBasicHTTPRequest(request);

				m_boroughData.push_back(sBoroughData(data, request, requestID));
			}

			mysql_free_result(result);
		}
		else
		{
			bool available = true;
			auto it = m_boroughData.begin();
			while (it != m_boroughData.end())
			{
				auto& borough = *it;
				if (OnlineManager::getSingleton()->IsHTTPRequestAvailable(borough.m_requestID))
				{
					std::string str;
					OnlineManager::getSingleton()->GetBasicHTTPRequestResult(borough.m_requestID, str);

					Json::Reader reader;
					Json::Value root;
					reader.parse(str, root);

					std::string boroughCityName = borough.m_data.m_city.m_name;
					Json::Value& places = root;
					if (places.isArray())
					{
						StringTools::TransformToLower(boroughCityName);

						const int nbPlaces = places.size();
						for (int placeID = 0; placeID < nbPlaces; ++placeID)
						{
							Json::Value val = places.get(placeID, Json::nullValue);
							std::string name = val["Display"].asString();
							std::string type = val["Type"].asString();
							bool isBorough = (type == "Quartier");
							bool isCity = (type == "Ville") && (str.find("e (") != std::string::npos) || (str.find("er (") != std::string::npos);

							bool valid = isBorough || isCity;
							if (!valid)
							{
								std::string mes = "Rejected because invalid for " + name;
								DisplayMySQLMessage(mes);
								continue;
							}

							std::string tmp = name;
							StringTools::TransformToLower(tmp);
							auto findID = tmp.find(boroughCityName);
							if (findID == std::string::npos)
								continue;

							std::string strIndexID = val["Params"][isBorough ? "idq" : "ci"].asString();
							unsigned int index = std::stoi(strIndexID);

							borough.m_data.SetSelogerKey(index, isCity);

							DatabaseManager::getSingleton()->AddBoroughData(borough.m_data);

							std::string mes = "Added borough " + borough.m_data.m_name + " to city " + borough.m_data.m_city.m_name;
							DisplayMySQLMessage(mes);
						}
					}
					else
					{
						std::string mes = "Rejected because no place in " + boroughCityName;
						DisplayMySQLMessage(mes);
					}

					it = m_boroughData.erase(it);
					if (m_boroughData.size() == 0)
						m_updateSelogerInProgress = false;
				}
				else
				{
					available = false;
					++it;
				}
			}

			if (available)
				result = false;
		}
	}
	return result;
}

std::map<std::string, std::string> s_logicImmoKeys;

bool ImmoBank::MySQLDatabase::UpdateAllLogicImmoKeys()
{
	bool result = true;
	if (Tools::IsDevMode())
	{
		if (!m_updateLogicImmoInProgress)
		{
			m_updateLogicImmoInProgress = true;
			std::string query = "SELECT * FROM BOROUGHS";
			MYSQL_RES* result = ExecuteQuery(query);
			int nbEntries = 0;

			while (MYSQL_ROW row = mysql_fetch_row(result))
			{
				++nbEntries;
				int rowID = 0;
				BoroughData data;
				data.m_city.m_name = row[rowID++];
				data.m_name = row[rowID++];
				data.m_timeUpdate.SetData(strtoul(row[rowID++], nullptr, 10));
				data.m_meilleursAgentsKey = strtoul(row[rowID++], nullptr, 10);
				data.m_priceBuyApartment.m_val = (float)strtod(row[rowID++], nullptr);
				data.m_priceBuyApartment.m_min = (float)strtod(row[rowID++], nullptr);
				data.m_priceBuyApartment.m_max = (float)strtod(row[rowID++], nullptr);
				data.m_priceBuyHouse.m_val = (float)strtod(row[rowID++], nullptr);
				data.m_priceBuyHouse.m_min = (float)strtod(row[rowID++], nullptr);
				data.m_priceBuyHouse.m_max = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentHouse.m_val = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentHouse.m_min = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentHouse.m_max = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT1.m_val = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT1.m_min = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT1.m_max = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT2.m_val = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT2.m_min = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT2.m_max = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT3.m_val = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT3.m_min = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT3.m_max = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT4Plus.m_val = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT4Plus.m_min = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT4Plus.m_max = (float)strtod(row[rowID++], nullptr);
				data.m_selogerKey = strtoul(row[rowID++], nullptr, 10);
				if (const char* key = row[rowID++])
				{
					data.m_logicImmoKey = key;
					if (!data.m_logicImmoKey.empty())
					{
						std::string cityName = data.m_city.m_name;
						StringTools::RemoveSpecialCharacters(cityName);
						StringTools::ReplaceBadSyntax(cityName, " ", "-");
						StringTools::TransformToLower(cityName);
						s_logicImmoKeys[cityName] = data.m_logicImmoKey;
						continue;
					}
				}
				data.m_papKey = strtoul(row[rowID++], nullptr, 10);

				std::string cityName = data.m_city.m_name;
				StringTools::RemoveSpecialCharacters(cityName);
				StringTools::ReplaceBadSyntax(cityName, " ", "-");
				StringTools::TransformToLower(cityName);
				if (s_logicImmoKeys.find(data.m_city.m_name) != s_logicImmoKeys.end())
				{
					data.m_logicImmoKey = s_logicImmoKeys[cityName];
					DatabaseManager::getSingleton()->AddBoroughData(data);
					continue;
				}

				std::string request = data.ComputeLogicImmoKeyURL();
				int requestID = OnlineManager::getSingleton()->SendBasicHTTPRequest(request, true);

				m_boroughData.push_back(sBoroughData(data, request, requestID));
			}

			mysql_free_result(result);
		}
		else
		{
			bool available = true;
			auto it = m_boroughData.begin();
			while (it != m_boroughData.end())
			{
				auto& borough = *it;
				if (OnlineManager::getSingleton()->IsHTTPRequestAvailable(borough.m_requestID))
				{
					std::string str;
					OnlineManager::getSingleton()->GetBasicHTTPRequestResult(borough.m_requestID, str);

					Json::Reader reader;
					Json::Value root;
					reader.parse(str, root);

					std::string boroughCityName = borough.m_data.m_city.m_name;
					Json::Value& places = root["items"];
					if (places.isArray())
					{
						StringTools::RemoveSpecialCharacters(boroughCityName);
						StringTools::TransformToLower(boroughCityName);
						StringTools::ReplaceBadSyntax(boroughCityName, " ", "-");

						const int nbPlaces = places.size();
						for (int placeID = 0; placeID < nbPlaces; ++placeID)
						{
							Json::Value val = places.get(placeID, Json::nullValue);
							std::string name = val["name"].asString();
							StringTools::TransformToLower(name);
							StringTools::ReplaceBadSyntax(name, " ", "-");
							if (name != boroughCityName)
								continue;

							std::string key = val["key"].asString();
							std::string zipCode = val["postCode"].asString();
							int zip = borough.m_data.m_city.m_zipCode;
							if (!zipCode.empty())
							{
								zip = stoi(zipCode);
								if ((borough.m_data.m_city.m_zipCode != 0) && (zip != borough.m_data.m_city.m_zipCode))
									continue;
							}

							borough.m_data.m_city.m_zipCode = zip;
							borough.m_data.SetLogicImmoKey(key);

							std::string cityName = borough.m_data.m_city.m_name;
							StringTools::RemoveSpecialCharacters(cityName);
							StringTools::ReplaceBadSyntax(cityName, " ", "-");
							StringTools::TransformToLower(cityName);
							s_logicImmoKeys[cityName] = key;

							DatabaseManager::getSingleton()->AddBoroughData(borough.m_data);

							std::string mes = "Added borough " + borough.m_data.m_name + " to city " + borough.m_data.m_city.m_name;
							DisplayMySQLMessage(mes);
						}
					}
					else
					{
						std::string mes = "Rejected because no place in " + boroughCityName;
						DisplayMySQLMessage(mes);
					}

					it = m_boroughData.erase(it);
					if (m_boroughData.size() == 0)
						m_updateLogicImmoInProgress = false;
				}
				else
				{
					available = false;
					++it;
				}
			}

			if (available)
				result = false;
		}
	}
	return result;
}

std::map<std::string, unsigned int> s_papKeys;

bool ImmoBank::MySQLDatabase::UpdateAllPapKeys()
{
	bool result = true;
	if (Tools::IsDevMode())
	{
		std::map<std::string, int> cities;
		if (!m_updatePapInProgress)
		{
			m_updatePapInProgress = true;
			std::string query = "SELECT * FROM BOROUGHS";
			MYSQL_RES* result = ExecuteQuery(query);
			int nbEntries = 0;

			while (MYSQL_ROW row = mysql_fetch_row(result))
			{
				++nbEntries;
				int rowID = 0;
				BoroughData data;
				data.m_city.m_name = row[rowID++];
				data.m_name = row[rowID++];
				data.m_timeUpdate.SetData(strtoul(row[rowID++], nullptr, 10));
				data.m_meilleursAgentsKey = strtoul(row[rowID++], nullptr, 10);
				data.m_priceBuyApartment.m_val = (float)strtod(row[rowID++], nullptr);
				data.m_priceBuyApartment.m_min = (float)strtod(row[rowID++], nullptr);
				data.m_priceBuyApartment.m_max = (float)strtod(row[rowID++], nullptr);
				data.m_priceBuyHouse.m_val = (float)strtod(row[rowID++], nullptr);
				data.m_priceBuyHouse.m_min = (float)strtod(row[rowID++], nullptr);
				data.m_priceBuyHouse.m_max = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentHouse.m_val = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentHouse.m_min = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentHouse.m_max = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT1.m_val = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT1.m_min = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT1.m_max = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT2.m_val = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT2.m_min = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT2.m_max = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT3.m_val = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT3.m_min = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT3.m_max = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT4Plus.m_val = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT4Plus.m_min = (float)strtod(row[rowID++], nullptr);
				data.m_priceRentApartmentT4Plus.m_max = (float)strtod(row[rowID++], nullptr);
				data.m_selogerKey = strtoul(row[rowID++], nullptr, 10);
				if (const char* key = row[rowID++])	data.m_logicImmoKey = key;

				auto papKey = row[rowID++];
				data.m_papKey = papKey != nullptr ? strtoul(papKey, nullptr, 10) : 0;
				if (data.m_papKey > 0)
				{
					std::string cityName = data.m_city.m_name;
					StringTools::RemoveSpecialCharacters(cityName);
					StringTools::ReplaceBadSyntax(cityName, " ", "-");
					StringTools::TransformToLower(cityName);
					s_papKeys[cityName] = data.m_papKey;
					continue;
				}

				if ((data.m_papKey != 0xFFFFFFFF) && (data.m_papKey != 0))
					continue;

				std::string cityName = data.m_city.m_name;
				StringTools::RemoveSpecialCharacters(cityName);
				StringTools::ReplaceBadSyntax(cityName, " ", "-");
				StringTools::TransformToLower(cityName);
				if (s_papKeys.find(data.m_city.m_name) != s_papKeys.end())
				{
					data.m_papKey = s_papKeys[cityName];
					DatabaseManager::getSingleton()->AddBoroughData(data);
					continue;
				}

				std::string request = data.ComputePapKeyURL();
				int requestID = OnlineManager::getSingleton()->SendBasicHTTPRequest(request, true);

				m_boroughData.push_back(sBoroughData(data, request, requestID));
			}

			mysql_free_result(result);
		}
		else
		{
			bool available = true;
			auto it = m_boroughData.begin();
			while (it != m_boroughData.end())
			{
				auto& borough = *it;
				if (OnlineManager::getSingleton()->IsHTTPRequestAvailable(borough.m_requestID))
				{
					std::string str;
					OnlineManager::getSingleton()->GetBasicHTTPRequestResult(borough.m_requestID, str);

					Json::Reader reader;
					Json::Value root;
					reader.parse(str, root);
					
					Json::Value& places = root["_embedded"]["place"];
					if (places.isArray() && (places.size() > 0))
					{
						std::string cityName = borough.m_data.m_city.m_name;
						StringTools::RemoveSpecialCharacters(cityName);
						StringTools::ReplaceBadSyntax(cityName, " ", "-");
						StringTools::TransformToLower(cityName);

						Json::Value val = places.get(0u, Json::nullValue);
						if (!val["id"].isNull())
						{
							int key = 0;
							auto id = val["id"];
							if (id.isString())
							{
								std::string idStr = val["id"].asString();
								key = !idStr.empty() ? std::stoi(idStr) : -1;
							}
							else if (id.isInt() || id.isUInt())
							{
								key = id.asInt();
							}

							if (key != -1)
							{
								borough.m_data.SetPapKey(key);
								s_papKeys[cityName] = key;
							}
						}

						DatabaseManager::getSingleton()->AddBoroughData(borough.m_data);

						std::string mes = "Added borough " + borough.m_data.m_name + " to city " + borough.m_data.m_city.m_name;
						DisplayMySQLMessage(mes);
					}
					else
					{
						std::string mes = "Rejected because no place in " + root["_embedded"]["place"]["slug"].asString();
						DisplayMySQLMessage(mes);
					}

					it = m_boroughData.erase(it);
					if (m_boroughData.size() == 0)
						m_updatePapInProgress = false;
				}
				else
				{
					available = false;
					++it;
				}
			}

			if (available)
				result = false;
		}
	}
	return result;
}

bool ImmoBank::MySQLDatabase::UpdateLocalBaseToServer()
{
	bool result = false;

	std::vector<BoroughData> data;
	DatabaseManager::getSingleton()->GetAllBoroughs(data);

	for (auto& entry : data)
		WriteBoroughData(entry);

	return result;
}

bool ImmoBank::MySQLDatabase::UpdateServerToLocalBase()
{
	sCity city;
	static int queryID = -1;
	if (queryID == -1)
	{
		queryID = AskForCityBoroughList(city);
	}
	else if (IsQueryAvailable(queryID))
	{
		std::vector<BoroughData> list;
		GetResultBoroughList(queryID, list);

		for (auto& entry : list)
			DatabaseManager::getSingleton()->AddBoroughData(entry, false);

		return false;
	}

	return true;
}
