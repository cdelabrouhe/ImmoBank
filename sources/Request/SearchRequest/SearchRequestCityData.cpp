#include "SearchRequestCityData.h"
#include "SearchRequestCityBoroughs.h"
#include "Online\OnlineManager.h"
#include "SearchRequestResulCityBorough.h"
#include "SearchRequestResulCityBoroughData.h"
#include <algorithm>
#include "SearchRequestCityBoroughData.h"

//---------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------
void SearchRequestCityData::Init()
{
	// Get boroughs (if they exist) from Database
	if (!DatabaseManager::getSingleton()->GetBoroughs(m_city.m_name, m_boroughs))
	{
		m_state = UpdateStep_GetBoroughList;

		SearchRequestCityBoroughs boroughs;
		boroughs.m_city = m_city;
		m_boroughsRequestID = OnlineManager::getSingleton()->SendRequest(&boroughs);
	}
	else
	{
		InitBoroughPricesRequest();
	}
}

//---------------------------------------------------------------------------------------------------------------------------------
void SearchRequestCityData::InitBoroughPricesRequest()
{
	if (m_boroughs.size() == 0)
	{
		m_state = UpdateStep_COUNT;
		return;
	}

	if (m_boroughs[0].m_priceApartmentBuyMax == 0)
	{
		m_state = UpdateStep_ComputeBoroughsPrices;

		for (auto ID = 0; ID < m_boroughs.size(); ++ID)
		{
			SearchRequestCityBoroughData data;
			data.m_data = m_boroughs[ID];
			data.m_city = m_city;
			m_httpRequestsID.push_back(OnlineManager::getSingleton()->SendRequest(&data));
		}
	}
	else
		m_state = UpdateStep_COUNT;
}

//---------------------------------------------------------------------------------------------------------------------------------
void SearchRequestCityData::Process()
{
	switch (m_state)
	{
		// Borough list
	case UpdateStep_GetBoroughList:
		if ((m_boroughsRequestID > -1) && OnlineManager::getSingleton()->IsRequestAvailable(m_boroughsRequestID))
		{
			std::vector<SearchRequestResult*> list;
			OnlineManager::getSingleton()->GetRequestResult(m_boroughsRequestID, list);
			m_boroughsRequestID = -1;

			for (auto result : list)
			{
				if (result->m_resultType == SearchRequestType_CityBoroughs)
				{
					SearchRequestResulCityBorough* borough = static_cast<SearchRequestResulCityBorough*>(result);
					sBoroughData data;
					data.m_cityName = m_city.m_name;
					data.m_name = borough->m_name;
					data.m_key = borough->m_internalID;
					m_boroughs.push_back(data);

					// Store data into DB
					DatabaseManager::getSingleton()->AddBoroughData(data);
					delete borough;
				}
			}

			InitBoroughPricesRequest();
		}
		break;

		// Boroughs prices
	case UpdateStep_ComputeBoroughsPrices:
	{
		bool available = true;
		int ID = 0;
		for (ID = 0; ID < m_httpRequestsID.size(); ++ID)
		{
			available &= OnlineManager::getSingleton()->IsRequestAvailable(m_httpRequestsID[ID]);
			if (!available)
				break;
		}

		if (available)
		{
			bool valid = true;
			for (int ID = 0; ID < m_httpRequestsID.size(); ++ID)
			{
				std::vector<SearchRequestResult*> list;
				if (OnlineManager::getSingleton()->GetRequestResult(m_httpRequestsID[ID], list))
				{
					m_boroughsRequestID = -1;

					for (auto result : list)
					{
						if (result->m_resultType == SearchRequestType_CityBoroughData)
						{
							SearchRequestResulCityBoroughData* borough = static_cast<SearchRequestResulCityBoroughData*>(result);
							sBoroughData data = borough->m_data;
							auto it = std::find_if(m_boroughs.begin(), m_boroughs.end(), [data](sBoroughData& _data)->bool { return _data.m_name == data.m_name; });
							if (it != m_boroughs.end())
							{
								*it = data;

								// Store data into DB
								DatabaseManager::getSingleton()->AddBoroughData(data);
							}
						}
					}
				}
			}

			IncreaseStep();
		}
	}
	break;
	}
}

//---------------------------------------------------------------------------------------------------------------------------------
void SearchRequestCityData::End()
{

}

//---------------------------------------------------------------------------------------------------------------------------------
void SearchRequestCityData::copyTo(SearchRequest* _target)
{
	SearchRequest::copyTo(_target);
	if (_target->m_requestType != SearchRequestType_CityData)
		return;

	SearchRequestCityData* target = (SearchRequestCityData*)_target;
	target->m_city = m_city;
}

//---------------------------------------------------------------------------------------------------------------------------------
bool SearchRequestCityData::IsAvailable() const
{
	return m_state == UpdateStep_COUNT;
}

//---------------------------------------------------------------------------------------------------------------------------------
bool SearchRequestCityData::GetResult(std::vector<SearchRequestResult*>& _results)
{
	if (!IsAvailable())
		return false;

	if (m_boroughsRequestID > -1)
		return false;

	bool valid = true;
	return valid;
}