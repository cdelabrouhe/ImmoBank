#pragma once

#include <vector>
#include <map>

//-------------------------------------------------------------------------------------------------
// DATA
//-------------------------------------------------------------------------------------------------
class OnlineDatabase;
struct SearchRequest;
struct SearchRequestResult;

struct sInternalSearchRequest
{
public:
	bool IsAvailable() const;
	bool GetResult(std::vector<SearchRequestResult>& _results);
	void End();

public:
	std::vector<std::pair<OnlineDatabase*, int>>	m_internalRequests;
};

class DatabaseManager
{
public:
	static DatabaseManager* getSingleton();

	void	Init();
	void	Process();
	void	End();
	
	int		SendRequest(const SearchRequest& _request);
	bool	GetRequestResult(const int _requestID, std::vector<SearchRequestResult>& _result);

	bool	IsRequestAvailable(int _requestID) const;
	void	DeleteRequest(int _requestID);
	int		SendBasicHTTPRequest(const std::string& _request);
	bool	IsBasicHTTPRequestAvailable(int _requestID) const;
	bool	GetBasicHTTPRequestResult(const int _requestID, std::string& _result);
	void	CancelBasicHTTPRequest(const int _requestID);

protected:
	std::vector<OnlineDatabase*>			m_databases;
	std::map<int,sInternalSearchRequest>	m_requests;
};