#pragma once

#include "OnlineDatabase.h"

namespace ImmoBank
{
	class PapOnlineDatabase : public OnlineDatabase
	{
	public:
		virtual void Init() override;
		virtual void Process() override;
		virtual int SendRequest(SearchRequest* _request) override;

		virtual void ReferenceCity(const std::string& _name) override;
		virtual void ReferenceBorough(const BoroughData& _borough) override;
		virtual bool HasCity(const std::string& _name, const int _zipCode, sCity& _city) override;

	protected:
		virtual bool ProcessResult(SearchRequest* _initialRequest, std::string& _str, std::vector<SearchRequestResult*>& _results) override;

		std::string ComputeKeyURL(const std::string& _name);

	private:
		std::map<std::pair<std::string, int>, unsigned int>	m_keys;
		int		m_currentKeyID = -1;
	};
}