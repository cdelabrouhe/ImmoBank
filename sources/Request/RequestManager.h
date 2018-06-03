#pragma once

#include <vector>

class EditableRequest;
struct SearchRequest;

class RequestManager
{
public:
	static RequestManager* getSingleton();

	struct sRequest
	{
		sRequest(EditableRequest* _request = nullptr) : m_request(_request)	{}
		EditableRequest*	m_request = nullptr;
		bool		m_deleted = false;
	};

	void Init();
	void Process();
	void End();

	void DisplayRequests();

	EditableRequest* CreateRequest(SearchRequest* _request);
	EditableRequest* CreateRequestAnnounceDefault();

	void AskForDeleteRequest(EditableRequest* _request);

private:
	void DeleteRequest(EditableRequest* _request);
	sRequest* getRequest(EditableRequest* _request);

private:
	std::vector<sRequest>	m_requests;
};