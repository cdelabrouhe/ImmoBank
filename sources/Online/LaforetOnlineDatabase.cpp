#include "LaforetOnlineDatabase.h"
#include "Request/SearchRequest/SearchRequest.h"
#include "Tools/StringTools.h"
#include "Request/SearchRequest/SearchRequestAnnounce.h"
#include "OnlineManager.h"
#include "extern/jsoncpp/reader.h"
#include "Request/SearchRequest/SearchRequestResultAnnounce.h"

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

	std::string request = "https://www.laforet.com/api/immo/properties?page=1&perPage=40&transaction=buy";

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

bool LaforetOnlineDatabase::ProcessResult(SearchRequest* _initialRequest, std::string& _str, std::vector<SearchRequestResult*>& _results)
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
		result->m_surface = (float)data["surface"].asDouble();
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

/*void LaforetOnlineDatabase::sRecherche::Serialize(const std::string& _str, SearchRequestAnnounce* _initialRequest)
{
	std::string startStr = "{return";
	std::string stopStr = ";";
	auto startID = _str.find(startStr);
	std::string str = _str.substr(startID, _str.size());

	std::string announceStartStr = "{immo_id:";

	while (str.size() > 0)
	{
		auto start = str.find(announceStartStr);

		int charID = start + 1;
		int cpt = 1;
		while (cpt > 0)
		{
			if (str[charID] == '{')
			{
				++cpt;
			}
			else if (str[charID] == '}')
			{
				--cpt;
			}
			++charID;
		}

		auto stop = charID;

		// End => stop serialization
		if ((start == std::string::npos) || (stop == std::string::npos))
		{
			str = "";
			continue;
		}

		std::string strAnnonce = str.substr(start, stop);
		sAnnonce announce;
		if (announce.Serialize(strAnnonce, _initialRequest))
		{
			m_annonces[announce.m_ID] = announce;
			str = str.substr(stop + 1, str.size());
		}
		else
			str = "";
	}

	// HTML extraction (remove end of file)
	std::string strHTML = _str;
	auto findID = strHTML.find("window.__NUXT__=(function(");
	if (findID != std::string::npos)
	{
		// Parse all announces and try to find the equivalent in the HTML code
		strHTML = strHTML.substr(0, findID);
		for (auto& entry : m_annonces)
		{
			int ID = entry.first;
			auto& ann = entry.second;
			std::string str = strHTML;
			
			// Look for ID
			findID = str.find(std::to_string(ID));

			ann.m_URL = "http://www.laforet.com" + ExtractStringFromPosition(str, findID, '\"');

			auto startID = findID - 30;
			str = str.substr(startID, findID - startID);
			
			// Extract short description
			findID = str.find_last_of("/");
			str = str.substr(findID + 1, str.size());
			findID = str.find_first_of("-");

			// Extract category
			std::string tmp = str.substr(0, findID);
			StringTools::TransformToLower(tmp);
			if (tmp == "appartement")
				ann.m_category = Category_Apartment;
			else
				ann.m_category = Category_House;

			// Extract nomber of rooms
			str = str.substr(findID + 1, str.size());
			findID = str.find_first_of("-");
			tmp = str.substr(0, findID);
			ann.m_nbRooms = stoi(tmp);
			ann.m_nbBedRooms = ann.m_nbRooms - 1;	// No information about bedrooms => assume it's nbRooms - 1

			// Generate announce name
			ann.m_name = (ann.m_category == Category_Apartment ? "Appartement " : "Maison ") + std::to_string(ann.m_nbRooms) + " pieces";
		}
	}
}

void ExtractArray(const std::string& _str, const std::string& _lookFor, std::vector<std::string>& _list)
{
	auto findID = _str.find(_lookFor);
	if (findID != std::string::npos)
	{
		std::string str = _str.substr(findID + _lookFor.size() + 1, _str.size());
		auto startArrayID = str.find_first_of("[");
		auto stopArrayID = str.find_first_of("]");
		str = str.substr(startArrayID + 1, stopArrayID - startArrayID);

		StringTools::ReplaceBadSyntax(str, "\"", "");
		StringTools::ReplaceBadSyntax(str, "\\u002F", "/");

		auto commaID = str.find_first_of(",");
		while (commaID != std::string::npos)
		{
			_list.push_back(str.substr(0, commaID));
			str = str.substr(commaID + 1, str.size());
			commaID = str.find_first_of(",");
		}
	}
}

std::string Extract(const std::string& _str, const std::string& _lookFor)
{
	auto findID = _str.find(_lookFor);
	if (findID != std::string::npos)
	{
		std::string str = _str.substr(findID + _lookFor.size() + 1, _str.size());
		auto stopCommaID = str.find_first_of(",");
		auto stopParenthesisID = str.find_first_of("}");
		bool validComma = stopCommaID > 0;
		bool validParenthesis = stopParenthesisID > 0;
		size_t stopID = 0;
		if (!validComma)
			stopID = stopParenthesisID;
		else if (!validParenthesis)
			stopID = stopCommaID;
		else
			stopID = stopCommaID < stopParenthesisID ? stopCommaID : stopParenthesisID;

		return str.substr(0, stopID);
	}
	return "";
}

int ExtractInt(const std::string& _str, const std::string& _lookFor)
{
	std::string str = Extract(_str, _lookFor);
	if (!str.empty())
	{
		return stoi(str);
	}
	return 0;
}

float ExtractFloat(const std::string& _str, const std::string& _lookFor)
{
	std::string str = Extract(_str, _lookFor);
	if (!str.empty())
	{
		return stof(str);
	}
	return 0.f;
}

std::string ExtractString(const std::string& _str, const std::string& _lookFor)
{
	std::string str = Extract(_str, _lookFor);
	if (!str.empty())
	{
		StringTools::ReplaceBadSyntax(str, "\"", "");
		StringTools::ReplaceBadSyntax(str, "\\", "");
		StringTools::RemoveSpecialCharacters(str);
		return str;
	}
	return "";
}*/