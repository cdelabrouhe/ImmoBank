#include "SearchRequest.h"
#include <Tools/StringTools.h>
#include "extern/ImGui/imgui.h"
#include "Online/OnlineManager.h"
#include "Online/OnlineDatabase.h"
#include "extern/jsoncpp/reader.h"
#include "extern/jsoncpp/value.h"
#include "SearchResult.h"
#include <algorithm>

//---------------------------------------------------------------------------------------------------------------------------------
void SearchRequest::copyTo(SearchRequest* _target)
{
	_target->m_requestType = m_requestType;
}

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

static const char* s_characters = "abcdefghijklmnopqrstuvwxyz123456789";
static auto s_nbCharacters = strlen(s_characters);

//---------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------
void SearchRequestCityBoroughs::Init()
{
	for (int ID = 0; ID < s_nbCharacters; ++ID)
	{
		char character = s_characters[ID];
		std::string str = "%20" + std::string((const char*)(&s_characters[ID]));
		str.resize(4);
		//std::string str = " " + std::string((const char*)(&character));
		std::string request = "https://api.meilleursagents.com/geo/v1/?q=" + m_city.m_name + str + "&types=arrmuns,boroughs";
		m_httpRequestsID.push_back(OnlineManager::getSingleton()->SendBasicHTTPRequest(request));
	}
}

//---------------------------------------------------------------------------------------------------------------------------------
void SearchRequestCityBoroughs::copyTo(SearchRequest* _target)
{
	SearchRequest::copyTo(_target);
	if (_target->m_requestType != SearchRequestType_CityBoroughs)
		return;

	SearchRequestCityBoroughs* target = (SearchRequestCityBoroughs*)_target;
	target->m_city = m_city;
}

//---------------------------------------------------------------------------------------------------------------------------------
bool SearchRequestCityBoroughs::IsAvailable() const
{
	bool available = true;
	int ID = 0;
	for (ID = 0; ID < m_httpRequestsID.size(); ++ID)
	{
		available &= OnlineManager::getSingleton()->IsBasicHTTPRequestAvailable(m_httpRequestsID[ID]);
		if (!available)
			break;
	}

	return available;
}

//---------------------------------------------------------------------------------------------------------------------------------
bool SearchRequestCityBoroughs::GetResult(std::vector<SearchRequestResult*>& _results)
{
	bool valid = true;
	for (int ID = 0; ID < m_httpRequestsID.size(); ++ID)
	{
		std::string str;
		if (OnlineManager::getSingleton()->GetBasicHTTPRequestResult(m_httpRequestsID[ID], str))
		{
			Json::Reader reader;
			Json::Value root;
			reader.parse(str, root);

			Json::Value& places = root["response"]["places"];
			if (places.isArray())
			{
				const int nbPlaces = places.size();
				for (int ID = 0; ID < nbPlaces; ++ID)
				{
					Json::Value val = places.get(ID, Json::nullValue);
					std::string name = val["name"].asString();
					int coma = (int)name.find_first_of(",");
					if (coma > -1)
						name = name.substr(0, coma);

					std::string strInternalID = val["id"].asString();
					unsigned int internalID = std::stoi(strInternalID);

					StringTools::RemoveSpecialCharacters(name);

					// Search if this borough is not already in the list
					bool found = false;
					for (auto result : _results)
					{
						if (result->m_resultType == SearchRequestType_CityBoroughs)
						{
							SearchRequestResulCityBorough* borough = static_cast<SearchRequestResulCityBorough*>(result);
							if (borough->m_name == name)
								found = true;
						}
					}

					// Unknown borough => add it
					if (!found)
					{
						SearchRequestResulCityBorough* result = new SearchRequestResulCityBorough();
						result->m_name = name;
						result->m_internalID = internalID;
						_results.push_back(result);
					}
				}
			}
		}
		else
			valid = false;
	}

	return valid;
}

//---------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------
void SearchRequestCityData::Init()
{
	// Get boroughs (if they exist) from Database
	if (!DatabaseManager::getSingleton()->GetBoroughs(m_city.m_name, m_boroughs))
	{
		m_state = UpdateStep_GetBoroughList;
		
		SearchRequestCityBoroughs boroughs;
		boroughs.m_city = m_city;
		m_boroughsRequestID = OnlineManager::getSingleton()->SendRequest(&boroughs);
	}
	else
	{
		InitBoroughPricesRequest();
	}
}

//---------------------------------------------------------------------------------------------------------------------------------
void SearchRequestCityData::InitBoroughPricesRequest()
{
	if (m_boroughs.size() == 0)
	{
		m_state = UpdateStep_COUNT;
		return;
	}

	if (m_boroughs[0].m_timeUpdate.GetData() == 0)
	{
		m_state = UpdateStep_ComputeBoroughsPrices;

		for (auto ID = 0; ID < m_boroughs.size(); ++ID)
		{
			SearchRequestCityBoroughData data;
			data.m_data = m_boroughs[ID];
			data.m_city = m_city;
			m_httpRequestsID.push_back(OnlineManager::getSingleton()->SendRequest(&data));
		}
	}
	else
		m_state = UpdateStep_COUNT;
}

//---------------------------------------------------------------------------------------------------------------------------------
void SearchRequestCityData::Process()
{
	switch (m_state)
	{
		// Borough list
	case UpdateStep_GetBoroughList:
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
					sBoroughData data;
					data.m_cityName = m_city.m_name;
					data.m_name = borough->m_name;
					data.m_key = borough->m_internalID;
					m_boroughs.push_back(data);

					// Store data into DB
					DatabaseManager::getSingleton()->AddBoroughData(data);
					delete borough;
				}
			}

			InitBoroughPricesRequest();
		}
		break;

		// Boroughs prices
	case UpdateStep_ComputeBoroughsPrices:
	{
		bool available = true;
		int ID = 0;
		for (ID = 0; ID < m_httpRequestsID.size(); ++ID)
		{
			available &= OnlineManager::getSingleton()->IsRequestAvailable(m_httpRequestsID[ID]);
			if (!available)
				break;
		}

		if (available)
		{
			bool valid = true;
			for (int ID = 0; ID < m_httpRequestsID.size(); ++ID)
			{
				std::vector<SearchRequestResult*> list;
				if (OnlineManager::getSingleton()->GetRequestResult(m_httpRequestsID[ID], list))
				{
					m_boroughsRequestID = -1;

					for (auto result : list)
					{
						if (result->m_resultType == SearchRequestType_CityBoroughData)
						{
							SearchRequestResulCityBoroughData* borough = static_cast<SearchRequestResulCityBoroughData*>(result);
							sBoroughData data = borough->m_data;
							auto it = std::find_if(m_boroughs.begin(), m_boroughs.end(), [data](sBoroughData& _data)->bool { return _data.m_name == data.m_name; });
							if (it != m_boroughs.end())
							{
								*it = data;

								// Store data into DB
								DatabaseManager::getSingleton()->AddBoroughData(data);
							}							
						}
					}
				}
			}

			IncreaseStep();
		}
	}
	break;
	}
}

//---------------------------------------------------------------------------------------------------------------------------------
void SearchRequestCityData::End()
{
	
}

//---------------------------------------------------------------------------------------------------------------------------------
void SearchRequestCityData::copyTo(SearchRequest* _target)
{
	SearchRequest::copyTo(_target);
	if (_target->m_requestType != SearchRequestType_CityData)
		return;

	SearchRequestCityData* target = (SearchRequestCityData*)_target;
	target->m_city = m_city;
}

//---------------------------------------------------------------------------------------------------------------------------------
bool SearchRequestCityData::IsAvailable() const
{
	return m_state == UpdateStep_COUNT;
}

//---------------------------------------------------------------------------------------------------------------------------------
bool SearchRequestCityData::GetResult(std::vector<SearchRequestResult*>& _results)
{
	if (!IsAvailable())
		return false;

	if (m_boroughsRequestID > -1)
		return false;

	bool valid = true;
	return valid;
}

//---------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------
void SearchRequestCityBoroughData::Init()
{
	//std::string request = "https://www.meilleursagents.com/prix-immobilier/montpellier-34000/quartier_antigone-170492247/"
	std::string boroughName = m_data.m_name;
	StringTools::ReplaceBadSyntax(boroughName, " ", "-");
	StringTools::ReplaceBadSyntax(boroughName, "'", "-");
	std::string request = "https://www.meilleursagents.com/prix-immobilier/" + m_city.m_name + "-" + std::to_string(m_city.m_zipCode) + "/quartier_" + boroughName + "-" + std::to_string(m_data.m_key);
	StringTools::TransformToLower(request);
	m_httpRequestsID = OnlineManager::getSingleton()->SendBasicHTTPRequest(request);
}

//---------------------------------------------------------------------------------------------------------------------------------
void SearchRequestCityBoroughData::copyTo(SearchRequest* _target)
{
	SearchRequest::copyTo(_target);
	if (_target->m_requestType != SearchRequestType_CityBoroughData)
		return;

	SearchRequestCityBoroughData* target = (SearchRequestCityBoroughData*)_target;
	target->m_city = m_city;
	target->m_data = m_data;
}

//---------------------------------------------------------------------------------------------------------------------------------
bool SearchRequestCityBoroughData::IsAvailable() const
{
	if (m_httpRequestsID > -1)
		return OnlineManager::getSingleton()->IsBasicHTTPRequestAvailable(m_httpRequestsID);

	return true;
}

//---------------------------------------------------------------------------------------------------------------------------------
bool SearchRequestCityBoroughData::GetResult(std::vector<SearchRequestResult*>& _results)
{
	bool valid = true;
	if (m_httpRequestsID > -1)
	{
		std::string str;
		if (OnlineManager::getSingleton()->GetBasicHTTPRequestResult(m_httpRequestsID, str))
		{
			SearchRequestResulCityBoroughData* result = new SearchRequestResulCityBoroughData();
			result->m_data = m_data;
			result->m_data.m_priceApartmentBuyMax = 1.0f;
			result->m_data.m_priceHouseBuyMin = 1.0f;
			result->m_data.m_priceHouseBuyMax = 1.0f;
			result->m_data.m_priceRentMin = 1.0f;
			result->m_data.m_priceRentMax = 1.0f;
			_results.push_back(result);
		}
		else
			valid = false;
	}

	return valid;
}