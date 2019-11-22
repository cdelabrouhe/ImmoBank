#include "HTTPDownloader.h"
#include <sstream>
#include <iostream>
#include "curl/curl.h"
#include "Tools/StringTools.h"
#include "Tools/Thread/Thread.h"
#include <Tools/Types.h>

using namespace ImmoBank;

static const char* s_curlUserAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/60.0.3112.113 Safari/537.36";

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
		{
			request.Process(downloader);
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		else
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
		{
			it->second.End();
			it = m_requests.erase(it);
		}
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
int HTTPDownloader::SendRequest(const std::string& _request, RequestResultType _resultType, bool _modifyUserAgent, const std::string& _writeFilePath)
{
	m_mutex->lock();

	// Search for a valid request ID
	int ID = 0;
	while (m_requests.find(ID) != m_requests.end())
		++ID;

	// Store request
	sStoreRequest& request = m_requests[ID];
	request.m_type = _resultType;
	request.m_request = _request;
	request.m_protectUserAgent = _modifyUserAgent;
	request.m_writeFilePath = _writeFilePath;

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
			_request.m_protectUserAgent = it->second.m_protectUserAgent;
			_request.m_type = it->second.m_type;
			found = true;
		}
		++it;
	}
	m_mutex->unlock();

	return found;
}

//------------------------------------------------------------------------------------------------
void HTTPDownloader::SetResult(const int _requestID, sRequest& _result)
{
	m_mutex->lock();
	auto it = m_requests.find(_requestID);
	if (it != m_requests.end())
	{
		it->second.m_result = _result.m_result;
		it->second.m_finished = true;
		if (_result.m_resultBinarySize > 0)
		{
			it->second.m_resultBinary.m_memory = (char*)malloc(_result.m_resultBinary.m_size);
			it->second.m_resultBinarySize = _result.m_resultBinarySize;
			memcpy(it->second.m_resultBinary.m_memory, _result.m_resultBinary.m_memory, _result.m_resultBinarySize);
		}
	}
	m_mutex->unlock();
}

//------------------------------------------------------------------------------------------------
bool HTTPDownloader::GetResultString(const int _requestID, std::string& _result)
{
	bool valid = false;

	m_mutex->lock();
	auto it = m_requests.find(_requestID);
	if (it != m_requests.end())
	{
		sStoreRequest& request = it->second;
		if (request.m_finished)
		{
			_result = request.m_result;
			request.m_canceled = true;
			valid = true;
		}
	}
	m_mutex->unlock();
	return valid;
}

//------------------------------------------------------------------------------------------------
bool HTTPDownloader::GetResultBinary(const int _requestID, unsigned char*& _result, int& _size)
{
	bool valid = false;

	m_mutex->lock();
	auto it = m_requests.find(_requestID);
	if (it != m_requests.end())
	{
		sStoreRequest& request = it->second;
		if (request.m_finished)
		{
			if (request.m_resultBinarySize > 0)
			{
				_size = request.m_resultBinarySize;
				_result = (unsigned char*)malloc(_size);
				memcpy(_result, request.m_resultBinary.m_memory, request.m_resultBinary.m_size);
			}

			request.m_canceled = true;
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
void HTTPDownloader::download(const std::string& _url, bool _modifyUserAgent, std::stringstream& _out)
{
	const char* url = _url.c_str();
	curl_easy_setopt(m_curl, CURLOPT_URL, url);
	/* example.com is redirected, so we tell libcurl to follow redirection */
	if (_modifyUserAgent)
		curl_easy_setopt(m_curl, CURLOPT_USERAGENT, s_curlUserAgent);
	curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, FALSE);
	curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(m_curl, CURLOPT_NOSIGNAL, 1); //Prevent "longjmp causes uninitialized stack frame" bug
	curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &_out);
	/* Perform the request, res will get the return code */
	CURLcode res = curl_easy_perform(m_curl);
	/* Check for errors */
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
			curl_easy_strerror(res));
	}
}

//------------------------------------------------------------------------------------------------
size_t write_data_binary(void *contents, size_t size, size_t nmemb, void* _stream)
{
	size_t realsize = size * nmemb;
	MemoryStruct *mem = (MemoryStruct *)_stream;

	char *ptr = (char*)realloc(mem->m_memory, mem->m_size + realsize + 1);
	if (ptr == NULL)
	{
		/* out of memory! */
		printf("not enough memory (realloc returned NULL)\n");
		return 0;
	}

	mem->m_memory = (char*)ptr;
	memcpy(&(mem->m_memory[mem->m_size]), contents, realsize);
	mem->m_size += realsize;
	mem->m_memory[mem->m_size] = 0;

	return realsize;
}

//------------------------------------------------------------------------------------------------
void HTTPDownloader::downloadBinary(const std::string& _url, bool _modifyUserAgent, MemoryStruct& _out, const char* _filePath)
{
	const char* url = _url.c_str();
	curl_easy_setopt(m_curl, CURLOPT_URL, url);
	/* example.com is redirected, so we tell libcurl to follow redirection */
	if (_modifyUserAgent)
		curl_easy_setopt(m_curl, CURLOPT_USERAGENT, s_curlUserAgent);
	curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, FALSE);
	curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(m_curl, CURLOPT_NOSIGNAL, 1); //Prevent "longjmp causes uninitialized stack frame" bug

	// Save directly as a file
	if (_filePath)
	{
		FILE* fp = fopen(_filePath, "wb");
		curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, NULL);
		curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, fp);
		fclose(fp);
	}
	// Save in memory
	else
	{
		curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, write_data_binary);
		curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &_out);
	}
	/* Perform the request, res will get the return code */
	CURLcode res = curl_easy_perform(m_curl);
	/* Check for errors */
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
			curl_easy_strerror(res));
	}
}

//------------------------------------------------------------------------------------------------
void sRequest::Process(HTTPDownloader* _downloader)
{
	switch (m_type)
	{
	case RequestResultType_String:
		{
			std::stringstream stream;
			_downloader->download(m_request, m_protectUserAgent, stream);
			
			m_result = stream.str();

			// Remove any space / tab from string
			StringTools::RemoveEOL(m_result);
		}
		break;

	case RequestResultType_Binary:
		{
			// Memory alloc 
			m_resultBinary.m_memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */
			m_resultBinary.m_size = 0;

			// Copy data
			_downloader->downloadBinary(m_request, m_protectUserAgent, m_resultBinary, !m_writeFilePath.empty() ? m_writeFilePath.c_str() : nullptr);
			m_resultBinarySize = m_resultBinary.m_size;
		}
		break;

	default:
		break;
	}

	_downloader->SetResult(m_requestID, *this);
}

//------------------------------------------------------------------------------------------------
void sStoreRequest::End()
{
	free(m_resultBinary.m_memory);
	m_resultBinarySize = 0;
}