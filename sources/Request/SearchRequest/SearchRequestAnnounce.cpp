#include "SearchRequestAnnounce.h"
#include "Online\OnlineManager.h"
#include "Online\OnlineDatabase.h"
#include "SearchRequestResulCityBorough.h"
#include "SearchRequestCityBoroughs.h"

//---------------------------------------------------------------------------------------------------------------------------------
void SearchRequestAnnounce::Init()
{
	SearchRequestCityBoroughs boroughs;
	boroughs.m_city = m_city;
	m_boroughsRequestID = OnlineManager::getSingleton()->SendRequest(&boroughs);
}

//---------------------------------------------------------------------------------------------------------------------------------
void SearchRequestAnnounce::Process()
{
	if ((m_boroughsRequestID > -1) && OnlineManager::getSingleton()->IsRequestAvailable(m_boroughsRequestID))
	{
		std::vector<SearchRequestResult*> list;
		OnlineManager::getSingleton()->GetRequestResult(m_boroughsRequestID, list);
		m_boroughsRequestID = -1;

		for (auto result : list)
		{
			if (result->m_resultType == SearchRequestType_CityBoroughs)
			{
				SearchRequestResulCityBorough* borough = static_cast<SearchRequestResulCityBorough*>(result);
				m_boroughs.push_back(borough->m_name);
				delete borough;
			}
		}

		// Trigger internal requests
		auto databases = OnlineManager::getSingleton()->GetOnlineDatabases();
		for (auto db : databases)
			m_internalRequests.push_back(std::make_pair(db, db->SendRequest(this)));
	}
}

//---------------------------------------------------------------------------------------------------------------------------------
void SearchRequestAnnounce::End()
{
	for (auto& request : m_internalRequests)
	{
		OnlineDatabase* db = request.first;
		db->DeleteRequest(request.second);
	}
	m_internalRequests.clear();
}

//---------------------------------------------------------------------------------------------------------------------------------
void SearchRequestAnnounce::copyTo(SearchRequest* _target)
{
	SearchRequest::copyTo(_target);
	if (_target->m_requestType != SearchRequestType_Announce)
		return;

	SearchRequestAnnounce* target = (SearchRequestAnnounce*)_target;
	target->m_city = m_city;
	target->m_type = m_type;
	target->m_categories = m_categories;
	target->m_priceMin = m_priceMin;
	target->m_priceMax = m_priceMax;
	target->m_surfaceMin = m_surfaceMin;
	target->m_surfaceMax = m_surfaceMax;
	target->m_nbRooms = m_nbRooms;
	target->m_nbBedRooms = m_nbBedRooms;
}

//---------------------------------------------------------------------------------------------------------------------------------
bool SearchRequestAnnounce::IsAvailable() const
{
	if (m_boroughsRequestID > -1)
		return false;

	bool valid = true;
	for (auto& request : m_internalRequests)
	{
		OnlineDatabase* db = request.first;
		int requestID = request.second;
		valid &= db->IsRequestAvailable(requestID);
	}
	return valid;
}

//---------------------------------------------------------------------------------------------------------------------------------
bool SearchRequestAnnounce::GetResult(std::vector<SearchRequestResult*>& _results)
{
	if (!IsAvailable())
		return false;

	if (m_boroughsRequestID > -1)
		return false;

	bool valid = true;
	for (auto& request : m_internalRequests)
	{
		OnlineDatabase* db = request.first;
		int requestID = request.second;
		valid &= db->GetRequestResult(requestID, _results);
		db->DeleteRequest(requestID);
	}

	m_internalRequests.clear();

	return valid;
}