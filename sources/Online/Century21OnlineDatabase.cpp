#include "Century21OnlineDatabase.h"
#include "Request/SearchRequest/SearchRequest.h"
#include "Tools/StringTools.h"
#include "Request/SearchRequest/SearchRequestAnnounce.h"
#include "OnlineManager.h"
#include "extern/jsoncpp/reader.h"
#include "Request/SearchRequest/SearchRequestResultAnnounce.h"
#include "extern/TinyXML/tinyxml.h"

using namespace ImmoBank;

AUTO_REFERENCE_ONLINE_DATABASE(Century21OnlineDatabase)

void Century21OnlineDatabase::Init()
{
	SetName("Century21");
}

std::string Century21OnlineDatabase::GetKey(BoroughData& _borough) const
{
	std::string key;
	if (_borough.m_name == s_wholeCityName)
	{
		key = "cpv-" + std::to_string(_borough.m_city.m_zipCode) + "_" + _borough.m_city.m_name;
		StringTools::TransformToLower(key);
	}
	else
	{
		key = "cp-" + std::to_string(_borough.m_city.m_zipCode);
	}
	return key;
}

std::string Century21OnlineDatabase::GetKeyAsString(BoroughData& _borough) const
{
	return GetKey(_borough);
}

int Century21OnlineDatabase::SendRequest(SearchRequest* _request)
{
	if (_request->m_requestType != SearchRequestType_Announce)
		return -1;

	SearchRequestAnnounce* announce = (SearchRequestAnnounce*)_request;

	// Cherche ville avec https://www.century21.fr/autocomplete/localite/?q=montpellier
	// Recherche avec https://www.century21.fr/annonces/achat/cpv-34000_montpellier/b-0-460000/?nombres_de_pieces=2&nombres_de_pieces=3

	std::string request = "https://www.century21.fr/annonces/achat/";

	// Localisation
	BoroughData borough;
	sCityData cityData;
	DatabaseManager::getSingleton()->GetCityData(announce->m_city.m_name, announce->m_city.m_zipCode, cityData, &borough);
	request += "/" + GetKey(borough);

	// Surface
	request += "/s-" + std::to_string(announce->m_surfaceMin) + "-";

	// Surface terrain
	request += "/st-0-";

	// Price
	request += "/b-0-" + std::to_string(announce->m_priceMax);

	// Nb rooms
	request += "/p-" + std::to_string(announce->m_nbRoomsMin);
	for (int roomID = announce->m_nbRoomsMin + 1; roomID <= announce->m_nbRoomsMax; ++roomID)
		request += "-" + std::to_string(roomID);

	request += "/";

	int ID = 0;
	while (m_requests.find(ID) != m_requests.end())
		++ID;

	m_requests[ID].m_requestID = OnlineManager::getSingleton()->SendBasicHTTPRequest(request, true);
	m_requests[ID].m_initialRequest = announce;
	m_requests[ID].m_request = request;

	return ID;
}

bool Century21OnlineDatabase::_ProcessResult(SearchRequest* _initialRequest, std::string& _str, std::vector<SearchRequestResult*>& _results)
{
	if (_initialRequest->m_requestType != SearchRequestType_Announce)
		return false;

	std::string doc = _str;
	/*FILE* f = fopen("data_test_century21.html", "rt");
	if (f)
	{
		char* test_data = (char*)malloc(10000000);
		fread(test_data, sizeof(char), 10000000, f);
		fclose(f);
		doc = test_data;
		free(test_data);
	}*/

	if (doc.empty())
		return true;

	SearchRequestAnnounce* announce = (SearchRequestAnnounce*)_initialRequest;

	std::string limit = "<div id=\"bien_";
	std::string linkDelimiter = "<a href=";
	std::string priceDelimiter = "<div class=\"price tw-py-1 tw-text-xl tw-font-semibold tw-text-center\">";
	std::string titleDelimiter = "alt=";
	std::string imgDelimiter = "src=";
	std::string infDelimiter = "<br/>";
	int size = limit.size();
	auto delimiter = doc.find(limit);
	while (delimiter != std::string::npos)
	{
		SearchRequestResultAnnounce* result = new SearchRequestResultAnnounce(*announce);
		result->m_database = GetName();
		doc = doc.substr(delimiter + size, doc.size());
		delimiter = doc.find(limit);
		std::string announceData = doc.substr(0, delimiter);
		auto findLink = announceData.find(linkDelimiter);

		// URL
		std::string link = announceData.substr(findLink + linkDelimiter.size() + 1, announceData.size());
		result->m_URL = "https://www.century21.fr" + link.substr(0, link.find("\""));

		// Price
		auto findPrice = announceData.find(priceDelimiter);
		std::string priceData = announceData.substr(findPrice + priceDelimiter.size(), announceData.size());
		priceData = priceData.substr(0, priceData.find("&"));
		StringTools::ReplaceBadSyntax(priceData, " ", "");
		result->m_price = stoi(priceData);

		// Title
		auto findTitle = announceData.find(titleDelimiter);
		std::string titleData = announceData.substr(findTitle + titleDelimiter.size() + 1, announceData.size());
		titleData = titleData.substr(0, titleData.find("\""));
		StringTools::RemoveSpecialCharacters(titleData);
		result->m_name = titleData;
		result->m_description = titleData;

		// Image
		auto findImg = announceData.find(imgDelimiter);
		std::string imgData = announceData.substr(findImg + imgDelimiter.size() + 1, announceData.size());
		imgData = imgData.substr(0, imgData.find("\""));
		StringTools::RemoveSpecialCharacters(imgData);
		result->m_imageTinyURL = "https://www.century21.fr" + imgData;
		result->m_imageURL = result->m_imageTinyURL;

		// Data
		auto findData = announceData.find(infDelimiter);
		std::string infData = announceData.substr(findData + infDelimiter.size(), announceData.size());
		infData = infData.substr(0, infData.find("</h4>"));
		StringTools::RemoveSpecialCharacters(infData);

		// Surface
		std::string surfaceStr = infData.substr(0, infData.find_last_of(","));
		auto surfaceDelimiter = surfaceStr.find_first_of(",");
		if (surfaceDelimiter != std::string::npos)
		{
			StringTools::ReplaceBadSyntax(surfaceStr, " ", "");
			surfaceStr = surfaceStr.substr(0, surfaceDelimiter);
		}
		result->m_surface = stof(surfaceStr);

		// Nb rooms
		std::string nbRoomsStr = infData.substr(infData.find_last_of(",") + 1, infData.size());
		StringTools::RemoveEOL(nbRoomsStr);
		StringTools::ReplaceBadSyntax(nbRoomsStr, " ", "");
		nbRoomsStr = nbRoomsStr.substr(0, 1);
		result->m_nbRooms = stoi(nbRoomsStr);
		result->m_nbBedRooms = result->m_nbRooms;

		result->Init();

		_results.push_back(result);

		delimiter = doc.find(limit);
	}

	return true;
}