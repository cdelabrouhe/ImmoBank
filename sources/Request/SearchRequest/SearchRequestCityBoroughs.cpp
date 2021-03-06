#include "SearchRequestCityBoroughs.h"
#include "Online\OnlineManager.h"
#include "extern/jsoncpp/reader.h"
#include "extern/jsoncpp/value.h"
#include "Tools\StringTools.h"
#include "SearchRequestResulCityBorough.h"
#include <algorithm>
#include <Online/OnlineDatabase.h>

using namespace ImmoBank;

static const char* s_characters = "abcdefghijklmnopqrstuvwxyz123456789";
static auto s_nbCharacters = strlen(s_characters);

//---------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------
void SearchRequestCityBoroughs::Init()
{
	SwitchState(State_GetRawList);
}

//---------------------------------------------------------------------------------------------------------------------------------
void SearchRequestCityBoroughs::SwitchState(State _state)
{
	if (m_state == _state)
		return;

	m_state = _state;
	switch (m_state)
	{
	case State_GetRawList:
	{
		for (int ID = 0; ID < s_nbCharacters; ++ID)
		{
			char character = s_characters[ID];
			std::string str = "%20" + std::string((const char*)(&character));
			str.resize(4);
			std::string name = m_city.m_name;
			StringTools::ReplaceBadSyntax(name, " ", "%20");
			std::string request = "https://geo.meilleursagents.com/geo/v1/?q=" + name + str + "&types=arrmuns,boroughs";
			m_httpRequestsID.push_back(OnlineManager::getSingleton()->SendBasicHTTPRequest(request));
		}

		auto& dbs = OnlineManager::getSingleton()->GetOnlineDatabases();
		for (auto* db : dbs)
		{
			db->ReferenceCity(m_city.m_name);
		}
		break;
	}
	default:
		break;
	}
}

//---------------------------------------------------------------------------------------------------------------------------------
void SearchRequestCityBoroughs::Process()
{
	switch (m_state)
	{
		case State_GetRawList:
		{
			bool available = true;
			int ID = 0;
			for (ID = 0; ID < m_httpRequestsID.size(); ++ID)
			{
				available &= OnlineManager::getSingleton()->IsHTTPRequestAvailable(m_httpRequestsID[ID]);
				if (!available)
					break;
			}

			if (available)
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
								for (auto result : m_boroughs)
								{
									if (result.m_resultType == SearchRequestType_CityBoroughs)
									{
										SearchRequestResulCityBorough& borough = static_cast<SearchRequestResulCityBorough&>(result);
										if (borough.m_name == name)
											found = true;
									}
								}

								// Unknown borough => add it
								if (!found)
								{
									SearchRequestResulCityBorough result;
									result.m_name = name;
									result.m_internalID = internalID;
									m_boroughs.push_back(result);
								}
							}
						}
					}
					else
						valid = false;
				}

				SwitchState(State_DONE);
			}
		}
		break;

		default:
			break;
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
	target->m_boroughs = m_boroughs;
}

//---------------------------------------------------------------------------------------------------------------------------------
bool SearchRequestCityBoroughs::IsAvailable() const
{
	return m_state == State_DONE;
}

//---------------------------------------------------------------------------------------------------------------------------------
bool SearchRequestCityBoroughs::GetResult(std::vector<SearchRequestResult*>& _results)
{
	for (auto& borough : m_boroughs)
	{
		SearchRequestResulCityBorough* result = new SearchRequestResulCityBorough();
		result->m_name = borough.m_name;
		result->m_internalID = borough.m_internalID;
		_results.push_back(result);
	}

	return true;
}