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
#include "Tools/moodycamel/blockingconcurrentqueue.h"

struct sStoreRequest
{
	std::string m_request;
	std::string m_result;
	bool m_finished = false;
	bool m_canceled = false;
};

class Thread;
class HTTPDownloader;
struct sRequest
{
	sRequest(int _requestID = -1, std::string _request = "")
		: m_requestID(_requestID), m_request(_request)
	{}

	void Process(HTTPDownloader* _downloader);

	int	m_requestID;
	std::string	m_request;
	std::string	m_result;
};

class HTTPDownloader
{
	friend struct sRequest;
public:
	HTTPDownloader()	{}
	~HTTPDownloader()	{}

	void Init();
	void Process();
	void End();

	int SendRequest(const std::string& _request);
	void SetResult(const int _requestID, std::string& _result);
	bool GetResult(const int _requestID, std::string& _result);
	bool IsRequestAvailable(const int _requestID) const;
	void CancelRequest(const int _requestID);

protected:
	/**
	* Download a file using HTTP GET and store in in a std::string
	* @param url The URL to download
	* @return The download result
	*/
	std::string download(const std::string& _url);

private:
	void* m_curl;
	std::map<int, sStoreRequest>	m_requests;

	std::mutex*	m_mutex;
	Thread*		m_thread;

public:
	moodycamel::ConcurrentQueue<sRequest> m_onGoingRequests;
};
