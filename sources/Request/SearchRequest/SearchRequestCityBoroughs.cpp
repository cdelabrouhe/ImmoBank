#include "SearchRequestCityBoroughs.h"
#include "Online\OnlineManager.h"
#include "extern/jsoncpp/reader.h"
#include "extern/jsoncpp/value.h"
#include "Tools\StringTools.h"
#include "SearchRequestResulCityBorough.h"
#include <algorithm>

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
		break;
	}
	case State_CheckSeLoger:
	{
		m_httpRequestsID.clear();
		for (auto& borough : m_boroughs)
		{
			BoroughData data;
			data.m_name = borough.m_name;
			std::string request = data.ComputeSeLogerKeyURL();
			m_httpRequestsID.push_back(OnlineManager::getSingleton()->SendBasicHTTPRequest(request));
		}
		break;
	}
	case State_CheckLogicImmo:
	{
		// No boroughs on LogicImmo => set city key
		for (auto& borough : m_boroughs)
			borough.m_logicImmoID = m_city.m_logicImmoKey;

		break;
	}
	case State_CheckPap:
	{
		// No boroughs on Pap => set city key
		for (auto& borough : m_boroughs)
			borough.m_papKeyID = m_city.m_papKey;

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

				SwitchState(State_CheckLogicImmo);
			}
		}
		break;

		case State_CheckSeLoger:
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
				std::string cityName = m_city.m_name;
				StringTools::TransformToLower(cityName);

				std::vector<SearchRequestResulCityBorough>	resultBoroughs;

				bool valid = true;
				for (int ID = 0; ID < m_httpRequestsID.size(); ++ID)
				{
					std::string str;
					if (OnlineManager::getSingleton()->GetBasicHTTPRequestResult(m_httpRequestsID[ID], str))
					{
						Json::Reader reader;
						Json::Value root;
						reader.parse(str, root);

						Json::Value& places = root;
						if (places.isArray())
						{
							const int nbPlaces = places.size();
							for (int placeID = 0; placeID < nbPlaces; ++placeID)
							{
								Json::Value val = places.get(placeID, Json::nullValue);
								std::string type = val["Type"].asString();
								std::string name = val["Display"].asString();
								wchar_t wStr[1024];
								mbstowcs(wStr, name.c_str(), 1024);
								std::wstring wName = wStr;
								const bool isNeighboor = type == "Quartier";
								const bool isCity = type == "Ville";
								if (isCity && (std::find_if(wName.begin(), wName.end(), ::isdigit) == wName.end()))
									continue;

								std::string tmp = name;
								StringTools::TransformToLower(tmp);
								auto findID = tmp.find(cityName);
								if (findID == std::string::npos)
									continue;

								std::string strIndexID = isCity ? val["Params"]["ci"].asString() : val["Params"]["idq"].asString();
								unsigned int index = std::stoi(strIndexID);
								index = BoroughData::ConvertSelogerKey(index, isCity);

								SearchRequestResulCityBorough& borough = m_boroughs[ID];
								borough.m_selogerID = index;
								resultBoroughs.push_back(borough);
							}
						}
					}
					else
						valid = false;
				}

				if (valid)
				{
					m_boroughs.clear();
					for (auto& borough : resultBoroughs)
						m_boroughs.push_back(borough);

					SwitchState(State_CheckLogicImmo);
				}
			}
		}
		break;

		case State_CheckLogicImmo:
		{			
			SwitchState(State_CheckPap);
			break;
		}

		case State_CheckPap:
		{
			SwitchState(State_DONE);
			break;
		}

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
		result->m_selogerID = borough.m_selogerID;
		result->m_logicImmoID = borough.m_logicImmoID;
		result->m_papKeyID = borough.m_papKeyID;
		_results.push_back(result);
	}

	return true;
}