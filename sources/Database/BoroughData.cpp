#include "BoroughData.h"
#include <vector>
#include "Online\OnlineManager.h"
#include "Request\SearchRequest\SearchRequestCityBoroughData.h"
#include "Request\SearchRequest\SearchRequestResulCityBoroughData.h"

//-------------------------------------------------------------------------------------------------
void BoroughData::Init()
{
	SearchRequestCityBoroughData request;
	request.m_data = *this;
	request.m_city = m_city;
	m_httpRequestID = OnlineManager::getSingleton()->SendRequest(&request);
}

//-------------------------------------------------------------------------------------------------
bool BoroughData::Process()
{
	if (OnlineManager::getSingleton()->IsRequestAvailable(m_httpRequestID))
	{
		std::vector<SearchRequestResult*> list;
		if (OnlineManager::getSingleton()->GetRequestResult(m_httpRequestID, list))
		{
			m_httpRequestID = -1;

			for (auto result : list)
			{
				if (result->m_resultType == SearchRequestType_CityBoroughData)
				{
					SearchRequestResulCityBoroughData* borough = static_cast<SearchRequestResulCityBoroughData*>(result);
					DatabaseManager::getSingleton()->AddBoroughData(borough->m_data);
				}
			}
		}

		return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
void BoroughData::End()
{

}

//-------------------------------------------------------------------------------------------------
bool BoroughData::IsWholeCity() const
{
	return m_name == s_wholeCityName;
}