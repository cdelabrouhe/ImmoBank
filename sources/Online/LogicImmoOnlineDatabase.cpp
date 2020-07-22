#include "LogicImmoOnlineDatabase.h"
#include "Request/SearchRequest/SearchRequest.h"
#include "Tools/StringTools.h"
#include "Request/SearchRequest/SearchRequestAnnounce.h"
#include "OnlineManager.h"
#include "extern/jsoncpp/reader.h"
#include "Request/SearchRequest/SearchRequestResultAnnounce.h"

using namespace ImmoBank;

//-------------------------------------------------------------------------------------------------
void LogicImmoOnlineDatabase::Init()
{
	SetName("LogicImmo");
	SetDatabaseName("DB_LOGICIMMO");
	AddEntry("CITY", ColumnType_TEXT);
	AddEntry("ZIPCODE", ColumnType_INT);
	AddEntry("KEY", ColumnType_TEXT);

	m_intervalBetweenRequests = 30;
}

//-------------------------------------------------------------------------------------------------
int LogicImmoOnlineDatabase::SendRequest(SearchRequest* _request)
{
	if (_request->m_requestType != SearchRequestType_Announce)
		return -1;

	SearchRequestAnnounce* announce = (SearchRequestAnnounce*)_request;

	// Doc: https://github.com/axeleroy/untoitpourcaramel
	// Cherche ville avec http://lisemobile.logic-immo.com/li.search_localities.php?client=v8.a&fulltext=Montpellier
	// Requete avec http://lisemobile.logic-immo.com/li.search_ads.php

	std::string request = "http://lisemobile.logic-immo.com/li.search_ads.php?client=v8.a&domain=sales";

	// Apartment / house => no such information in LogicImmo parameters (or just didn't find it)
	int categoryID = 0;
	auto nbCategories = announce->m_categories;
	for (auto category : announce->m_categories)
	{
		switch (category)
		{
		case Category_Apartment:
			//request += "&appartement=on";
			break;
		case Category_House:
			//request += "&maison=on";
			break;
		default:
			//printf("Error: unknown request category");
			return -1;
		}

		++categoryID;
	}

	// Localisation (no borough for now)
	BoroughData borough;
	sCityData cityData;
	DatabaseManager::getSingleton()->GetCityData(announce->m_city.m_name, announce->m_city.m_zipCode, cityData, &borough);
	request += "&localities=" + borough.m_logicImmoKey;

	// Price
	request += "&price_range=" + std::to_string(announce->m_priceMin) + "," + std::to_string(announce->m_priceMax);

	// Surface
	request += "&area_range=" + std::to_string(announce->m_surfaceMin) + "," + std::to_string(announce->m_surfaceMax);

	// Nb rooms
	request += "&rooms_range=" + std::to_string(announce->m_nbRoomsMin);
	for (int roomID = announce->m_nbRoomsMin + 1; roomID <= announce->m_nbRoomsMax; ++roomID)
		request += "," + std::to_string(roomID);

	// Nb bedrooms
	request += "&bedrooms_range=" + std::to_string(announce->m_nbBedRoomsMin);
	for (int roomID = announce->m_nbBedRoomsMin + 1; roomID <= announce->m_nbBedRoomsMax; ++roomID)
		request += "," + std::to_string(roomID);

	int ID = 0;
	while (m_requests.find(ID) != m_requests.end())
		++ID;

	m_requests[ID].m_requestID = OnlineManager::getSingleton()->SendBasicHTTPRequest(request, true);
	m_requests[ID].m_initialRequest = announce;
	m_requests[ID].m_request = request;

	return ID;
}

//-------------------------------------------------------------------------------------------------
bool LogicImmoOnlineDatabase::ProcessResult(SearchRequest* _initialRequest, std::string& _str, std::vector<SearchRequestResult*>& _results)
{
	if (_initialRequest->m_requestType != SearchRequestType_Announce)
		return false;

	if (_str.empty())
		return true;

	SearchRequestAnnounce* announce = (SearchRequestAnnounce*)_initialRequest;

	/*std::string str;
	FILE* f = fopen("data_test_laforet.html", "rt");
	if (f)
	{
		char* test_data = (char*)malloc(10000000);
		fread(test_data, sizeof(char), 10000000, f);
		fclose(f);
		str = test_data;
		free(test_data);
	}*/

	Json::Value root;
	Json::Reader reader;
	reader.parse(_str, root);

	Json::Value& datas = root["items"];
	int nbAnnounces = datas.size();
	for (int announceID = 0; announceID < nbAnnounces; ++announceID)
	{
		Json::Value& data = datas[announceID];
		SearchRequestResultAnnounce* result = new SearchRequestResultAnnounce(*announce);
		result->m_database = GetName();
		result->m_name = data["info"]["propertyType"]["name"].asString();
		StringTools::RemoveSpecialCharacters(result->m_name);
		result->m_description = data["info"]["text"].asString();
		StringTools::RemoveSpecialCharacters(result->m_description);
		result->m_price = data["pricing"]["amount"].asInt();
		result->m_surface = (float)data["properties"]["area"].asDouble();
		result->m_URL = data["info"]["link"].asString();
		result->m_imageURL = data["pictures"].get(0u, Json::nullValue).asString();
		StringTools::FindAndReplaceAll(result->m_imageURL, "[WIDTH]", "640");
		StringTools::FindAndReplaceAll(result->m_imageURL, "[HEIGHT]", "480");
		StringTools::FindAndReplaceAll(result->m_imageURL, "[SCALE]", "1");
		result->m_nbRooms = data["properties"]["rooms"].asInt();
		result->m_nbBedRooms = data["properties"]["bedrooms"].asInt();
		int category = stoi(data["info"]["propertyType"]["identifier"].asString());
		if (category == 1)
			result->m_category = Category_Apartment;
		else
			result->m_category = Category_House;

		result->Init();

		_results.push_back(result);
	}

	return true;
}

//-------------------------------------------------------------------------------------------------
void LogicImmoOnlineDatabase::Process()
{
	// LogicImmo
	if ((m_currentKeyID > -1) && OnlineManager::getSingleton()->IsHTTPRequestAvailable(m_currentKeyID))
	{
		std::string result;
		OnlineManager::getSingleton()->GetBasicHTTPRequestResult(m_currentKeyID, result);
		m_currentKeyID = -1;
		Json::Value root;
		Json::Reader reader;
		if (reader.parse(result, root))
		{
			// Parse LogicImmo keys
			if (root.isObject())
			{
				Json::Value& items = root["items"];
				unsigned int nbCities = items.size();
				for (unsigned int ID = 0; ID < nbCities; ++ID)
				{
					Json::Value val = items.get(ID, Json::nullValue);
					std::string name = val["name"].asString();
					std::string zipCode = val["postCode"].asString();
					if (zipCode.size() == 0)
						continue;

					int zip = stoi(zipCode);
					zip /= 1000;
					StringTools::TransformToLower(name);
					StringTools::FixName(name);
					StringTools::ConvertToImGuiText(name);
					m_keys[std::make_pair(name, zip)] = val["key"].asString();
				}
			}
		}
	}

	OnlineDatabase::Process();
}

//-------------------------------------------------------------------------------------------------
void LogicImmoOnlineDatabase::ReferenceCity(const std::string& _name)
{
	if (m_currentKeyID > -1)
	{
		OnlineManager::getSingleton()->CancelBasicHTTPRequest(m_currentKeyID);
		m_currentKeyID = -1;
	}

	sCityData data;
	DatabaseManager::getSingleton()->GetCityData(_name, -1, data);
	if (data.m_data.m_logicImmoKey.empty())
	{
		std::string request = ComputeKeyURL(_name);
		m_currentKeyID = OnlineManager::getSingleton()->SendBasicHTTPRequest(request, true);
	}
	else
		m_keys[std::make_pair(_name, -1)] = data.m_data.m_logicImmoKey;
}

//-------------------------------------------------------------------------------------------------
void LogicImmoOnlineDatabase::ReferenceBorough(const BoroughData& _borough)
{
	ReferenceCity(_borough.m_city.m_name);	// No specific bogough in LogicImmo for now
}

//-------------------------------------------------------------------------------------------------
std::string LogicImmoOnlineDatabase::ComputeKeyURL(const std::string& _name)
{
	std::string name = _name;
	StringTools::RemoveSpecialCharacters(name);
	StringTools::ReplaceBadSyntax(name, "-", "%20");
	StringTools::ReplaceBadSyntax(name, " ", "%20");
	std::string request = "http://lisemobile.logic-immo.com/li.search_localities.php?client=v8.a&fulltext=" + name;
	return request;
}

//-------------------------------------------------------------------------------------------------
bool LogicImmoOnlineDatabase::HasCity(const std::string& _name, const int _zipCode, sCity& _city)
{
	int zip = _zipCode / 1000;
	auto it = m_keys.find(std::make_pair(_name, zip));
	if (it != m_keys.end())
	{
		_city.m_logicImmoKey = it->second;
		return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
bool* LogicImmoOnlineDatabase::ForceUpdate()
{
	return &m_forceUpdateInProgress;
}

//-------------------------------------------------------------------------------------------------
void LogicImmoOnlineDatabase::UpdateData(const std::string& _cityName, const int _zipCode, const std::string& _key)
{
	auto pair = std::make_pair(_cityName, _zipCode);
	auto it = m_keys.find(pair);
	if (it == m_keys.end())
		m_keys[pair] = _key;

	sLocalData localData;
	localData.m_cityName = _cityName;
	localData.m_zipCode = _zipCode;
	localData.m_key = _key;
	UpdateEntryData(localData);
}

//-------------------------------------------------------------------------------------------------
EntryData* LogicImmoOnlineDatabase::GetEntryDataFromSource(EntryData* _source)
{
	sLocalData* source = (sLocalData*)_source;
	for (auto* data : m_data)
	{
		sLocalData* localData = (sLocalData*)data;
		if ((source->m_cityName == localData->m_cityName) && (source->m_zipCode == localData->m_zipCode))
			return data;
	}

	return nullptr;
}

//-------------------------------------------------------------------------------------------------
EntryData* LogicImmoOnlineDatabase::GenerateEntryData()
{
	return new sLocalData;
}

//-------------------------------------------------------------------------------------------------
void LogicImmoOnlineDatabase::sLocalData::Generate(DatabaseHelper* _db)
{
	m_data.resize(_db->GetEntryCount());
	m_data[0].m_sVal = m_cityName;
	m_data[1].m_iVal = m_zipCode;
	m_data[2].m_sVal = m_key;
}

//-------------------------------------------------------------------------------------------------
void LogicImmoOnlineDatabase::sLocalData::copyTo(EntryData* _target)
{
	sLocalData* target = (sLocalData*)_target;
	target->m_cityName = m_cityName;
	target->m_zipCode = m_zipCode;
	target->m_key = m_key;
}

//-------------------------------------------------------------------------------------------------
void LogicImmoOnlineDatabase::DecodeData(const std::string& _data, const sBoroughData& _sourceBorough)
{
	Json::Reader reader;
	Json::Value root;
	reader.parse(_data, root);

	Json::Value& places = root["items"];
	if (places.isArray())
	{
		const int nbPlaces = places.size();
		for (int placeID = 0; placeID < nbPlaces; ++placeID)
		{
			Json::Value val = places.get(placeID, Json::nullValue);
			std::string name = val["name"].asString();
			if (name.empty())
				continue;

			StringTools::TransformToLower(name);
			StringTools::ReplaceBadSyntax(name, " ", "-");
			std::string key = val["key"].asString();
			std::string zipCode = val["postCode"].asString();
			int zip = _sourceBorough.m_data.m_city.m_zipCode;
			if (!zipCode.empty())
				zip = stoi(zipCode);
			else
				continue;

			UpdateData(name, zip, key);
		}
	}
}
