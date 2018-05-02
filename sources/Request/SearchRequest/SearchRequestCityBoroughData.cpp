#include "SearchRequestCityBoroughData.h"
#include "Tools\StringTools.h"
#include "Online\OnlineManager.h"
#include "SearchRequestResulCityBoroughData.h"
#include "extern/jsoncpp/reader.h"
#include "extern/jsoncpp/value.h"

#define TEST_HTML

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

			StringTools::RemoveEOL(tmp);

			Json::Reader reader;
			Json::Value root;
			reader.parse(tmp, root);
			Json::Value& rental = root["rental"]["apartment"];
			Json::Value& valRentT1 = rental["t1"];
			Json::Value& valRentT2 = rental["t2"];
			Json::Value& valRentT3 = rental["t3"];
			Json::Value& valRentT4 = rental["t4_plus"];
			sPrice rentT1(valRentT1["value"].asDouble(), valRentT1["low"].asDouble(), valRentT1["high"].asDouble());
			sPrice rentT2(valRentT2["value"].asDouble(), valRentT2["low"].asDouble(), valRentT2["high"].asDouble());
			sPrice rentT3(valRentT3["value"].asDouble(), valRentT3["low"].asDouble(), valRentT3["high"].asDouble());
			sPrice rentT4(valRentT4["value"].asDouble(), valRentT4["low"].asDouble(), valRentT4["high"].asDouble());
			Json::Value& sellApartment = root["sell"]["apartment"];
			Json::Value& sellHouse = root["sell"]["house"];
			sPrice buyApartment(sellApartment["value"].asDouble(), sellApartment["low"].asDouble(), sellApartment["high"].asDouble());
			sPrice buyHouse(sellHouse["value"].asDouble(), sellHouse["low"].asDouble(), sellHouse["high"].asDouble());

			SearchRequestResulCityBoroughData* result = new SearchRequestResulCityBoroughData();
			result->m_data = m_data;
			result->m_data.m_priceRentApartmentT1 = rentT1;
			result->m_data.m_priceRentApartmentT2 = rentT2;
			result->m_data.m_priceRentApartmentT3 = rentT3;
			result->m_data.m_priceRentApartmentT4Plus = rentT4;
			result->m_data.m_priceBuyApartment = buyApartment;
			result->m_data.m_priceBuyHouse = buyHouse;
			_results.push_back(result);
		}
		else
			valid = false;
	}

	return valid;
}