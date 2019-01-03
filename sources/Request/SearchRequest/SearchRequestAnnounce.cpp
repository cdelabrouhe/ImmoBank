#include "SearchRequestAnnounce.h"
#include "Online\OnlineManager.h"
#include "Online\OnlineDatabase.h"
#include "SearchRequestResulCityBorough.h"
#include "SearchRequestCityBoroughs.h"
#include <algorithm>

using namespace ImmoBank;

//---------------------------------------------------------------------------------------------------------------------------------
void SearchRequestAnnounce::Init()
{
	DatabaseManager::getSingleton()->GetBoroughs(m_city, m_boroughs);

	// Trigger internal requests
	std::vector<OnlineDatabase*>& databases = OnlineManager::getSingleton()->GetOnlineDatabases();
	for (auto db : databases)
		m_internalRequests.push_back(std::make_pair(db, db->SendRequest(this)));
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
	target->m_boroughList = m_boroughList;
	target->m_categories = m_categories;
	target->m_priceMin = m_priceMin;
	target->m_priceMax = m_priceMax;
	target->m_surfaceMin = m_surfaceMin;
	target->m_surfaceMax = m_surfaceMax;
	target->m_nbRoomsMin = m_nbRoomsMin;
	target->m_nbRoomsMax = m_nbRoomsMax;
	target->m_nbBedRoomsMin = m_nbBedRoomsMin;
	target->m_nbBedRoomsMax = m_nbBedRoomsMax;
}

//---------------------------------------------------------------------------------------------------------------------------------
bool SearchRequestAnnounce::IsAvailable() const
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

//---------------------------------------------------------------------------------------------------------------------------------
bool SearchRequestAnnounce::GetResult(std::vector<SearchRequestResult*>& _results)
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

	m_internalRequests.clear();

	return valid;
}

void SearchRequestAnnounce::AddBorough(BoroughData& _data)
{
	m_boroughList.push_back(_data);
}

void SearchRequestAnnounce::RemoveBorough(const std::string& _name)
{
	auto it = std::find_if(m_boroughList.begin(), m_boroughList.end(), [_name](const BoroughData& _data)->bool
	{
		return _data.m_name == _name;
	});

	if (it != m_boroughList.end())
		m_boroughList.erase(it);
}

bool SearchRequestAnnounce::HasBorough(const std::string& _name)
{
	auto it = std::find_if(m_boroughList.begin(), m_boroughList.end(), [_name](const BoroughData& _data)->bool
	{
		return _data.m_name == _name;
	});

	return (it != m_boroughList.end());
}
