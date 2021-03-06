#include "PapOnlineDatabase.h"
#include "Request/SearchRequest/SearchRequest.h"
#include "Tools/StringTools.h"
#include "Request/SearchRequest/SearchRequestAnnounce.h"
#include "OnlineManager.h"
#include "extern/jsoncpp/reader.h"
#include "Request/SearchRequest/SearchRequestResultAnnounce.h"
#include "Text/TextManager.h"

using namespace ImmoBank;

AUTO_REFERENCE_ONLINE_DATABASE(PapOnlineDatabase)

void PapOnlineDatabase::Init()
{
	SetName("PAP");
	SetDatabaseName("DB_PAP");
	AddEntry("CITY", ColumnType_TEXT);
	AddEntry("ZIPCODE", ColumnType_INT);
	AddEntry("DBKEY", ColumnType_INT);

	m_intervalBetweenRequests = 30;
}

int PapOnlineDatabase::SendRequest(SearchRequest* _request)
{
	if (_request->m_requestType != SearchRequestType_Announce)
		return -1;

	SearchRequestAnnounce* announce = (SearchRequestAnnounce*)_request;

	std::string request = "https://ws.pap.fr/immobilier/annonces?";

	// Apartment / house
	int categoryID = 0;
	bool found = false;
	auto nbCategories = announce->m_categories;
	for (auto category : announce->m_categories)
	{
		switch (category)
		{
		case Category_Apartment:
		{
			request += !found ? "typebien=appartement" : "&typebien=appartement";
			found = true;
		}
		break;
		case Category_House:
		{
			request += !found ? "typebien=maison" : "&typebien=maison";
			found = true;
		}
		break;
		default:
			printf("Error: unknown request category");
			return -1;
		}

		++categoryID;
	}

	// Localisation (no borough for now)
	BoroughData borough;
	sCityData cityData;
	DatabaseManager::getSingleton()->GetCityData(announce->m_city.m_name, announce->m_city.m_zipCode, cityData, &borough);
	request += "&recherche[geo][ids][]=" + std::to_string(GetKey(borough));

	// Price
	request += "&recherche[prix][min]=" + std::to_string(announce->m_priceMin);
	request += "&recherche[prix][max]=" + std::to_string(announce->m_priceMax);

	// Surface
	request += "&recherche[surface][min]=" + std::to_string(announce->m_surfaceMin);
	request += "&recherche[surface][max]=" + std::to_string(announce->m_surfaceMax);

	// Nb rooms
	request += "&recherche[nb_pieces][min]=" + std::to_string(announce->m_nbRoomsMin);
	request += "&recherche[nb_pieces][max]=" + std::to_string(announce->m_nbRoomsMax);
	/*request += "&recherche[nb_chambres][min]=" + std::to_string(announce->m_nbBedRoomsMin);
	request += "&recherche[nb_chambres][max]=" + std::to_string(announce->m_nbBedRoomsMax);*/

	int ID = 0;
	while (m_requests.find(ID) != m_requests.end())
		++ID;

	m_requests[ID].m_requestID = OnlineManager::getSingleton()->SendBasicHTTPRequest(request);
	m_requests[ID].m_initialRequest = announce;
	m_requests[ID].m_request = request;

	return ID;
}

bool PapOnlineDatabase::_ProcessResult(SearchRequest* _initialRequest, std::string& _str, std::vector<SearchRequestResult*>& _results)
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

	Json::Value& datas = root["_embedded"]["annonce"];
	int nbAnnounces = datas.size();
	for (int announceID = 0; announceID < nbAnnounces; ++announceID)
	{
		Json::Value& data = datas[announceID];
		SearchRequestResultAnnounce* result = new SearchRequestResultAnnounce(*announce);
		result->m_database = GetName();
		std::string name = data["typebien"].asString() + std::string(" ") + std::to_string(data["nb_pieces"].asInt()) + std::string(" ") + std::string(GET_TEXT("SearchRequestResultNbRooms"));
		result->m_name = name;
		result->m_description = name;
		result->m_price = data["prix"].asInt();
		result->m_surface = (float)data["surface"].asDouble();
		result->m_URL = data["_links"]["desktop"]["href"].asString();
		auto& photos = data["_embedded"]["photo"];
		if (!photos.isNull())
			result->m_imageURL = photos.get(0u, Json::nullValue)["_links"]["self"]["href"].asString();
		result->m_nbRooms = data["nb_pieces"].asInt();
		result->m_nbBedRooms = data["nb_chambres_max"].asInt();
		std::string category = data["typebien"].asString();
		if (category == "appartement")
			result->m_category = Category_Apartment;
		else
			result->m_category = Category_House;

		result->Init();

		_results.push_back(result);
	}

	return true;
}

//-------------------------------------------------------------------------------------------------
void PapOnlineDatabase::Process()
{
	if ((m_currentKeyID > -1) && OnlineManager::getSingleton()->IsHTTPRequestAvailable(m_currentKeyID))
	{
		std::string result;
		OnlineManager::getSingleton()->GetBasicHTTPRequestResult(m_currentKeyID, result);
		m_currentKeyID = -1;
		Json::Value root;
		Json::Reader reader;
		if (reader.parse(result, root))
		{
			// Parse keys
			if (root.isObject())
			{
				Json::Value& items = root["_embedded"]["place"];
				unsigned int nbCities = items.size();
				for (unsigned int ID = 0; ID < nbCities; ++ID)
				{
					Json::Value val = items.get(ID, Json::nullValue);
					std::string name = val["slug"].asString();
					int zip = -1;
					auto delimiter = name.find_last_of("-");
					if (delimiter != std::string::npos)
					{
						std::string zipStr = name.substr(delimiter + 1, name.size());
						zip = stoi(zipStr);
						if (zipStr.size() > 3)
							zip /= 1000;
						name = name.substr(0, delimiter);
					}

					StringTools::ReplaceBadSyntax(name, "-", " ");
					StringTools::TransformToLower(name);
					StringTools::FixName(name);
					StringTools::ConvertToImGuiText(name);
					EntryData* data = GetEntryData(name, zip);
					
					if (data == nullptr)
					{
						_UpdateData(name, zip, val["id"].asInt());
					}
					else
					{
						sLocalData* localData = (sLocalData*)data;
						localData->m_cityName = name;
						localData->m_zipCode = zip;
						localData->m_key = val["id"].asInt();
						UpdateDataInternal(localData);
					}
				}
			}
		}
	}

	OnlineDatabase::Process();
}

//-------------------------------------------------------------------------------------------------
void PapOnlineDatabase::ReferenceCity(const std::string& _name)
{
	// Pap
	if (m_currentKeyID > -1)
	{
		OnlineManager::getSingleton()->CancelBasicHTTPRequest(m_currentKeyID);
		m_currentKeyID = -1;
	}

	std::string request = _ComputeKeyURL(_name);
	m_currentKeyID = OnlineManager::getSingleton()->SendBasicHTTPRequest(request, true);
}

//-------------------------------------------------------------------------------------------------
void PapOnlineDatabase::ReferenceBorough(const BoroughData& _borough)
{
	ReferenceCity(_borough.m_city.m_name);	// No specific bogough in PAP for now
}

//-------------------------------------------------------------------------------------------------
std::string PapOnlineDatabase::_ComputeKeyURL(const std::string& _name)
{
	std::string name = _name;
	StringTools::RemoveSpecialCharacters(name);
	StringTools::ReplaceBadSyntax(name, "-", "%20");
	StringTools::ReplaceBadSyntax(name, " ", "%20");
	std::string str = "https://ws.pap.fr/gis/places?recherche[cible]=pap-recherche-ac&recherche[q]=" + name;
	return str;
}

//-------------------------------------------------------------------------------------------------
bool PapOnlineDatabase::HasCity(const std::string& _name, const int _zipCode)
{
	int zip = _zipCode;
	EntryData* data = GetEntryData(_name, zip);
	if (data != nullptr)
		return true;

	zip = _zipCode / 1000;
	data = GetEntryData(_name, zip);
	if (data != nullptr)
		return true;

	return false;
}

//-------------------------------------------------------------------------------------------------
void PapOnlineDatabase::_UpdateData(const std::string& _cityName, const int _zipCode, unsigned int _key)
{
	sLocalData localData;
	localData.m_cityName = _cityName;
	localData.m_zipCode = _zipCode;
	localData.m_key = _key;
	UpdateEntryData(localData);
}

//-------------------------------------------------------------------------------------------------
EntryData* PapOnlineDatabase::_GetEntryDataFromSource(EntryData* _source) const
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
EntryData* PapOnlineDatabase::_GenerateEntryData()
{
	return new sLocalData;
}

//-------------------------------------------------------------------------------------------------
void PapOnlineDatabase::sLocalData::Generate(DatabaseHelper* _db)
{
	m_data.resize(_db->GetEntryCount());
	m_data[0].m_sVal = m_cityName;
	m_data[1].m_iVal = m_zipCode;
	m_data[2].m_iVal = m_key;
}

//-------------------------------------------------------------------------------------------------
void PapOnlineDatabase::sLocalData::Load(DatabaseHelper* _db)
{
	m_cityName = m_data[0].m_sVal;
	m_zipCode = m_data[1].m_iVal;
	m_key = m_data[2].m_iVal;
}

//-------------------------------------------------------------------------------------------------
void PapOnlineDatabase::sLocalData::copyTo(EntryData* _target)
{
	sLocalData* target = (sLocalData*)_target;
	target->m_cityName = m_cityName;
	target->m_zipCode = m_zipCode;
	target->m_key = m_key;
}

//-------------------------------------------------------------------------------------------------
void PapOnlineDatabase::_DecodeData(const std::string& _data, const sBoroughData& _sourceBorough)
{
	Json::Reader reader;
	Json::Value root;
	reader.parse(_data, root);

	// Parse keys
	if (root.isObject())
	{
		int mainZipCode = -1;
		Json::Value& items = root["_embedded"]["place"];
		unsigned int nbCities = items.size();
		for (unsigned int ID = 0; ID < nbCities; ++ID)
		{
			Json::Value val = items.get(ID, Json::nullValue);
			std::string name = val["slug"].asString();
			int zip = -1;
			auto delimiter = name.find_last_of("-");
			if (delimiter != std::string::npos)
			{
				std::string zipStr = name.substr(delimiter + 1, name.size());
				auto letter = zipStr.find_first_of("e");
				bool valid = true;
				if (letter != std::string::npos)
				{
					for (char c : zipStr)
						valid &= isdigit(c) > 0;

					if (valid)
						zip = mainZipCode + stoi(zipStr);
				}
				else
				{
					zip = stoi(zipStr);
					mainZipCode = zip;
					if (zipStr.size() < 3)
						mainZipCode *= 1000;

					if (zipStr.size() < 3)
						zip *= 1000;
				}

				if (valid)
					name = name.substr(0, delimiter);
			}

			StringTools::ReplaceBadSyntax(name, "-", " ");
			StringTools::TransformToLower(name);
			StringTools::FixName(name);
			StringTools::ConvertToImGuiText(name);

			_UpdateData(name, zip, val["id"].asInt());
		}
	}
}

//-------------------------------------------------------------------------------------------------
EntryData* PapOnlineDatabase::_GetEntryDataFromFullKey(void* _key) const
{
	std::pair<std::string,int> key = *(std::pair<std::string, int>*)_key;
	for (auto* entry : m_data)
	{
		std::pair<std::string, int> localKey = std::make_pair(entry->m_data[0].m_sVal, entry->m_data[1].m_iVal);
		if (localKey == key)
			return entry;
	}
	return nullptr;
}

//-------------------------------------------------------------------------------------------------
EntryData* PapOnlineDatabase::GetEntryData(const std::string& _cityName, const int _zipCode) const
{
	std::pair<std::string, int> key = std::make_pair(_cityName, _zipCode);
	return _GetEntryDataFromFullKey((void*)&key);
}

//-------------------------------------------------------------------------------------------------
EntryData* PapOnlineDatabase::_GetEntryDataFromCityName(const std::string& _name) const
{
	for (auto* entry : m_data)
	{
		if (entry->m_data[0].m_sVal == _name)
			return entry;
	}
	return nullptr;
}

//-------------------------------------------------------------------------------------------------
int PapOnlineDatabase::GetKey(BoroughData& _borough) const
{
	std::string name = _borough.m_city.m_name;
	StringTools::ReplaceBadSyntax(name, "-", " ");
	StringTools::TransformToLower(name);
	StringTools::FixName(name);
	StringTools::ConvertToImGuiText(name);
	EntryData* data = GetEntryData(name, _borough.m_city.m_zipCode);
	if (data == nullptr)
		data = _GetEntryDataFromCityName(name);

	if (data != nullptr)
		return data->m_data[2].m_iVal;

	return -1;
}

//-------------------------------------------------------------------------------------------------
std::string ImmoBank::PapOnlineDatabase::GetKeyAsString(BoroughData& _borough) const
{
	int key = GetKey(_borough);
	return (key != -1) ? std::to_string(key) : "NO KEY FOUND";
}