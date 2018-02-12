#pragma once

#include <vector>

class Request;
struct SearchRequestAnnounce;

class RequestManager
{
public:
	static RequestManager* getSingleton();

	struct sRequest
	{
		sRequest(Request* _request = nullptr) : m_request(_request)	{}
		Request*	m_request = nullptr;
		bool		m_deleted = false;
	};

	void Init();
	void Process();
	void End();

	void DisplayRequests();

	Request* CreateRequest(SearchRequestAnnounce& _request);
	Request* CreateDefaultRequest();

	void AskForDeleteRequest(Request* _request);

private:
	void DeleteRequest(Request* _request);
	sRequest* getRequest(Request* _request);

private:
	std::vector<sRequest>	m_requests;
};