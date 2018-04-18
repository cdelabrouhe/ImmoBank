#include "SearchRequestCityBoroughData.h"
#include "Tools\StringTools.h"
#include "Online\OnlineManager.h"
#include "SearchRequestResulCityBoroughData.h"
#include "extern/jsoncpp/reader.h"
#include "extern/jsoncpp/value.h"

//#define TEST_HTML

//---------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------
void SearchRequestCityBoroughData::Init()
{
	//std::string request = "https://www.meilleursagents.com/prix-immobilier/montpellier-34000/quartier_antigone-170492247/"
	std::string boroughName = m_data.m_name;
	StringTools::ReplaceBadSyntax(boroughName, " ", "-");
	StringTools::ReplaceBadSyntax(boroughName, "'", "-");
	std::string request = "https://www.meilleursagents.com/prix-immobilier/" + m_city.m_name + "-" + std::to_string(m_city.m_zipCode) + "/quartier_" + boroughName + "-" + std::to_string(m_data.m_key);
	StringTools::TransformToLower(request);
#ifndef TEST_HTML
	m_httpRequestsID = OnlineManager::getSingleton()->SendBasicHTTPRequest(request);
#else
	m_httpRequestsID = 0;
#endif
}

//---------------------------------------------------------------------------------------------------------------------------------
void SearchRequestCityBoroughData::copyTo(SearchRequest* _target)
{
	SearchRequest::copyTo(_target);
	if (_target->m_requestType != SearchRequestType_CityBoroughData)
		return;

	SearchRequestCityBoroughData* target = (SearchRequestCityBoroughData*)_target;
	target->m_city = m_city;
	target->m_data = m_data;
}

//---------------------------------------------------------------------------------------------------------------------------------
bool SearchRequestCityBoroughData::IsAvailable() const
{
	if (m_httpRequestsID > -1)
#ifndef TEST_HTML
		return OnlineManager::getSingleton()->IsBasicHTTPRequestAvailable(m_httpRequestsID);
#else
		return true;
#endif

	return true;
}

//---------------------------------------------------------------------------------------------------------------------------------
bool SearchRequestCityBoroughData::GetResult(std::vector<SearchRequestResult*>& _results)
{
	bool valid = true;
	if (m_httpRequestsID > -1)
	{
		std::string str;
#ifndef TEST_HTML
		if (OnlineManager::getSingleton()->GetBasicHTTPRequestResult(m_httpRequestsID, str))
#else
		FILE* f = fopen("data_test.html", "rt");
		if (f)
		{
			char* test_data = (char*)malloc(1000000);
			fread(test_data, sizeof(char), 1000000, f);
			fclose(f);
			str = test_data;
			free(test_data);
		}
		if (true)
#endif
		{
			StringTools::RemoveEOL(str);
			std::string searchStr("MA.Context.placePrices = ");
			auto findID = str.find(searchStr);
			if (findID == std::string::npos)
			{
				// Bot behavior detected ?
				findID = str.find("behavior");
				if (findID != std::string::npos)
					return false;
			}

			std::string tmp = str.substr(findID + searchStr.size(), str.size());
			findID = tmp.find_first_of(";");
			tmp = tmp.substr(0, findID);

			Json::Reader reader;
			Json::Value root;
			reader.parse(tmp, root);

			SearchRequestResulCityBoroughData* result = new SearchRequestResulCityBoroughData();
			result->m_data = m_data;
			result->m_data.m_priceApartmentBuyMax = 1.0f;
			result->m_data.m_priceHouseBuyMin = 1.0f;
			result->m_data.m_priceHouseBuyMax = 1.0f;
			result->m_data.m_priceRentMin = 1.0f;
			result->m_data.m_priceRentMax = 1.0f;
			_results.push_back(result);
		}
		else
			valid = false;
	}

	return valid;
}