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
	AddEntry("DBKEY", ColumnType_TEXT);

	m_intervalBetweenRequests = 60;
}

//-------------------------------------------------------------------------------------------------
int LogicImmoOnlineDatabase::SendRequest(SearchRequest* _request)
{
	if (_request->m_requestType != SearchRequestType_Announce)
		return -1;

	SearchRequestAnnounce* announce = (SearchRequestAnnounce*)_request;
	std::string key = GetKey(announce->m_city);
	if (key.empty())
		return -1;

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
	request += "&localities=" + GetKey(cityData.m_data);

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
bool LogicImmoOnlineDatabase::_ProcessResult(SearchRequest* _initialRequest, std::string& _str, std::vector<SearchRequestResult*>& _results)
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
					EntryData* data = GetEntryData(name, zip);

					if (data == nullptr)
					{
						_UpdateData(name, zip, val["key"].asString());
					}
					else
					{
						sLocalData* localData = (sLocalData*)data;
						localData->m_cityName = name;
						localData->m_zipCode = zip;
						localData->m_key = val["key"].asString();
						UpdateDataInternal(localData);
					}
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

	std::string request = _ComputeKeyURL(_name);
	m_currentKeyID = OnlineManager::getSingleton()->SendBasicHTTPRequest(request, true);
}

//-------------------------------------------------------------------------------------------------
void LogicImmoOnlineDatabase::ReferenceBorough(const BoroughData& _borough)
{
	ReferenceCity(_borough.m_city.m_name);	// No specific bogough in LogicImmo for now
}

//-------------------------------------------------------------------------------------------------
std::string LogicImmoOnlineDatabase::_ComputeKeyURL(const std::string& _name)
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
	EntryData* data = GetEntryData(_name, zip);
	if (data != nullptr)
		return true;

	return false;
}

//-------------------------------------------------------------------------------------------------
void LogicImmoOnlineDatabase::_UpdateData(const std::string& _cityName, const int _zipCode, const std::string& _key)
{
	sLocalData localData;
	localData.m_cityName = _cityName;
	localData.m_zipCode = _zipCode;
	localData.m_key = _key;
	UpdateEntryData(localData);
}

//-------------------------------------------------------------------------------------------------
EntryData* LogicImmoOnlineDatabase::_GetEntryDataFromSource(EntryData* _source) const
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
EntryData* LogicImmoOnlineDatabase::_GenerateEntryData()
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
void LogicImmoOnlineDatabase::sLocalData::Load(DatabaseHelper* _db)
{
	m_cityName = m_data[0].m_sVal;
	m_zipCode = m_data[1].m_iVal;
	m_key = m_data[2].m_sVal;
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
void LogicImmoOnlineDatabase::_DecodeData(const std::string& _data, const sBoroughData& _sourceBorough)
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

			_UpdateData(name, zip, key);
		}
	}
}

//-------------------------------------------------------------------------------------------------
EntryData* LogicImmoOnlineDatabase::_GetEntryDataFromFullKey(void* _key) const
{
	std::pair<std::string, int> key = *(std::pair<std::string, int>*)_key;
	for (auto* entry : m_data)
	{
		std::pair<std::string, int> localKey = std::make_pair(entry->m_data[0].m_sVal, entry->m_data[0].m_iVal);
		if (localKey == key)
			return entry;
	}
	return nullptr;
}

//-------------------------------------------------------------------------------------------------
EntryData* LogicImmoOnlineDatabase::GetEntryData(const std::string& _cityName, const int _zipCode) const
{
	std::pair<std::string, int> key = std::make_pair(_cityName, _zipCode);
	return _GetEntryDataFromFullKey((void*)&key);
}

//-------------------------------------------------------------------------------------------------
EntryData* LogicImmoOnlineDatabase::_GetEntryDataFromCityName(const std::string& _name) const
{
	for (auto* entry : m_data)
	{
		if (entry->m_data[0].m_sVal == _name)
			return entry;
	}
	return nullptr;
}

//-------------------------------------------------------------------------------------------------
std::string LogicImmoOnlineDatabase::GetKey(sCity& _city) const
{
	std::string name = _city.m_name;
	StringTools::TransformToLower(name);
	StringTools::FixName(name);
	StringTools::ConvertToImGuiText(name);
	EntryData* data = GetEntryData(name, _city.m_zipCode);
	if (data == nullptr)
		data = _GetEntryDataFromCityName(name);

	if (data != nullptr)
		return data->m_data[2].m_sVal;

	return "";
}

/*//-------------------------------------------------------------------------------------------------
void LogicImmoOnlineDatabase::ForceUpdateDataFromMainTable()
{
	std::vector<BoroughData> list;
	DatabaseManager::getSingleton()->GetAllBoroughs(list);

	for (auto& entry : list)
	{
		EntryData* data = GetEntryData(entry.m_city.m_name, entry.m_city.m_zipCode);
		if ((data == nullptr) && (entry.m_city.m_zipCode != 0) && !entry.m_logicImmoKey.empty())
		{
			_UpdateData(entry.m_city.m_name, entry.m_city.m_zipCode, entry.m_logicImmoKey);
		}
	}
}*/