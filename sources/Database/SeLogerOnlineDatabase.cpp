#include "SeLogerOnlineDatabase.h"
#include "HTTPDownloader.h"
#include "Request/SearchRequest.h"
#include "Tools/StringTools.h"

void SeLogerOnlineDatase::Init(HTTPDownloader* _downloader)
{
	m_downloader = _downloader;
	SetName("SeLoger");
}

int SeLogerOnlineDatase::SendRequest(const SearchRequest& _request)
{
	std::string request = "http://ws.seloger.com/search.xml?";

	// Buy / rent
	switch (_request.m_type)
	{
	case Type_Rent:
		request += "idtt=1";
		break;
	default:
		request += "idtt=2";
		break;
	}

	// Apartment / house
	switch (_request.m_category)
	{
	case Category_Apartment:
		request += "&idtypebien=1";
		break;
	case Category_House:
		request += "&idtypebien=2";
		break;
	default:
		printf("Error: unknown request category");
		return -1;
	}

	// Force city to Montpellier (INSEE code)
	int code = _request.m_city.m_inseeCode;
	int rightCode = code % 1000;
	int leftCode = (code - rightCode) / 1000;
	code = ((leftCode * 10) * 1000) + rightCode;
	request += "&ci=" + std::to_string(code);

	// Tri
	request += "&tri=initial";

	// Price
	request += "&pxmin=" + std::to_string(_request.m_priceMin);
	request += "&pxmax=" + std::to_string(_request.m_priceMax);

	// Surface
	request += "&surfacemin=" + std::to_string(_request.m_surfaceMin);
	request += "&surfacemax=" + std::to_string(_request.m_surfaceMax);

	// Nb rooms
	request += "&nb_pieces=" + std::to_string(_request.m_nbRooms);
	request += "&nb_chambres=" + std::to_string(_request.m_nbBedRooms);
	
	int ID = 0;
	while (m_requests.find(ID) != m_requests.end())
		++ID;

	m_requests[ID].m_requestID = m_downloader->SendRequest(request);
	m_requests[ID].m_initialRequest = _request;

	return ID;
}

bool SeLogerOnlineDatase::IsRequestAvailable(const int _requestID)
{
	auto it = m_requests.find(_requestID);
	if (it != m_requests.end())
		return m_downloader->IsRequestAvailable(it->second.m_requestID);

	return false;
}

bool SeLogerOnlineDatase::GetRequestResult(const int _requestID, std::vector<SearchRequestResult>& _result)
{
	auto it = m_requests.find(_requestID);
	if (it != m_requests.end())
	{
		std::string str;
		if (m_downloader->GetResult(it->second.m_requestID, str))
			return ProcessResult(it->second.m_initialRequest, str, _result);
	}
	return false;
}

void SeLogerOnlineDatase::Process()
{

}

void SeLogerOnlineDatase::End()
{

}

bool SeLogerOnlineDatase::ProcessResult(SearchRequest& _initialRequest, std::string& _str, std::vector<SearchRequestResult>& _results)
{
	sRecherche recherche;
	recherche.Serialize(_str);

	for (auto& annonce : recherche.m_summary.m_annonces)
	{
		SearchRequestResult result(_initialRequest);
		result.m_database = GetName();
		result.m_name = annonce.m_name;
		result.m_description = annonce.m_description;
		result.m_price = annonce.m_price;
		result.m_surface = annonce.m_surface;
		result.m_URL = annonce.m_URL;
		result.m_nbRooms = annonce.m_nbRooms;
		result.m_nbBedRooms = annonce.m_nbBedRooms;

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
	for (int ID = 0; ID < m_annonces.size(); ++ID)
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
}
