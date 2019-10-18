#include "LaforetOnlineDatabase.h"
#include "Request/SearchRequest/SearchRequest.h"
#include "Tools/StringTools.h"
#include "Request/SearchRequest/SearchRequestAnnounce.h"
#include "OnlineManager.h"
#include "extern/jsoncpp/reader.h"
#include "Request/SearchRequest/SearchRequestResultAnnounce.h"

DISABLE_OPTIMIZE
using namespace ImmoBank;

void LaforetOnlineDatabase::Init()
{
	SetName("Laforet");
}

int LaforetOnlineDatabase::SendRequest(SearchRequest* _request)
{
	if (_request->m_requestType != SearchRequestType_Announce)
		return -1;

	SearchRequestAnnounce* announce = (SearchRequestAnnounce*)_request;

	std::string request = "http://www.laforet.com/acheter/rechercher?slug=&ajaxValue=0";

	// Apartment / house
	int categoryID = 0;
	auto nbCategories = announce->m_categories;
	for (auto category : announce->m_categories)
	{
		switch (category)
		{
		case Category_Apartment:
			request += "&appartement=on";
			break;
		case Category_House:
			request += "&maison=on";
			break;
		default:
			printf("Error: unknown request category");
			return -1;
		}

		++categoryID;
	}

	std::string boroughData = "&idq=";
	std::string cityData = "&ci=";
	bool hasBoroughData = false;
	bool firstBorough = true;
	bool firstCity = true;
	for (auto& borough : announce->m_boroughList)
	{
		bool isCity = false;
		int boroughKey = -1;
		boroughKey = borough.GetSelogerKey(&isCity);
		if (!isCity)
		{
			if (!firstBorough)
				boroughData += ",";
			boroughData += std::to_string(boroughKey);
			hasBoroughData = true;
			firstBorough = false;
		}
	}

	if (hasBoroughData)
		request += boroughData;

	// Price
	request += "&price_min=" + std::to_string(announce->m_priceMin);
	request += "&price_max=" + std::to_string(announce->m_priceMax);

	// Surface
	request += "&surface_min=" + std::to_string(announce->m_surfaceMin);
	request += "&surface_max=" + std::to_string(announce->m_surfaceMax);

	// Nb rooms
	for (int roomID = announce->m_nbRoomsMin; roomID <= announce->m_nbRoomsMax; ++roomID)
		request += "&rooms" + std::to_string(roomID) + "=" + std::to_string(roomID);

	for (int roomID = announce->m_nbBedRoomsMin + 1; roomID <= announce->m_nbBedRoomsMax; ++roomID)
		request += "&bedrooms" + std::to_string(roomID) + "=" + std::to_string(roomID);

	int ID = 0;
	while (m_requests.find(ID) != m_requests.end())
		++ID;

	m_requests[ID].m_requestID = OnlineManager::getSingleton()->SendBasicHTTPRequest(request);
	m_requests[ID].m_initialRequest = announce;

	return ID;
}

bool LaforetOnlineDatabase::ProcessResult(SearchRequest* _initialRequest, std::string& _str, std::vector<SearchRequestResult*>& _results)
{
	if (_initialRequest->m_requestType != SearchRequestType_Announce)
		return false;

	SearchRequestAnnounce* announce = (SearchRequestAnnounce*)_initialRequest;

	sRecherche recherche;
	recherche.Serialize(_str);

	for (auto& annonce : recherche.m_annonces)
	{
		SearchRequestResultAnnounce* result = new SearchRequestResultAnnounce(*announce);
		result->m_database = GetName();
		result->m_name = annonce.m_name;
		result->m_description = annonce.m_description;
		result->m_price = annonce.m_price;
		result->m_surface = annonce.m_surface;
		result->m_URL = annonce.m_URL;
		result->m_imageURL = annonce.m_imageURL;
		result->m_nbRooms = annonce.m_nbRooms;
		result->m_nbBedRooms = annonce.m_nbBedRooms;
		result->m_category = annonce.m_category;

		result->Init();

		_results.push_back(result);
	}
	return true;
}

void LaforetOnlineDatabase::sRecherche::Serialize(const std::string& _str)
{
	std::string startStr = "json";
	std::string stopStr = "pictos";
	int startID = _str.find(startStr);

	std::string str = _str.substr(startID, _str.size());
	while (str.size() > 0)
	{
		int start = str.find(startStr);
		int stop = str.find(stopStr);

		// End => stop serialization
		if ((start == std::string::npos) && (stop == std::string::npos))
		{
			str = "";
			continue;
		}

		std::string strAnnonce = str.substr(startStr.size(), stop);
		m_annonces.push_back(sAnnonce());
		m_annonces.back().Serialize(strAnnonce);

		str = str.substr(stop + stopStr.size());
	}
	printf("");
}

void ImmoBank::LaforetOnlineDatabase::sAnnonce::Serialize(const std::string& _str)
{
	int start = _str.find_first_of("{");
	int stop = _str.find_first_of("}");
	std::string strJson = _str.substr(start, stop);
	StringTools::ReplaceBadSyntax(strJson, "&quot;", "\"");
	Json::Reader reader;
	Json::Value root;
	reader.parse(strJson, root);
	printf("");
}
