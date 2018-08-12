#include "CityComputeData.h"
#include "DatabaseManager.h"
#include "Request\SearchRequest\SearchRequestCityBoroughs.h"
#include "Online\OnlineManager.h"
#include "Request\SearchRequest\SearchRequestResulCityBorough.h"

//-------------------------------------------------------------------------------------------------
void CityComputeData::Init()
{
	m_state = UpdateStep_GetCityData;

	sCityData data;
	if (DatabaseManager::getSingleton()->GetCityData(m_city, data))
	{
		SearchRequestCityBoroughs boroughs;
		boroughs.m_city = m_city;
		m_boroughsListID = OnlineManager::getSingleton()->SendRequest(&boroughs);

		m_state = UpdateStep_GetBoroughList;
	}
}

//-------------------------------------------------------------------------------------------------
bool CityComputeData::Process()
{
	switch (m_state)
	{
		// Borough list
	case UpdateStep_GetBoroughList:
		if ((m_boroughsListID > -1) && OnlineManager::getSingleton()->IsRequestAvailable(m_boroughsListID))
		{
			std::vector<SearchRequestResult*> list;
			OnlineManager::getSingleton()->GetRequestResult(m_boroughsListID, list);
			m_boroughsListID = -1;

			for (auto result : list)
			{
				if (result->m_resultType == SearchRequestType_CityBoroughs)
				{
					SearchRequestResulCityBorough* borough = static_cast<SearchRequestResulCityBorough*>(result);
					BoroughData data;
					data.m_city = m_city;
					data.m_name = borough->m_name;
					data.m_key = borough->m_internalID;
					m_boroughs.push_back(data);

					// Store data into DB
					BoroughData localData;
					if (!DatabaseManager::getSingleton()->GetBoroughData(data.m_city.m_name, data.m_name, localData))
						DatabaseManager::getSingleton()->AddBoroughData(data);

					delete borough;
				}
			}

			IncreaseStep();
		}
		break;

		// Boroughs prices
	case UpdateStep_ComputeBoroughsPrices:
	{
		if (true)
		{
			IncreaseStep();
		}
	}
	break;
	}

	return m_state == UpdateStep_COUNT;
}

//-------------------------------------------------------------------------------------------------
void CityComputeData::End()
{

}