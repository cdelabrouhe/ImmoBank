#include "PapOnlineDatabase.h"
#include "Request/SearchRequest/SearchRequest.h"
#include "Tools/StringTools.h"
#include "Request/SearchRequest/SearchRequestAnnounce.h"
#include "OnlineManager.h"
#include "extern/jsoncpp/reader.h"
#include "Request/SearchRequest/SearchRequestResultAnnounce.h"

using namespace ImmoBank;

void PapOnlineDatabase::Init()
{
	SetName("PAP");
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
			request += !found ? "&types=apartment" : "%2Capartment";
			found = true;
		}
		break;
		case Category_House:
		{
			request += !found ? "&types=house" : "%2Chouse";
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
	request += "&cities=" + std::to_string(announce->m_city.m_inseeCode);

	// Price
	request += "&min=" + std::to_string(announce->m_priceMin);
	request += "&max=" + std::to_string(announce->m_priceMax);

	// Surface
	request += "&surface=" + std::to_string(announce->m_surfaceMin);

	// Nb rooms
	request += "&rooms=" + std::to_string(announce->m_nbRoomsMax);
	request += "&bedrooms" + std::to_string(announce->m_nbBedRoomsMax);

	int ID = 0;
	while (m_requests.find(ID) != m_requests.end())
		++ID;

	m_requests[ID].m_requestID = OnlineManager::getSingleton()->SendBasicHTTPRequest(request);
	m_requests[ID].m_initialRequest = announce;
	m_requests[ID].m_request = request;

	return ID;
}

bool PapOnlineDatabase::ProcessResult(SearchRequest* _initialRequest, std::string& _str, std::vector<SearchRequestResult*>& _results)
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

	Json::Value& datas = root["data"];
	int nbAnnounces = datas.size();
	for (int announceID = 0; announceID < nbAnnounces; ++announceID)
	{
		Json::Value& data = datas[announceID];
		SearchRequestResultAnnounce* result = new SearchRequestResultAnnounce(*announce);
		result->m_database = GetName();
		std::string name = data["slug"].asString();
		result->m_name = name;
		auto findID = result->m_name.find_last_of("-");
		result->m_name = result->m_name.substr(0, findID);
		StringTools::ReplaceBadSyntax(result->m_name, "-", " ");
		result->m_description = data["description"].asString();
		StringTools::RemoveSpecialCharacters(result->m_description);
		result->m_price = data["price"].asInt();
		result->m_surface = data["surface"].asDouble();
		result->m_URL = "https://www.laforet.com/agence-immobiliere/" + data["agency"]["slug"].asString() + "/acheter/" + data["address"]["city_slug"].asString() + "/" + name;
		result->m_imageURL = data["photos"].get(0u, Json::nullValue).asString();
		result->m_nbRooms = data["rooms"].asInt();
		result->m_nbBedRooms = data["bedrooms"].asInt();
		std::string category = data["type"].asString();
		if (category == "apartment")
			result->m_category = Category_Apartment;
		else
			result->m_category = Category_House;

		result->Init();

		_results.push_back(result);
	}

	return true;
}