#include "Tools.h"
#include <Tools/StringTools.h>
#include "extern/jsoncpp/reader.h"
#include <Database/BoroughData.h>

float Tools::ComputeRentabilityRate(float _rent, float _price)
{
	return _rent * 12.f * 100.f / _price;
}

bool Tools::ExtractPricesFromHTMLSource(const std::string& _source, sPrice& _rentT1, sPrice& _rentT2, sPrice& _rentT3, sPrice& _rentT4Plus, sPrice& _buyApartment, sPrice& _buyHouse)
{
	std::string searchStr("MA.Context.placePrices = ");
	auto findID = _source.find(searchStr);
	if (findID == std::string::npos)
	{
		// Bot behavior detected ?
		findID = _source.find("behavior");
		if (findID != std::string::npos)
		{
			printf("ERROR: can't retrieve borough information, too many requests and website banned us\n");
			/*static bool s_test = false;
			if (s_test)
			{
				FILE* f = fopen("error.html", "wt");
				if (f)
				{
					fwrite(_source.data(), sizeof(char), (size_t)_source.size(), f);
					fclose(f);
				}
			}
			return false;*/
			return true;
		}
		else
		{
			findID = _source.find("Ooops");
			if (findID != std::string::npos)
			{
				printf("ERROR: bad request, maybe a bad city name ?\n");
				return false;
			}
			else
			{
				printf("ERROR: bad request, bad copy / paste ?\n");
				return false;
			}
		}
	}

	std::string tmp = _source.substr(findID + searchStr.size(), _source.size());
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
	_rentT1 = sPrice((float)valRentT1["value"].asDouble(), (float)valRentT1["low"].asDouble(), (float)valRentT1["high"].asDouble());
	_rentT2 = sPrice((float)valRentT2["value"].asDouble(), (float)valRentT2["low"].asDouble(), (float)valRentT2["high"].asDouble());
	_rentT3 = sPrice((float)valRentT3["value"].asDouble(), (float)valRentT3["low"].asDouble(), (float)valRentT3["high"].asDouble());
	_rentT4Plus = sPrice((float)valRentT4["value"].asDouble(), (float)valRentT4["low"].asDouble(), (float)valRentT4["high"].asDouble());
	Json::Value& sellApartment = root["sell"]["apartment"];
	Json::Value& sellHouse = root["sell"]["house"];
	_buyApartment = sPrice((float)sellApartment["value"].asDouble(), (float)sellApartment["low"].asDouble(), (float)sellApartment["high"].asDouble());
	_buyHouse = sPrice((float)sellHouse["value"].asDouble(), (float)sellHouse["low"].asDouble(), (float)sellHouse["high"].asDouble());

	return true;
}