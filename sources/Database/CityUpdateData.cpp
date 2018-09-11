#include "CityUpdateData.h"
#include "DatabaseManager.h"
#include "Request\SearchRequest\SearchRequestCityBoroughs.h"
#include "Online\OnlineManager.h"
#include "Request\SearchRequest\SearchRequestResulCityBorough.h"

//-------------------------------------------------------------------------------------------------
void CityUpdateData::Init()
{
	m_boroughsListID = DatabaseManager::getSingleton()->AskForExternalDBCityBoroughs(m_city);
}

//-------------------------------------------------------------------------------------------------
bool CityUpdateData::Process()
{
	std::vector<BoroughData>	list;
	if ((m_boroughsListID > -1) && DatabaseManager::getSingleton()->IsExternalDBCityBoroughsAvailable(m_boroughsListID, list))
	{
		for (auto& borough : list)
		{
			DatabaseManager::getSingleton()->AddBoroughData(borough);
		}

		return true;
	}
	return false;
}