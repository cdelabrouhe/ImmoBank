#include "PapOnlineDatabase.h"
#include "Request/SearchRequest/SearchRequest.h"
#include "Tools/StringTools.h"
#include "Request/SearchRequest/SearchRequestAnnounce.h"
#include "OnlineManager.h"
#include "extern/jsoncpp/reader.h"
#include "Request/SearchRequest/SearchRequestResultAnnounce.h"
#include "Text/TextManager.h"

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
	request += "&recherche[geo][ids][]=" + std::to_string(announce->m_city.m_papKey);

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
					auto pair = std::make_pair(name, zip);
					auto it = m_keys.find(pair);
					if (it == m_keys.end())
						m_keys[pair] = val["id"].asUInt();
				}
			}
		}
	}
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

	sCityData data;
	DatabaseManager::getSingleton()->GetCityData(_name, -1, data);
	if (data.m_data.m_papKey == 0)
	{
		std::string request = ComputeKeyURL(_name);
		m_currentKeyID = OnlineManager::getSingleton()->SendBasicHTTPRequest(request, true);
	}
	else
		m_keys[std::make_pair(_name, -1)] = data.m_data.m_papKey;
}

//-------------------------------------------------------------------------------------------------
std::string PapOnlineDatabase::ComputeKeyURL(const std::string& _name)
{
	std::string name = _name;
	StringTools::RemoveSpecialCharacters(name);
	StringTools::ReplaceBadSyntax(name, "-", "%20");
	StringTools::ReplaceBadSyntax(name, " ", "%20");
	std::string str = "https://ws.pap.fr/gis/places?recherche[cible]=pap-recherche-ac&recherche[q]=" + name;
	return str;
}

//-------------------------------------------------------------------------------------------------
bool PapOnlineDatabase::HasCity(const std::string& _name, const int _zipCode, sCity& _city)
{
	int zip = _zipCode / 1000;
	auto it = m_keys.find(std::make_pair(_name, zip));
	if (it != m_keys.end())
	{
		_city.m_papKey = it->second;
		return true;
	}

	return false;
}