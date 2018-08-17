#include "SearchRequestCityBoroughs.h"
#include "Online\OnlineManager.h"
#include "extern/jsoncpp/reader.h"
#include "extern/jsoncpp/value.h"
#include "Tools\StringTools.h"
#include "SearchRequestResulCityBorough.h"

static const char* s_characters = "abcdefghijklmnopqrstuvwxyz123456789";
static auto s_nbCharacters = strlen(s_characters);

//---------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------
void SearchRequestCityBoroughs::Init()
{
	for (int ID = 0; ID < s_nbCharacters; ++ID)
	{
		char character = s_characters[ID];
		std::string str = "%20" + std::string((const char*)(&s_characters[ID]));
		str.resize(4);
		std::string name = m_city.m_name;
		StringTools::ReplaceBadSyntax(name, " ", "%20");
		std::string request = "https://api.meilleursagents.com/geo/v1/?q=" + name + str + "&types=arrmuns,boroughs";
		m_httpRequestsID.push_back(OnlineManager::getSingleton()->SendBasicHTTPRequest(request));
	}
}

//---------------------------------------------------------------------------------------------------------------------------------
void SearchRequestCityBoroughs::copyTo(SearchRequest* _target)
{
	SearchRequest::copyTo(_target);
	if (_target->m_requestType != SearchRequestType_CityBoroughs)
		return;

	SearchRequestCityBoroughs* target = (SearchRequestCityBoroughs*)_target;
	target->m_city = m_city;
}

//---------------------------------------------------------------------------------------------------------------------------------
bool SearchRequestCityBoroughs::IsAvailable() const
{
	bool available = true;
	int ID = 0;
	for (ID = 0; ID < m_httpRequestsID.size(); ++ID)
	{
		available &= OnlineManager::getSingleton()->IsBasicHTTPRequestAvailable(m_httpRequestsID[ID]);
		if (!available)
			break;
	}

	return available;
}

//---------------------------------------------------------------------------------------------------------------------------------
bool SearchRequestCityBoroughs::GetResult(std::vector<SearchRequestResult*>& _results)
{
	bool valid = true;
	for (int ID = 0; ID < m_httpRequestsID.size(); ++ID)
	{
		std::string str;
		if (OnlineManager::getSingleton()->GetBasicHTTPRequestResult(m_httpRequestsID[ID], str))
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

					std::string strInternalID = val["id"].asString();
					unsigned int internalID = std::stoi(strInternalID);

					StringTools::RemoveSpecialCharacters(name);

					// Search if this borough is not already in the list
					bool found = false;
					for (auto result : _results)
					{
						if (result->m_resultType == SearchRequestType_CityBoroughs)
						{
							SearchRequestResulCityBorough* borough = static_cast<SearchRequestResulCityBorough*>(result);
							if (borough->m_name == name)
								found = true;
						}
					}

					// Unknown borough => add it
					if (!found)
					{
						SearchRequestResulCityBorough* result = new SearchRequestResulCityBorough();
						result->m_name = name;
						result->m_internalID = internalID;
						_results.push_back(result);
					}
				}
			}
		}
		else
			valid = false;
	}

	return valid;
}