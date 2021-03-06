#pragma once

#include <vector>
#include <map>
#include <string>

#define AUTO_REFERENCE_ONLINE_DATABASE(name)\
struct AutoReferenceOnlineDatabase##name \
{ \
	AutoReferenceOnlineDatabase##name##() \
	{ \
		OnlineManager::getSingleton()->ReferenceDatabase(new name##()); \
	} \
}; \
AutoReferenceOnlineDatabase##name s_autoRefOnlineDB##name##; \


namespace ImmoBank
{
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
		int		SendBasicHTTPRequest(const std::string& _request, bool _modifyUserAgent = false);
		int		SendBinaryHTTPRequest(const std::string& _request, const std::string& _writeFilePath = "", bool _modifyUserAgent = false);
		bool	IsHTTPRequestAvailable(int _requestID) const;
		bool	GetBasicHTTPRequestResult(const int _requestID, std::string& _result);
		bool	GetBinaryHTTPRequestResult(const int _requestID, unsigned char*& _result, int& _size);
		void	CancelBasicHTTPRequest(const int _requestID);
		void	ReferenceDatabase(OnlineDatabase* _db);

		void	DisplayDebug();

		OnlineDatabase* GetOnlineDatabase(const std::string& _name) const;
		std::vector<OnlineDatabase*>& GetOnlineDatabases() { return m_databases; }

	public:
		bool	m_displayDebug = false;

	protected:
		std::vector<OnlineDatabase*>		m_databases;
		std::map<int, SearchRequest*>		m_requests;

		std::string m_testRequestResult;
		char	m_inputDebug[2048];
		int		m_testRequest = -1;
	};
}