#include "OrpiOnlineDatabase.h"
#include "Request/SearchRequest/SearchRequest.h"
#include "Tools/StringTools.h"
#include "Request/SearchRequest/SearchRequestAnnounce.h"
#include "OnlineManager.h"
#include "extern/jsoncpp/reader.h"
#include "Request/SearchRequest/SearchRequestResultAnnounce.h"

using namespace ImmoBank;
DISABLE_OPTIMIZE
void OrpiOnlineDatabase::Init()
{
	SetName("ORPI");
}

int OrpiOnlineDatabase::SendRequest(SearchRequest* _request)
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

	// Localisation (no borough for now)
	request += "&localisation=" + announce->m_city.m_name + "%28" + std::to_string(announce->m_city.m_zipCode) + "%29";

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

	m_requests[ID].m_requestID = 0;// OnlineManager::getSingleton()->SendBasicHTTPRequest(request);
	m_requests[ID].m_initialRequest = announce;

	return ID;
}

bool OrpiOnlineDatabase::ProcessResult(SearchRequest* _initialRequest, std::string& _str, std::vector<SearchRequestResult*>& _results)
{
	/*if (_initialRequest->m_requestType != SearchRequestType_Announce)
		return false;

	if (_str.empty())
		return true;*/

	SearchRequestAnnounce* announce = (SearchRequestAnnounce*)_initialRequest;

	std::string str;
	FILE* f = fopen("data_test_orpi.html", "rt");
	if (f)
	{
		char* test_data = (char*)malloc(10000000);
		fread(test_data, sizeof(char), 10000000, f);
		fclose(f);
		str = test_data;
		free(test_data);
	}

	sRecherche recherche;
	recherche.Serialize(str);

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

	static bool s_test = false;
	if (s_test)
	{
		FILE* f = fopen("error.txt", "wt");
		if (f)
		{
			fwrite(str.data(), sizeof(char), (size_t)str.size(), f);
			fclose(f);
		}
	}
		
	StringTools::ReplaceBadSyntax(str, "&", "");
	StringTools::ReplaceBadSyntax(str, "quot;quot;", "\"");
	StringTools::ReplaceBadSyntax(str, "quot;", "\"");
	StringTools::ReplaceBadSyntax(str, "#x20;", " ");
	StringTools::ReplaceBadSyntax(str, "#x7B;", "{");
	StringTools::ReplaceBadSyntax(str, "#x7D;", "}");
	StringTools::ReplaceBadSyntax(str, "#x3A;", ":");
	StringTools::ReplaceBadSyntax(str, "#x5B;", "[");
	StringTools::ReplaceBadSyntax(str, "#x5D;", "]");
	StringTools::ReplaceBadSyntax(str, "#x2F;", "/");
	StringTools::ReplaceBadSyntax(str, "#x5C;", "\\");

	Json::Reader reader;
	Json::Value root;
	reader.parse(str, root);
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
}

bool OrpiOnlineDatabase::sAnnonce::Serialize(const std::string& _str)
{
	auto start = _str.find_first_of("{");
	auto stop = _str.find_first_of("}");

	if ((start == std::string::npos) || (stop == std::string::npos))
		return false;

	std::string strJson = _str.substr(start, stop);
	StringTools::ReplaceBadSyntax(strJson, "&quot;", "\"");
	Json::Reader reader;
	Json::Value root;
	reader.parse(strJson, root);

	m_city = root["city"].asString();
	m_name = root["title"].asString();
	m_description = root["description"].asString();
	StringTools::RemoveSpecialCharacters(m_name);
	StringTools::RemoveSpecialCharacters(m_description);
	m_URL = "http://www.laforet.com" + root["url"].asString();
	m_imageURL = root["imageUrl"].asString();
	std::string price = root["price"].asString();
	StringTools::ReplaceBadSyntax(price, " ", "");
	m_price = std::stoi(price);
	m_surface = (float)std::stoi(root["surface"].asString());
	m_nbRooms = std::stoi(root["roomsQuantity"].asString());
	m_nbBedRooms = std::stoi(root["bedroomsQuantity"].asString());

	std::string str = root["propertyType"].asString();
	if (!str.empty())
	{
		int type = std::stoi(str);
		switch (type)
		{
		case 1:			m_category = Category_House;		break;
		case 2:			m_category = Category_Apartment;	break;
		default:		m_category = Category_NONE;			break;
		}
	}

	return true;
}

bool ImmoBank::OrpiOnlineDatabase::IsRequestAvailable(int _requestID)
{
	return true;
}
