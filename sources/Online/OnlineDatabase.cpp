#include "OnlineDatabase.h"
#include "OnlineManager.h"
#include <Tools/StringTools.h>
#include <extern/jsoncpp/reader.h>

using namespace ImmoBank;

bool OnlineDatabase::IsRequestAvailable(int _requestID)
{
	auto it = m_requests.find(_requestID);
	if (it != m_requests.end())
		return OnlineManager::getSingleton()->IsHTTPRequestAvailable(it->second.m_requestID);

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
			bool result = _ProcessResult(request, str, _result);
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

void OnlineDatabase::Process()
{
	// Force update
	if (m_forceUpdateInProgress)
		_ProcessForceUpdate();
}

void OnlineDatabase::_ProcessForceUpdate()
{
	if (!m_forceUpdateInitialized)
	{
		m_forceUpdateInitialized = true;
		std::vector<BoroughData> data;
		DatabaseManager::getSingleton()->GetAllBoroughs(data);

		for (auto& borough : data)
		{
			std::string request = _ComputeKeyURL(borough.m_city.m_name);
			m_boroughData.push_back(sBoroughData(borough, request, -1));
		}
		return;
	}

	// Create requests when possible
	while (m_timer > m_intervalBetweenRequests)
	{
		m_timer -= m_intervalBetweenRequests;
		bool found = false;
		auto it = m_boroughData.begin();
		while (it != m_boroughData.end())
		{
			sBoroughData& boroughData = *it;
			if (boroughData.m_requestID == -1)
			{
				boroughData.m_requestID = OnlineManager::getSingleton()->SendBasicHTTPRequest(boroughData.m_request, true);
				++it;
				found = true;
				break;
			}
			else
				++it;
		}

		if (!found)
			break;
	}
	++m_timer;

	bool available = true;
	auto it = m_boroughData.begin();
	while (it != m_boroughData.end())
	{
		sBoroughData& borough = *it;
		if ((borough.m_requestID != -1) && OnlineManager::getSingleton()->IsHTTPRequestAvailable(borough.m_requestID))
		{
			std::string str;
			OnlineManager::getSingleton()->GetBasicHTTPRequestResult(borough.m_requestID, str);

			_DecodeData(str, borough);

			it = m_boroughData.erase(it);
			if (m_boroughData.size() == 0)
				m_forceUpdateInProgress = false;
		}
		else
		{
			available = false;
			++it;
		}
	}

	if (available)
	{
		m_forceUpdateInProgress = false;
		m_forceUpdateInitialized = false;
	}
}
//-------------------------------------------------------------------------------------------------
bool* OnlineDatabase::ForceUpdate()
{
	return &m_forceUpdateInProgress;
}