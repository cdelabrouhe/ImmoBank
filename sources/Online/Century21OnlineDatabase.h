#pragma once

#include "OnlineDatabase.h"

namespace ImmoBank
{
	class Century21OnlineDatabase : public OnlineDatabase
	{
	public:
		virtual void Init() override;
		virtual void Process() override;
		virtual int SendRequest(SearchRequest* _request) override;

	protected:
		virtual bool _ProcessResult(SearchRequest* _initialRequest, std::string& _str, std::vector<SearchRequestResult*>& _results) override;
	};
}