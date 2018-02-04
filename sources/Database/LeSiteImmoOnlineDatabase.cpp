#include "LeSiteImmoOnlineDatabase.h"
#include "HTTPDownloader.h"
#include "Request/SearchRequest.h"
#include "Tools/StringTools.h"

void LeSiteImmoOnlineDatabase::Init(HTTPDownloader* _downloader)
{
	m_downloader = _downloader;
	SetName("LeSiteImmo");
}

int LeSiteImmoOnlineDatabase::SendRequest(const SearchRequest& _request)
{
	return 0;
}

bool LeSiteImmoOnlineDatabase::IsRequestAvailable(const int _requestID)
{
	return true;
}

bool LeSiteImmoOnlineDatabase::GetRequestResult(const int _requestID, std::vector<SearchRequestResult>& _result)
{
	return true;
}

void LeSiteImmoOnlineDatabase::Process()
{

}

void LeSiteImmoOnlineDatabase::End()
{

}

bool LeSiteImmoOnlineDatabase::ProcessResult(SearchRequest& _initialRequest, std::string& _str, std::vector<SearchRequestResult>& _results)
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
