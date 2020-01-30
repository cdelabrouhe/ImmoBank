#include "OrpiOnlineDatabase.h"
#include "Request/SearchRequest/SearchRequest.h"
#include "Tools/StringTools.h"
#include "Request/SearchRequest/SearchRequestAnnounce.h"
#include "OnlineManager.h"
#include "extern/jsoncpp/reader.h"
#include "Request/SearchRequest/SearchRequestResultAnnounce.h"

using namespace ImmoBank;

void OrpiOnlineDatabase::Init()
{
	SetName("ORPI");
}

int OrpiOnlineDatabase::SendRequest(SearchRequest* _request)
{
	if (_request->m_requestType != SearchRequestType_Announce)
		return -1;

	SearchRequestAnnounce* announce = (SearchRequestAnnounce*)_request;

	std::string request = "https://www.orpi.com/recherche/buy?transaction=buy&resultUrl=";

	// Apartment / house
	int categoryID = 0;
	auto nbCategories = announce->m_categories;
	for (auto category : announce->m_categories)
	{
		switch (category)
		{
		case Category_Apartment:
			request += "&realEstateTypes%5B0%5D=appartement";
			break;
		case Category_House:
			request += "&realEstateTypes%5B0%5D=maison";
			break;
		default:
			printf("Error: unknown request category");
			return -1;
		}

		++categoryID;
	}

	// Localisation (no borough for now)
	std::string city = announce->m_city.m_name;
	StringTools::TransformToLower(city);
	request += "&locations%5B0%5D%5Bvalue%5D=" + city;
	request += "&locations%5B0%5D%5Blabel%5D=" + announce->m_city.m_name + "+(" + std::to_string(announce->m_city.m_zipCode) + ")";

	// Price
	request += "&minPrice=" + std::to_string(announce->m_priceMin);
	request += "&maxPrice=" + std::to_string(announce->m_priceMax);

	// Surface
	request += "&minSurface=" + std::to_string(announce->m_surfaceMin);
	request += "&maxSurface=" + std::to_string(announce->m_surfaceMax);

	// Nb rooms
	for (int roomID = announce->m_nbRoomsMin; roomID <= announce->m_nbRoomsMax; ++roomID)
		request += "&nbRooms" + std::to_string(roomID) + "=" + std::to_string(roomID);

	for (int roomID = announce->m_nbBedRoomsMin + 1; roomID <= announce->m_nbBedRoomsMax; ++roomID)
		request += "&nbBedrooms" + std::to_string(roomID) + "=" + std::to_string(roomID);

	int ID = 0;
	while (m_requests.find(ID) != m_requests.end())
		++ID;

	m_requests[ID].m_requestID = OnlineManager::getSingleton()->SendBasicHTTPRequest(request);
	m_requests[ID].m_initialRequest = announce;
	m_requests[ID].m_request = request;

	static bool s_test = false;
	if (s_test)
	{
		FILE* f = fopen("result.txt", "wt");
		if (f)
		{
			fwrite(request.data(), sizeof(char), (size_t)request.size(), f);
			fclose(f);
		}
	}

	return ID;
}

bool OrpiOnlineDatabase::ProcessResult(SearchRequest* _initialRequest, std::string& _str, std::vector<SearchRequestResult*>& _results)
{
	if (_initialRequest->m_requestType != SearchRequestType_Announce)
		return false;

	if (_str.empty())
		return true;

	SearchRequestAnnounce* announce = (SearchRequestAnnounce*)_initialRequest;

	/*std::string str;
	FILE* f = fopen("data_test_orpi.html", "rt");
	if (f)
	{
		char* test_data = (char*)malloc(10000000);
		fread(test_data, sizeof(char), 10000000, f);
		fclose(f);
		str = test_data;
		free(test_data);
	}*/

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

void OrpiOnlineDatabase::sRecherche::Serialize(const std::string& _str)
{
	std::string startStr = "data-result=";
	std::string stopStr = "&#x7D;\"";
	auto delimiter = _str.find(startStr);
	std::string str = _str.substr(delimiter + startStr.size() + 1, _str.size());

	delimiter = str.find(stopStr);
	str = str.substr(0, delimiter + stopStr.size() - 1);

	//StringTools::ReplaceBadSyntax(str, "&", "");
	StringTools::ReplaceBadSyntax(str, "&qquot;", "\"");
	StringTools::ReplaceBadSyntax(str, "&quot;quot;", "\"");
	StringTools::ReplaceBadSyntax(str, "&quot;", "\"");
	StringTools::ReplaceBadSyntax(str, "&#x20;", " ");
	StringTools::ReplaceBadSyntax(str, "&#x7B;", "{");
	StringTools::ReplaceBadSyntax(str, "&#x7D;", "}");
	StringTools::ReplaceBadSyntax(str, "&#x3A;", ":");
	StringTools::ReplaceBadSyntax(str, "&#x5B;", "[");
	StringTools::ReplaceBadSyntax(str, "&#x5D;", "]");
	StringTools::ReplaceBadSyntax(str, "&#x2F;", "/");
	StringTools::ReplaceBadSyntax(str, "&#x5C;", "");

	Json::Reader reader;
	Json::Value root;
	reader.parse(str, root);

	Json::Value& items = root["items"];
	const int nbItems = (int)items.size();
	for (int ID = 0; ID < nbItems; ++ID)
	{
		Json::Value data = items[ID];
		sAnnonce announce;
		if (announce.Serialize(data))
			m_annonces.push_back(announce);
	}

#if 0
	str = root.toStyledString();

	if (s_test)
	{
		FILE* f = fopen("result.txt", "wt");
		if (f)
		{
			fwrite(str.data(), sizeof(char), (size_t)str.size(), f);
			fclose(f);
		}
	}

	while (str.size() > 0)
	{
		auto start = str.find(startStr);
		auto stop = str.find(stopStr);

		// End => stop serialization
		if ((start == std::string::npos) && (stop == std::string::npos))
		{
			str = "";
			continue;
		}

		std::string strAnnonce = str.substr(start, stop);
		sAnnonce announce;
		if (announce.Serialize(strAnnonce))
		{
			m_annonces.push_back(announce);
			str = str.substr(stop + stopStr.size(), str.size());
		}
		else
			str = "";
	}
#endif
}

bool OrpiOnlineDatabase::sAnnonce::Serialize(const Json::Value& _data)
{
	m_city = _data["locationDescription"].asString();
	m_name = _data["slug"].asString();
	m_description = _data["longAd"].asString();
	StringTools::RemoveSpecialCharacters(m_name);
	StringTools::RemoveSpecialCharacters(m_description);
	m_URL = "https://www.orpi.com/annonce-vente-" + _data["slug"].asString();
	m_imageURL = _data["images"].get(0u, Json::nullValue).asString();
	m_price = _data["price"].asUInt();
	m_surface = (float)_data["surface"].asDouble();
	m_nbRooms = _data["nbRooms"].asUInt();
	m_nbBedRooms = !_data["nbBedrooms"].isNull() ? _data["nbBedrooms"].asUInt() : m_nbRooms - 1;

	std::string str = _data["obeType"].asString();
	if (!str.empty())
	{
		if (str == "apartment")
			m_category = Category_Apartment;
		else if (str == "house")
			m_category = Category_House;
		else
			m_category = Category_NONE;
	}

	return true;
}