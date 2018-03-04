#include "OnlineManager.h"
#include "HTTPDownloader.h"
#include "Request/SearchRequest.h"
#include "Tools/StringTools.h"
#include "SeLogerOnlineDatabase.h"
#include "LeSiteImmoOnlineDatabase.h"

OnlineManager* s_singleton = nullptr;

//-------------------------------------------------------------------------------------------------
OnlineManager* OnlineManager::getSingleton()
{
	if (s_singleton == nullptr)
		s_singleton = new OnlineManager();
	return s_singleton;
}

//-------------------------------------------------------------------------------------------------
// DATA
//-------------------------------------------------------------------------------------------------
HTTPDownloader	s_downloader;

//-------------------------------------------------------------------------------------------------
// FUNCTIONS
//-------------------------------------------------------------------------------------------------
void OnlineManager::Init()
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
void OnlineManager::Process()
{
	s_downloader.Process();

	for (auto db : m_databases)
		db->Process();

	for (auto& request : m_requests)
		request.second->Process();
}

//-------------------------------------------------------------------------------------------------
void OnlineManager::End()
{
	s_downloader.End();

	for (auto db : m_databases)
		db->End();
}

//-------------------------------------------------------------------------------------------------
int OnlineManager::SendRequest(SearchRequest* _request)
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
bool OnlineManager::IsRequestAvailable(int _requestID) const
{
	auto it = m_requests.find(_requestID);
	if (it != m_requests.end())
		return it->second->IsAvailable();
	return false;
}

//-------------------------------------------------------------------------------------------------
void OnlineManager::DeleteRequest(int _requestID)
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
bool OnlineManager::GetRequestResult(const int _requestID, std::vector<SearchRequestResult*>& _result)
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
int OnlineManager::SendBasicHTTPRequest(const std::string& _request)
{
	return s_downloader.SendRequest(_request);
}

//-------------------------------------------------------------------------------------------------
bool OnlineManager::IsBasicHTTPRequestAvailable(int _requestID) const
{
	return s_downloader.IsRequestAvailable(_requestID);
}

//-------------------------------------------------------------------------------------------------
bool OnlineManager::GetBasicHTTPRequestResult(const int _requestID, std::string& _result)
{
	return s_downloader.GetResult(_requestID, _result);
}

//-------------------------------------------------------------------------------------------------
void OnlineManager::CancelBasicHTTPRequest(const int _requestID)
{
	s_downloader.CancelRequest(_requestID);
}