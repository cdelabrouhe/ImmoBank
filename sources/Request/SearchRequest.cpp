#include "SearchRequest.h"
#include <Tools/StringTools.h>
#include "extern/ImGui/imgui.h"
#include <windows.h>
#include <shellapi.h>
#include "Database/DatabaseManager.h"
#include "Database/OnlineDatabase.h"
#include "extern/jsoncpp/reader.h"
#include "extern/jsoncpp/value.h"

void SearchRequestResultAnnounce::PostProcess()
{
	StringTools::RemoveSpecialCharacters(m_name);
	StringTools::RemoveSpecialCharacters(m_description);
}

void SearchRequestResultAnnounce::Display()
{
	ImGui::Separator();
	ImGui::SetWindowFontScale(1.2f);
	std::string name = (m_category == Category_Apartment ? "Apartment" : "House");
	name += ", " + m_name;
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), name.c_str());
	ImGui::SetWindowFontScale(1.0f);

	ImGui::TextWrapped(m_description.c_str());
	ImGui::Text("Price: %u", m_price);
	ImGui::Text("Surface: %.0f", m_surface);
	ImGui::Text("Nb rooms: %u", m_nbRooms);
	ImGui::Text("Nb bedrooms: %u", m_nbBedRooms);
	ImGui::TextColored(ImVec4(0.2f, 0.75f, 1.0f, 1.0f), "URL (%s)", m_database.c_str());
	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
		ShellExecuteA(NULL, "open", m_URL.c_str(), NULL, NULL, SW_SHOWDEFAULT);
}

void SearchRequest::copyTo(SearchRequest* _target)
{
	_target->m_requestType = m_requestType;
}

void SearchRequestAnnounce::Init()
{
	SearchRequestCityBoroughs boroughs;
	boroughs.m_city = m_city;
	m_boroughsRequestID = DatabaseManager::getSingleton()->SendRequest(&boroughs);
}

void SearchRequestAnnounce::Process()
{
	if ((m_boroughsRequestID > -1) && DatabaseManager::getSingleton()->IsRequestAvailable(m_boroughsRequestID))
	{
		std::vector<SearchRequestResult*> list;
		DatabaseManager::getSingleton()->GetRequestResult(m_boroughsRequestID, list);
		m_boroughsRequestID = -1;

		for (auto result : list)
		{
			if (result->m_resultType == SearchRequestType_CityBoroughs)
			{
				SearchRequestResulCityBorough* borough = static_cast<SearchRequestResulCityBorough*>(result);
				m_boroughs.push_back(borough->m_name);
				delete borough;
			}
		}

		// Trigger internal requests
		auto databases = DatabaseManager::getSingleton()->GetOnlineDatabases();
		for (auto db : databases)
			m_internalRequests.push_back(std::make_pair(db, db->SendRequest(this)));
	}
}

void SearchRequestAnnounce::End()
{
	for (auto& request : m_internalRequests)
	{
		OnlineDatabase* db = request.first;
		db->DeleteRequest(request.second);
	}
	m_internalRequests.clear();
}

void SearchRequestAnnounce::copyTo(SearchRequest* _target)
{
	SearchRequest::copyTo(_target);
	if (_target->m_requestType != SearchRequestType_Announce)
		return;

	SearchRequestAnnounce* target = (SearchRequestAnnounce*)_target;
	target->m_city = m_city;
	target->m_type = m_type;
	target->m_categories = m_categories;
	target->m_priceMin = m_priceMin;
	target->m_priceMax = m_priceMax;
	target->m_surfaceMin = m_surfaceMin;
	target->m_surfaceMax = m_surfaceMax;
	target->m_nbRooms = m_nbRooms;
	target->m_nbBedRooms = m_nbBedRooms;
}

bool SearchRequestAnnounce::IsAvailable() const
{
	if (m_boroughsRequestID > -1)
		return false;

	bool valid = true;
	for (auto& request : m_internalRequests)
	{
		OnlineDatabase* db = request.first;
		int requestID = request.second;
		valid &= db->IsRequestAvailable(requestID);
	}
	return valid;
}

bool SearchRequestAnnounce::GetResult(std::vector<SearchRequestResult*>& _results)
{
	if (!IsAvailable())
		return false;

	if (m_boroughsRequestID > -1)
		return false;

	bool valid = true;
	for (auto& request : m_internalRequests)
	{
		OnlineDatabase* db = request.first;
		int requestID = request.second;
		valid &= db->GetRequestResult(requestID, _results);
		db->DeleteRequest(requestID);
	}

	m_internalRequests.clear();

	return valid;
}

void SearchRequestCityBoroughs::Init()
{
	std::string request = "https://api.meilleursagents.com/geo/v1/?q=" + m_city.m_name + "^&types=boroughs";
	m_httpRequestID = DatabaseManager::getSingleton()->SendBasicHTTPRequest(request);
}

void SearchRequestCityBoroughs::copyTo(SearchRequest* _target)
{
	SearchRequest::copyTo(_target);
	if (_target->m_requestType != SearchRequestType_CityBoroughs)
		return;

	SearchRequestCityBoroughs* target = (SearchRequestCityBoroughs*)_target;
	target->m_city = m_city;
}

bool SearchRequestCityBoroughs::IsAvailable() const
{
	if (m_httpRequestID > -1)
		return DatabaseManager::getSingleton()->IsBasicHTTPRequestAvailable(m_httpRequestID);
	return false;
}

bool SearchRequestCityBoroughs::GetResult(std::vector<SearchRequestResult*>& _results)
{
	std::string str;
	if (DatabaseManager::getSingleton()->GetBasicHTTPRequestResult(m_httpRequestID, str))
	{
		Json::Reader reader;
		Json::Value root;
		reader.parse(str, root);

		Json::Value& places = root["response"]["places"];
		if (places.isArray())
		{
			const int nbPlaces = places.size();
			for (int ID = 0; ID < nbPlaces; ++ID)
			{
				Json::Value val = places.get(ID, Json::nullValue);
				std::string name = val["name"].asString();
				int coma = (int)name.find_first_of(",");
				if (coma > -1)
					name = name.substr(0, coma);

				SearchRequestResulCityBorough* result = new SearchRequestResulCityBorough();
				result->m_name = name;
				_results.push_back(result);
			}
		}
		return true;
	}

	return false;
}