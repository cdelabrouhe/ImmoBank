#include "OnlineDatabase.h"
#include "OnlineManager.h"

using namespace ImmoBank;

bool OnlineDatabase::IsRequestAvailable(int _requestID)
{
	auto it = m_requests.find(_requestID);
	if (it != m_requests.end())
		return OnlineManager::getSingleton()->IsBasicHTTPRequestAvailable(it->second.m_requestID);

	return false;
}

bool OnlineDatabase::GetRequestResult(int _requestID, std::vector<SearchRequestResult*>& _result)
{
	auto it = m_requests.find(_requestID);
	if (it != m_requests.end())
	{
		std::string str;
		if (OnlineManager::getSingleton()->GetBasicHTTPRequestResult(it->second.m_requestID, str))
		{
			SearchRequest* request = it->second.m_initialRequest;
			bool result = ProcessResult(request, str, _result);
			DeleteRequest(_requestID);
			return result;
		}
	}
	return false;
}

void OnlineDatabase::DeleteRequest(int _requestID)
{
	auto it = m_requests.find(_requestID);
	if (it != m_requests.end())
		m_requests.erase(it);
}