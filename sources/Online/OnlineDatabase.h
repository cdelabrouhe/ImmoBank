#pragma once

#include <string>
#include <map>
#include <vector>
#include <Request/SearchRequest/SearchRequest.h>
#include <Tools/DatabaseHelper.h>

namespace ImmoBank
{
	struct SearchRequest;
	struct SearchRequestResult;

	class OnlineDatabase : public DatabaseHelper
	{
	public:
		struct sBoroughData
		{
			sBoroughData() {}
			sBoroughData(BoroughData& _data, const std::string& _request, int _requestID = -1) : m_data(_data), m_request(_request), m_requestID(_requestID) {}
			BoroughData	m_data;
			std::string m_request;
			int m_requestID = -1;
		};

	public:
		virtual void Init() = 0;
		virtual void Process();
		virtual void End()		{}

		virtual bool IsRequestAvailable(int _requestID);
		virtual bool GetRequestResult(int _requestID, std::vector<SearchRequestResult*>& _result);

		virtual int SendRequest(SearchRequest* _request) = 0;

		void SetName(const std::string& _name) { m_name = _name; }
		const std::string&	 GetName() const { return m_name; }

		void DeleteRequest(int _requestID);

		virtual void ReferenceCity(const std::string& _name) {}
		virtual void ReferenceBorough(const BoroughData& _borough)	{}
		virtual bool HasCity(const std::string& _name, const int _zipCode) { return true; }

		virtual bool HasKey() { return false; }

		virtual std::string GetKeyAsString(sCity& _city) const	{ return ""; }

		bool* ForceUpdate();

	protected:
		virtual bool _ProcessResult(SearchRequest* _initialRequest, std::string& _str, std::vector<SearchRequestResult*>& _results) = 0;

		virtual std::string _ComputeKeyURL(const std::string& _name) { return ""; }
		virtual void _DecodeData(const std::string& _data, const sBoroughData& _sourceBorough) {}

	private:
		void _ProcessForceUpdate();

	public:
		bool	m_used = true;

	protected:
		std::string			m_name;


		struct sRequest
		{
			int				m_requestID;
			SearchRequest*	m_initialRequest;
			std::string		m_request;
		};
		std::map<int, sRequest>	m_requests;

		std::vector<sBoroughData>	m_boroughData;
		int		m_intervalBetweenRequests = 0;
		int		m_timer = 1;
		bool	m_forceUpdateInProgress = false;
		bool	m_forceUpdateInitialized = false;
	};
}