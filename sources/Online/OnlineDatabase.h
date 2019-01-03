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
		virtual void Process() = 0;
		virtual int SendRequest(SearchRequest* _request) = 0;
		virtual bool IsRequestAvailable(int _requestID) = 0;
		virtual bool GetRequestResult(int _requestID, std::vector<SearchRequestResult*>& _result) = 0;
		virtual void End() = 0;

		void SetName(const std::string& _name) { m_name = _name; }
		const std::string&	 GetName() const { return m_name; }

		void DeleteRequest(int _requestID)
		{
			auto it = m_requests.find(_requestID);
			if (it != m_requests.end())
				m_requests.erase(it);
		}

	protected:
		virtual bool ProcessResult(SearchRequest* _initialRequest, std::string& _str, std::vector<SearchRequestResult*>& _results) = 0;

	protected:
		std::string			m_name;

		struct sRequest
		{
			int				m_requestID;
			SearchRequest*	m_initialRequest;
		};
		std::map<int, sRequest>	m_requests;
	};
}