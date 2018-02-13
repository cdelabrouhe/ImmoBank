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

	switch (_request->m_requestType)
	{
		case SearchRequestType_Announce:
		{
			SearchRequestAnnounce* announce = new SearchRequestAnnounce();
			_request->copyTo(announce);

			sInternalSearchRequest& request = m_requests[ID];
			request.m_request = announce;
			for (auto db : m_databases)
				request.m_internalRequests.push_back(std::make_pair(db, db->SendRequest(announce)));

		}
		break;
	}
	

	return ID;
}

//-------------------------------------------------------------------------------------------------
bool DatabaseManager::IsRequestAvailable(int _requestID) const
{
	auto it = m_requests.find(_requestID);
	if (it != m_requests.end())
		return it->second.IsAvailable();
	return false;
}

//-------------------------------------------------------------------------------------------------
void DatabaseManager::DeleteRequest(int _requestID)
{
	auto it = m_requests.find(_requestID);
	if (it != m_requests.end())
	{
		it->second.End();
		m_requests.erase(it);
	}
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

//-------------------------------------------------------------------------------------------------
bool DatabaseManager::GetRequestResult(const int _requestID, std::vector<SearchRequestResult*>& _result)
{
	if (!IsRequestAvailable(_requestID))
		return false;

	bool valid = true;

	for (auto& entry : m_requests)
	{
		sInternalSearchRequest& request = entry.second;
		if (request.IsAvailable())
			valid &= request.GetResult(_result);
	}

	if (valid)
		DeleteRequest(_requestID);

	return valid;
}

//-------------------------------------------------------------------------------------------------
bool sInternalSearchRequest::IsAvailable() const
{
	bool valid = true;

	for (auto& request : m_internalRequests)
	{
		OnlineDatabase* db = request.first;
		int requestID = request.second;
		valid &= db->IsRequestAvailable(requestID);
	}

	return valid;
}

//-------------------------------------------------------------------------------------------------
bool sInternalSearchRequest::GetResult(std::vector<SearchRequestResult*>& _results)
{
	if (!IsAvailable())
		return false;

	bool valid = true;
	for (auto& request : m_internalRequests)
	{
		OnlineDatabase* db = request.first;
		int requestID = request.second;
		valid &= db->GetRequestResult(requestID, _results);
		db->DeleteRequest(requestID);
	}

	delete m_request;
	m_request = nullptr;

	m_internalRequests.clear();

	return valid;
}

//-------------------------------------------------------------------------------------------------
void sInternalSearchRequest::End()
{
	for (auto& request : m_internalRequests)
	{
		OnlineDatabase* db = request.first;
		db->DeleteRequest(request.second);
	}
	m_internalRequests.clear();
	delete m_request;
}
