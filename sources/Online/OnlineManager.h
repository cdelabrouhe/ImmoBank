#pragma once

#include <vector>
#include <map>

//-------------------------------------------------------------------------------------------------
// DATA
//-------------------------------------------------------------------------------------------------
class OnlineDatabase;
struct SearchRequest;
struct SearchRequestResult;

class OnlineManager
{
public:
	static OnlineManager* getSingleton();

	void	Init();
	void	Process();
	void	End();
	
	int		SendRequest(SearchRequest* _request);
	bool	GetRequestResult(const int _requestID, std::vector<SearchRequestResult*>& _result);

	bool	IsRequestAvailable(int _requestID) const;
	void	DeleteRequest(int _requestID);
	int		SendBasicHTTPRequest(const std::string& _request);
	bool	IsBasicHTTPRequestAvailable(int _requestID) const;
	bool	GetBasicHTTPRequestResult(const int _requestID, std::string& _result);
	void	CancelBasicHTTPRequest(const int _requestID);

	std::vector<OnlineDatabase*>& GetOnlineDatabases() { return m_databases; }

protected:
	std::vector<OnlineDatabase*>		m_databases;
	std::map<int,SearchRequest*>		m_requests;
};