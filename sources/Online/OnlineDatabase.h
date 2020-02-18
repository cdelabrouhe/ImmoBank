#pragma once

#include <string>
#include <map>
#include <vector>
#include "Request/SearchRequest/SearchRequest.h"

namespace ImmoBank
{
	struct SearchRequest;
	struct SearchRequestResult;

	class OnlineDatabase
	{
	public:
		virtual void Init() = 0;
		virtual void Process()	{}
		virtual void End()		{}

		virtual bool IsRequestAvailable(int _requestID);
		virtual bool GetRequestResult(int _requestID, std::vector<SearchRequestResult*>& _result);

		virtual int SendRequest(SearchRequest* _request) = 0;

		void SetName(const std::string& _name) { m_name = _name; }
		const std::string&	 GetName() const { return m_name; }

		void DeleteRequest(int _requestID);

	protected:
		virtual bool ProcessResult(SearchRequest* _initialRequest, std::string& _str, std::vector<SearchRequestResult*>& _results) = 0;

	protected:
		std::string			m_name;

		struct sRequest
		{
			int				m_requestID;
			SearchRequest*	m_initialRequest;
			std::string		m_request;
		};
		std::map<int, sRequest>	m_requests;
	};
}