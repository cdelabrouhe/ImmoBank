#pragma once

#include "OnlineDatabase.h"

namespace ImmoBank
{
	class Century21OnlineDatabase : public OnlineDatabase
	{
	public:
		virtual void Init() override;
		virtual int SendRequest(SearchRequest* _request) override;

		virtual bool HasKey() { return true; }

		std::string GetKey(BoroughData& _borough) const;
		virtual std::string GetKeyAsString(BoroughData& _borough) const override;

	protected:
		virtual bool _ProcessResult(SearchRequest* _initialRequest, std::string& _str, std::vector<SearchRequestResult*>& _results) override;
	};
}