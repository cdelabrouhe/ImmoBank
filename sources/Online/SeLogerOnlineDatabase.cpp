#include "SeLogerOnlineDatabase.h"
#include "OnlineManager.h"
#include "Request/SearchRequest/SearchRequest.h"
#include "Tools/StringTools.h"
#include "Request/SearchRequest/SearchRequestAnnounce.h"
#include "Request/SearchRequest/SearchRequestResultAnnounce.h"

using namespace ImmoBank;

void SeLogerOnlineDatase::Init()
{
	SetName("SeLoger");
}

int SeLogerOnlineDatase::SendRequest(SearchRequest* _request)
{
	if (_request->m_requestType != SearchRequestType_Announce)
		return -1;

	SearchRequestAnnounce* announce = (SearchRequestAnnounce*)_request;

	std::string request = "http://ws.seloger.com/search.xml?";

	// Buy / rent
	switch (announce->m_type)
	{
	case Type_Rent:
		request += "idtt=1";
		break;
	default:
		request += "idtt=2";
		break;
	}

	// Apartment / house
	int categoryID = 0;
	auto nbCategories = announce->m_categories;
	for (auto category : announce->m_categories)
	{
		switch (category)
		{
		case Category_Apartment:
			request += categoryID == 0 ? "&idtypebien=1" : ",1";
			break;
		case Category_House:
			request += categoryID == 0 ? "&idtypebien=2" : ",2";
			break;
		default:
			printf("Error: unknown request category");
			return -1;
		}

		++categoryID;
	}

	// Force city to Montpellier (INSEE code)
	if (announce->m_boroughList.size() == 0)
	{
		int code = announce->m_city.m_inseeCode;
		int rightCode = code % 1000;
		int leftCode = (code - rightCode) / 1000;
		code = ((leftCode * 10) * 1000) + rightCode;
		request += "&ci=" + std::to_string(code);
	}
	else
	{
		std::string boroughData = "&idq=";
		std::string cityData = "&ci=";
		bool hasBoroughData = false;
		bool hasCityData = false;
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
			else
			{
				int code = announce->m_city.m_inseeCode;
				int rightCode = code % 1000;
				int leftCode = (code - rightCode) / 1000;
				code = ((leftCode * 10) * 1000) + rightCode;

				if (!firstCity)
					cityData += ",";
				cityData += std::to_string(code);
				hasCityData = true;
				firstCity = false;
			}
		}

		if (hasBoroughData)
			request += boroughData;

		if (hasCityData)
			request += cityData;
	}

	// Tri
	request += "&tri=initial";

	// Price
	request += "&pxmin=" + std::to_string(announce->m_priceMin);
	request += "&pxmax=" + std::to_string(announce->m_priceMax);

	// Surface
	request += "&surfacemin=" + std::to_string(announce->m_surfaceMin);
	request += "&surfacemax=" + std::to_string(announce->m_surfaceMax);

	// Nb rooms
	request += "&nb_pieces=" + std::to_string(announce->m_nbRoomsMin);
	for (int roomID = announce->m_nbRoomsMin + 1; roomID <= announce->m_nbRoomsMax; ++roomID)
		request += "," + std::to_string(roomID);

	request += "&nb_chambres=" + std::to_string(announce->m_nbBedRoomsMin);
	for (int roomID = announce->m_nbBedRoomsMin + 1; roomID <= announce->m_nbBedRoomsMax; ++roomID)
		request += "," + std::to_string(roomID);
	
	int ID = 0;
	while (m_requests.find(ID) != m_requests.end())
		++ID;

	m_requests[ID].m_requestID = OnlineManager::getSingleton()->SendBasicHTTPRequest(request);
	m_requests[ID].m_initialRequest = announce;

	return ID;
}

bool SeLogerOnlineDatase::IsRequestAvailable(const int _requestID)
{
	auto it = m_requests.find(_requestID);
	if (it != m_requests.end())
		return OnlineManager::getSingleton()->IsBasicHTTPRequestAvailable(it->second.m_requestID);

	return false;
}

bool SeLogerOnlineDatase::GetRequestResult(const int _requestID, std::vector<SearchRequestResult*>& _result)
{
	auto it = m_requests.find(_requestID);
	if (it != m_requests.end())
	{
		std::string str;
		if (OnlineManager::getSingleton()->GetBasicHTTPRequestResult(it->second.m_requestID, str))
		{
			SearchRequest* request = it->second.m_initialRequest;
			bool result = ProcessResult(request, str, _result);
			DeleteRequest(_requestID);
			return result;
		}
	}
	return false;
}

void SeLogerOnlineDatase::Process()
{

}

void SeLogerOnlineDatase::End()
{

}

bool SeLogerOnlineDatase::ProcessResult(SearchRequest* _initialRequest, std::string& _str, std::vector<SearchRequestResult*>& _results)
{
	if (_initialRequest->m_requestType != SearchRequestType_Announce)
		return false;

	SearchRequestAnnounce* announce = (SearchRequestAnnounce*)_initialRequest;

	sRecherche recherche;
	recherche.Serialize(_str);

	for (auto& annonce : recherche.m_summary.m_annonces)
	{
		SearchRequestResultAnnounce* result = new SearchRequestResultAnnounce(*announce);
		result->m_database = GetName();
		result->m_name = annonce.m_name;
		result->m_description = annonce.m_description;
		result->m_price = annonce.m_price;
		result->m_surface = annonce.m_surface;
		result->m_URL = annonce.m_URL;
		result->m_nbRooms = annonce.m_nbRooms;
		result->m_nbBedRooms = annonce.m_nbBedRooms;
		result->m_category = annonce.m_category;
		result->m_inseeCode = annonce.m_inseeCode;

		_results.push_back(result);
	}
	return true;
}

void SeLogerOnlineDatase::sRecherche::Serialize(const std::string& _str)
{
	std::string str = StringTools::GetXMLBaliseContent(_str, "recherche");
	m_summary.Serialize(str);
}

void SeLogerOnlineDatase::sSummary::Serialize(const std::string& _str)
{
	m_resume = StringTools::GetXMLBaliseContent(_str, "resume");
	m_resumeSansTri = StringTools::GetXMLBaliseContent(_str, "resumeSansTri");
	std::string str = StringTools::GetXMLBaliseContent(_str, "nbTrouvees");
	if (!str.empty())
		m_nbAnnonces = std::stoi(str);
	str = StringTools::GetXMLBaliseContent(_str, "nbAffichables");
	if (!str.empty())
		m_nbAnnoncesAffichables = std::stoi(str);

	std::vector<std::string> list;
	StringTools::GetXMLBaliseArray(_str, "annonces", "annonce", list);

	m_annonces.resize(list.size());
	for (auto ID = 0; ID < m_annonces.size(); ++ID)
	{
		m_annonces[ID].Serialize(list[ID]);
	}
}

void SeLogerOnlineDatase::sAnnonce::Serialize(const std::string& _str)
{
	std::string str = StringTools::GetXMLBaliseContent(_str, "idAnnonce");
	if (!str.empty())	m_ID = std::stoi(str);

	m_name = StringTools::GetXMLBaliseContent(_str, "libelle");
	m_description = StringTools::GetXMLBaliseContent(_str, "descriptif");
	m_URL = StringTools::GetXMLBaliseContent(_str, "permaLien");

	str = StringTools::GetXMLBaliseContent(_str, "prix");
	if (!str.empty())	m_price = std::stoi(str);

	str = StringTools::GetXMLBaliseContent(_str, "surface");
	if (!str.empty())	m_surface = std::stof(str);

	str = StringTools::GetXMLBaliseContent(_str, "nbPiece");
	if (!str.empty())	m_nbRooms = std::stoi(str);

	str = StringTools::GetXMLBaliseContent(_str, "nbChambre");
	if (!str.empty())	m_nbBedRooms = std::stoi(str);

	str = StringTools::GetXMLBaliseContent(_str, "codeInsee");
	if (!str.empty())	m_inseeCode = std::stoi(str);

	str = StringTools::GetXMLBaliseContent(_str, "idTypeBien");
	if (!str.empty())
	{
		int type = std::stoi(str);
		switch (type)
		{
		case 1:			m_category = Category_Apartment;	break;
		case 2:			m_category = Category_House;		break;
		default:		m_category = Category_NONE;			break;
		}
	}

	static bool s_test = false;
	if (s_test)
	{
		FILE* f = fopen("test.xml", "wt");
		if (!f)
			return;
		fwrite(_str.data(), sizeof(char), (size_t)_str.size(), f);
		fclose(f);
	}
}
