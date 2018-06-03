#include "LeSiteImmoOnlineDatabase.h"
#include "Request/SearchRequest/SearchRequest.h"
#include "Tools/StringTools.h"

void LeSiteImmoOnlineDatabase::Init()
{
	SetName("LeSiteImmo");
}

int LeSiteImmoOnlineDatabase::SendRequest(SearchRequest* _request)
{
	return 0;
}

bool LeSiteImmoOnlineDatabase::IsRequestAvailable(const int _requestID)
{
	return true;
}

bool LeSiteImmoOnlineDatabase::GetRequestResult(const int _requestID, std::vector<SearchRequestResult*>& _result)
{
	return true;
}

void LeSiteImmoOnlineDatabase::Process()
{

}

void LeSiteImmoOnlineDatabase::End()
{

}

bool LeSiteImmoOnlineDatabase::ProcessResult(SearchRequest* _initialRequest, std::string& _str, std::vector<SearchRequestResult*>& _results)
{
	return true;
}

void LeSiteImmoOnlineDatabase::sRecherche::Serialize(const std::string& _str)
{
}

void LeSiteImmoOnlineDatabase::sSummary::Serialize(const std::string& _str)
{
}

void LeSiteImmoOnlineDatabase::sAnnonce::Serialize(const std::string& _str)
{
}
