/**
* HTTPDownloader.hpp
*
* A simple C++ wrapper for the libcurl easy API.
*
* Written by Uli Köhler (techoverflow.net)
* Published under CC0 1.0 Universal (public domain)
*/
#pragma once

#include <string>
#include <map>
#include <mutex>

namespace ImmoBank
{
	enum RequestResultType
	{
		RequestResultType_NONE = -1,
		RequestResultType_String,
		RequestResultType_Binary,
		RequestResultType_COUNT
	};

	struct sStoreRequest
	{
		void Clear() { if (m_resultBinary) free(m_resultBinary); }

		std::string m_request;
		std::string m_result;
		char*		m_resultBinary = nullptr;
		int			m_resultBinarySize = 0;
		RequestResultType		m_type = RequestResultType_NONE;
		bool m_finished = false;
		bool m_canceled = false;
		bool m_protectUserAgent = false;
	};

	class Thread;
	class HTTPDownloader;
	struct sRequest
	{
		sRequest(RequestResultType _type = RequestResultType_NONE, int _requestID = -1, std::string _request = "")
			: m_requestID(_requestID), m_request(_request)
		{}

		void Process(HTTPDownloader* _downloader);

		RequestResultType		m_type = RequestResultType_NONE;
		int			m_requestID;
		std::string	m_request;
		std::string	m_result;
		char*		m_resultBinary = nullptr;
		int			m_resultBinarySize = 0;
		bool		m_protectUserAgent = false;
	};

	class HTTPDownloader
	{
		friend struct sRequest;
	public:
		HTTPDownloader() {}
		~HTTPDownloader() {}

		void Init();
		void Process();
		void End();

		int SendRequest(const std::string& _request, RequestResultType _resultType, bool _modifyUserAgent);
		void SetResult(const int _requestID, sRequest& _result);
		bool GetResultString(const int _requestID, std::string& _result);
		bool GetResultBinary(const int _requestID, unsigned char*& _result, int& _size);
		bool IsRequestAvailable(const int _requestID) const;
		void CancelRequest(const int _requestID);

		bool GetNextRequest(sRequest& _request);

		size_t GetNbRequests() const	{ return m_requests.size(); }

	protected:
		/**
		* Download a file using HTTP GET and store in in a std::string
		* @param url The URL to download
		* @return The download result
		*/
		void download(const std::string& _url, bool _modifyUserAgent, std::stringstream& _out);

	private:
		void* m_curl;
		std::map<int, sStoreRequest>	m_requests;

		std::mutex*	m_mutex;
		Thread*		m_thread;
	};
}