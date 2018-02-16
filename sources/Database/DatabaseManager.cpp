#include "DatabaseManager.h"
#include "HTTPDownloader.h"
#include "Request/SearchRequest.h"
#include "Tools/StringTools.h"
#include "SeLogerOnlineDatabase.h"
#include "LeSiteImmoOnlineDatabase.h"

DatabaseManager* s_singleton = nullptr;

//-------------------------------------------------------------------------------------------------
DatabaseManager* DatabaseManager::getSingleton()
{
	if (s_singleton == nullptr)
		s_singleton = new DatabaseManager();
	return s_singleton;
}

//-------------------------------------------------------------------------------------------------
// DATA
//-------------------------------------------------------------------------------------------------
HTTPDownloader	s_downloader;

//-------------------------------------------------------------------------------------------------
// FUNCTIONS
//-------------------------------------------------------------------------------------------------
void DatabaseManager::Init()
{
	s_downloader.Init();

	auto seLogerDB = new SeLogerOnlineDatase();
	seLogerDB->Init(&s_downloader);
	m_databases.push_back(seLogerDB);

	auto leSiteImmoDB = new LeSiteImmoOnlineDatabase();
	leSiteImmoDB->Init(&s_downloader);
	m_databases.push_back(leSiteImmoDB);
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::Process()
{
	s_downloader.Process();

	for (auto db : m_databases)
		db->Process();

	for (auto& request : m_requests)
		request.second->Process();
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::End()
{
	s_downloader.End();

	for (auto db : m_databases)
		db->End();
}

//-------------------------------------------------------------------------------------------------
int DatabaseManager::SendRequest(SearchRequest* _request)
{
	int ID = 0;
	while (m_requests.find(ID) != m_requests.end())
		++ID;

	SearchRequest* request = nullptr;
	switch (_request->m_requestType)
	{
		case SearchRequestType_Announce:
			request = new SearchRequestAnnounce();
			break;


		case SearchRequestType_CityBoroughs:
			request = new SearchRequestCityBoroughs();
			break;
	}
	
	if (request)
	{
		m_requests[ID] = request;
		_request->copyTo(request);
		request->Init();
	}
		
	return ID;
}

//-------------------------------------------------------------------------------------------------
bool DatabaseManager::IsRequestAvailable(int _requestID) const
{
	auto it = m_requests.find(_requestID);
	if (it != m_requests.end())
		return it->second->IsAvailable();
	return false;
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::DeleteRequest(int _requestID)
{
	auto it = m_requests.find(_requestID);
	if (it != m_requests.end())
	{
		SearchRequest* request = it->second;
		request->End();
		delete request;
		m_requests.erase(it);
	}
}

//-------------------------------------------------------------------------------------------------
bool DatabaseManager::GetRequestResult(const int _requestID, std::vector<SearchRequestResult*>& _result)
{
	if (!IsRequestAvailable(_requestID))
		return false;

	bool valid = true;

	for (auto& entry : m_requests)
	{
		SearchRequest* request = entry.second;
		if (request->IsAvailable())
			valid &= request->GetResult(_result);
	}

	if (valid)
		DeleteRequest(_requestID);

	return valid;
}

//-------------------------------------------------------------------------------------------------
int DatabaseManager::SendBasicHTTPRequest(const std::string& _request)
{
	return s_downloader.SendRequest(_request);
}

//-------------------------------------------------------------------------------------------------
bool DatabaseManager::IsBasicHTTPRequestAvailable(int _requestID) const
{
	return s_downloader.IsRequestAvailable(_requestID);
}

//-------------------------------------------------------------------------------------------------
bool DatabaseManager::GetBasicHTTPRequestResult(const int _requestID, std::string& _result)
{
	return s_downloader.GetResult(_requestID, _result);
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::CancelBasicHTTPRequest(const int _requestID)
{
	s_downloader.CancelRequest(_requestID);
}