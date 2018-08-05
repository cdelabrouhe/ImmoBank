#include "SearchRequestCityBoroughData.h"
#include "Tools\StringTools.h"
#include "Online\OnlineManager.h"
#include "SearchRequestResulCityBoroughData.h"
#include "extern/jsoncpp/reader.h"
#include "extern/jsoncpp/value.h"
#include <Tools\Tools.h>

//#define TEST_HTML

//---------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------
void SearchRequestCityBoroughData::Init()
{
	//std::string request = "https://www.meilleursagents.com/prix-immobilier/montpellier-34000/quartier_antigone-170492247/"
	std::string boroughName = m_data.m_name;
	StringTools::ReplaceBadSyntax(boroughName, " / ", "-");
	StringTools::ReplaceBadSyntax(boroughName, " \\ ", "-");
	StringTools::ReplaceBadSyntax(boroughName, " ", "-");
	StringTools::ReplaceBadSyntax(boroughName, "'", "-");
	std::string zipCode = std::to_string(m_city.m_zipCode);
	while (zipCode.size() < 5)
		zipCode = "0" + zipCode;

	StringTools::ConvertToImGuiText(m_city.m_name);
	std::string request = "https://www.meilleursagents.com/prix-immobilier/" + m_city.m_name + "-" + zipCode + "/";
	if (!m_data.IsWholeCity())
		request += "quartier_" + boroughName + "-" + std::to_string(m_data.m_key);
	StringTools::TransformToLower(request);
#ifndef TEST_HTML
	m_httpRequestsID = OnlineManager::getSingleton()->SendBasicHTTPRequest(request);// , true);
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
			sPrice rentT1;
			sPrice rentT2;
			sPrice rentT3;
			sPrice rentT4;
			sPrice buyApartment;
			sPrice buyHouse;
			Tools::ExtractPricesFromHTMLSource(str, rentT1, rentT2, rentT3, rentT4, buyApartment, buyHouse);
			/*std::string searchStr("MA.Context.placePrices = ");
			auto findID = str.find(searchStr);
			if (findID == std::string::npos)
			{
				// Bot behavior detected ?
				findID = str.find("behavior");
				if (findID != std::string::npos)
				{
// 					printf("ERROR: can't retrive borough information, too many requests and website banned us\n");
// 					static bool s_test = false;
// 					if (s_test)
// 					{
// 						FILE* f = fopen("error.html", "wt");
// 						if (f)
// 						{
// 							fwrite(str.data(), sizeof(char), (size_t)str.size(), f);
// 							fclose(f);
// 						}
// 					}
// 					return false;
					return true;
				}
				else
				{
					findID = str.find("Ooops");
					if (findID != std::string::npos)
					{
						printf("ERROR: bad request, maybe a bad city name ?\n");
					}
				}
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
			sPrice rentT1((float)valRentT1["value"].asDouble(), (float)valRentT1["low"].asDouble(), (float)valRentT1["high"].asDouble());
			sPrice rentT2((float)valRentT2["value"].asDouble(), (float)valRentT2["low"].asDouble(), (float)valRentT2["high"].asDouble());
			sPrice rentT3((float)valRentT3["value"].asDouble(), (float)valRentT3["low"].asDouble(), (float)valRentT3["high"].asDouble());
			sPrice rentT4((float)valRentT4["value"].asDouble(), (float)valRentT4["low"].asDouble(), (float)valRentT4["high"].asDouble());
			Json::Value& sellApartment = root["sell"]["apartment"];
			Json::Value& sellHouse = root["sell"]["house"];
			sPrice buyApartment((float)sellApartment["value"].asDouble(), (float)sellApartment["low"].asDouble(), (float)sellApartment["high"].asDouble());
			sPrice buyHouse((float)sellHouse["value"].asDouble(), (float)sellHouse["low"].asDouble(), (float)sellHouse["high"].asDouble());*/

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