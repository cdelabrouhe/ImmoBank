#include "RequestManager.h"
#include "EditableRequest/EditableRequest.h"
#include <algorithm>
#include "EditableRequest/EditableRequestAnnounce.h"
#include "EditableRequest/EditableRequestCityData.h"

RequestManager* s_singleton = nullptr;

//-------------------------------------------------------------------------------------------------
RequestManager* RequestManager::getSingleton()
{
	if (s_singleton == nullptr)
		s_singleton = new RequestManager();
	return s_singleton;
}

//-------------------------------------------------------------------------------------------------
void RequestManager::Init()
{
	
}

//-------------------------------------------------------------------------------------------------
void RequestManager::Process()
{
	for (auto& request : m_requests)
	{
		if (request.m_deleted)
		{
			DeleteRequest(request.m_request);
			break;
		}
	}
	
	for (auto& request : m_requests)
		request.m_request->Process();
}

//-------------------------------------------------------------------------------------------------
void RequestManager::End()
{

}

//-------------------------------------------------------------------------------------------------
void RequestManager::DisplayRequests()
{
	unsigned int ID = 0;
	for (auto& request : m_requests)
		request.m_request->Display(ID++);
}

//-------------------------------------------------------------------------------------------------
EditableRequest* RequestManager::CreateRequest(SearchRequest* _request)
{
	EditableRequest* request = nullptr;

	switch (_request->m_requestType)
	{
		case SearchRequestType_Announce:
		{
			request = new EditableRequestAnnounce();
			request->Init(_request);
			m_requests.push_back(sRequest(request));
		}
		break;

		case SearchRequestType_CityData:
		{
			request = new EditableRequestCityData();
			request->Init(_request);
			m_requests.push_back(sRequest(request));
		}
		break;

		default:
			break;
	}
	
	return request;
}

//-------------------------------------------------------------------------------------------------
EditableRequest* RequestManager::CreateRequestAnnounceDefault()
{
	SearchRequestAnnounce request;
	request.m_type = Type_Buy;
	request.m_city.m_name = "Montpellier";
	request.m_categories.push_back(Category_Apartment);
	request.m_categories.push_back(Category_House);
	request.m_priceMin = 210000;
	request.m_priceMax = 215000;
	request.m_nbRooms = 3;
	request.m_nbBedRooms = 2;
	request.m_surfaceMin = 50;
	request.m_surfaceMax = 70;

	return CreateRequest(&request);
}

//-------------------------------------------------------------------------------------------------
EditableRequest* RequestManager::CreateRequestCityDataDefault()
{
	SearchRequestCityData request;
	request.m_city.m_name = "Montpellier";

	return CreateRequest(&request);
}

//-------------------------------------------------------------------------------------------------
void RequestManager::AskForDeleteRequest(EditableRequest* _request)
{
	RequestManager::sRequest* request = getRequest(_request);
	if (request != nullptr)
		request->m_deleted = true;
}

//-------------------------------------------------------------------------------------------------
void RequestManager::DeleteRequest(EditableRequest* _request)
{
	std::vector<sRequest>::iterator it = std::find_if(m_requests.begin(), m_requests.end(), [_request](const sRequest& _r)->bool { return _r.m_request == _request; });
	if (it != m_requests.end())
	{
		RequestManager::sRequest& request = *it;
		request.m_request->End();
		delete request.m_request;
		m_requests.erase(it);
	}
}

//-------------------------------------------------------------------------------------------------
RequestManager::sRequest* RequestManager::getRequest(EditableRequest* _request)
{
	auto it = std::find_if(m_requests.begin(), m_requests.end(), [_request](const sRequest& _r)->bool { return _r.m_request == _request; });
	if (it != m_requests.end())
		return &(*it);
	return nullptr;
}