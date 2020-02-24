#pragma once

#include "OnlineDatabase.h"

namespace ImmoBank
{
	class PapOnlineDatabase : public OnlineDatabase
	{
	public:
		virtual void Init() override;
		virtual int SendRequest(SearchRequest* _request) override;

	protected:
		virtual bool ProcessResult(SearchRequest* _initialRequest, std::string& _str, std::vector<SearchRequestResult*>& _results) override;
	};
}