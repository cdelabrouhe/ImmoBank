#include "HTTPDownloader.h"
#include <sstream>
#include <iostream>
#include "curl/curl.h"
#include "Tools/StringTools.h"
#include "Tools/Thread/Thread.h"

#ifdef WIN32
#ifdef _DEBUG
#pragma comment(lib, "extern/libcurl/lib/Win32/libcurld.lib")
#else
#pragma comment(lib, "extern/libcurl/lib/Win32/libcurl.lib")
#endif

#else

#ifdef _DEBUG
#pragma comment(lib, "extern/libcurl/lib/x64/libcurld.lib")
#else
#pragma comment(lib, "extern/libcurl/lib/x64/libcurl.lib")
#endif
#endif

//------------------------------------------------------------------------------------------------
unsigned int ThreadStart(void* arg)
{
	HTTPDownloader* downloader = (HTTPDownloader*)arg;
	while (true)
	{
		sRequest request;
		if (downloader->GetNextRequest(request))
			request.Process(downloader);

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	return 0;
}

//------------------------------------------------------------------------------------------------
void HTTPDownloader::Init()
{
	// Init Curl
	m_curl = curl_easy_init();

	// Init thread
	m_thread = new Thread();
	m_thread->start(ThreadStart, this, "CurlRequestsThread");
	
	m_mutex = new std::mutex();
}

//------------------------------------------------------------------------------------------------
void HTTPDownloader::Process()
{
	m_mutex->lock();

	// Search for a valid request ID
	auto it = m_requests.begin();
	while (it != m_requests.end())
	{
		if (it->second.m_canceled)
			it = m_requests.erase(it);
		else
			++it;
	}
	m_mutex->unlock();
}

//------------------------------------------------------------------------------------------------
void HTTPDownloader::End()
{
	m_thread->stop();

	curl_easy_cleanup(m_curl);

	delete m_thread;
	delete m_mutex;
}

//------------------------------------------------------------------------------------------------
int HTTPDownloader::SendRequest(const std::string& _request)
{
	m_mutex->lock();

	// Search for a valid request ID
	int ID = 0;
	while (m_requests.find(ID) != m_requests.end())
		++ID;

	// Store request
	m_requests[ID].m_request = _request;

	m_mutex->unlock();

	return ID;
}

//------------------------------------------------------------------------------------------------
void HTTPDownloader::CancelRequest(const int _requestID)
{
	m_mutex->lock();
	auto it = m_requests.find(_requestID);
	if (it != m_requests.end())
		it->second.m_canceled = true;
	m_mutex->unlock();
}

//------------------------------------------------------------------------------------------------
bool HTTPDownloader::GetNextRequest(sRequest& _request)
{
	m_mutex->lock();
	bool found = false;
	auto it = m_requests.begin();
	while (it != m_requests.end() && !found)
	{
		if (!it->second.m_canceled && !it->second.m_finished)
		{
			_request.m_requestID = it->first;
			_request.m_request = it->second.m_request;
			found = true;
		}
		++it;
	}
	m_mutex->unlock();

	return found;
}

//------------------------------------------------------------------------------------------------
void HTTPDownloader::SetResult(const int _requestID, std::string& _result)
{
	m_mutex->lock();
	auto it = m_requests.find(_requestID);
	if (it != m_requests.end())
	{
		it->second.m_result = _result;
		it->second.m_finished = true;
	}
	m_mutex->unlock();
}

//------------------------------------------------------------------------------------------------
bool HTTPDownloader::GetResult(const int _requestID, std::string& _result)
{
	bool valid = false;

	m_mutex->lock();
	auto it = m_requests.find(_requestID);
	if (it != m_requests.end())
	{
		if (it->second.m_finished)
		{
			_result = it->second.m_result;
			it->second.m_canceled = true;
			valid = true;
		}
	}
	m_mutex->unlock();
	return valid;
}

//------------------------------------------------------------------------------------------------
bool HTTPDownloader::IsRequestAvailable(const int _requestID) const
{
	bool valReturn = false;
	m_mutex->lock();
	auto it = m_requests.find(_requestID);
	if (it != m_requests.end())
		valReturn = !it->second.m_canceled && it->second.m_finished;
	m_mutex->unlock();

	return valReturn;
}

//------------------------------------------------------------------------------------------------
size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
	std::string data((const char*)ptr, (size_t)size * nmemb);
	*((std::stringstream*)stream) << data << std::endl;
	return size * nmemb;
}

//------------------------------------------------------------------------------------------------
std::string HTTPDownloader::download(const std::string& _url)
{
	const char* url = _url.c_str();
	curl_easy_setopt(m_curl, CURLOPT_URL, url);
	/* example.com is redirected, so we tell libcurl to follow redirection */
	curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, FALSE);
	curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(m_curl, CURLOPT_NOSIGNAL, 1); //Prevent "longjmp causes uninitialized stack frame" bug
	std::stringstream out;
	curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &out);
	/* Perform the request, res will get the return code */
	CURLcode res = curl_easy_perform(m_curl);
	/* Check for errors */
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
			curl_easy_strerror(res));
	}
	return out.str();
}

//------------------------------------------------------------------------------------------------
void sRequest::Process(HTTPDownloader* _downloader)
{
	m_result = _downloader->download(m_request);

	// Remove any space / tab from string
	StringTools::RemoveEOL(m_result);

	_downloader->SetResult(m_requestID, m_result);
}