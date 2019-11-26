#include "OnlineManager.h"
#include "HTTPDownloader.h"
#include "Request/SearchRequest/SearchRequest.h"
#include "Tools/StringTools.h"
#include "SeLogerOnlineDatabase.h"
#include "LaforetOnlineDatabase.h"
#include "OrpiOnlineDatabase.h"
#include "LogicImmoOnlineDatabase.h"
#include "extern/ImGui/imgui.h"
#include "Tools/Tools.h"

using namespace ImmoBank;

OnlineManager* s_singleton = nullptr;

//-------------------------------------------------------------------------------------------------
OnlineManager* OnlineManager::getSingleton()
{
	if (s_singleton == nullptr)
		s_singleton = new OnlineManager();
	return s_singleton;
}

//-------------------------------------------------------------------------------------------------
// DATA
//-------------------------------------------------------------------------------------------------
HTTPDownloader	s_downloader;

//-------------------------------------------------------------------------------------------------
// FUNCTIONS
//-------------------------------------------------------------------------------------------------
void OnlineManager::Init()
{
	s_downloader.Init();

	/*auto seLogerDB = new SeLogerOnlineDatase();
	seLogerDB->Init();
	m_databases.push_back(seLogerDB);*/

	auto laforetDB = new LaforetOnlineDatabase();
	laforetDB->Init();
	m_databases.push_back(laforetDB);

	auto orpiDB = new OrpiOnlineDatabase();
	orpiDB->Init();
	m_databases.push_back(orpiDB);*/

	auto logicImmoDB = new LogicImmoOnlineDatabase();
	logicImmoDB->Init();
	m_databases.push_back(logicImmoDB);
}

//-------------------------------------------------------------------------------------------------
void OnlineManager::Process()
{
	s_downloader.Process();

	for (auto db : m_databases)
		db->Process();

	for (auto& request : m_requests)
		request.second->Process();

	if (Tools::IsDevMode())
		DisplayDebug();
}

//-------------------------------------------------------------------------------------------------
void OnlineManager::End()
{
	s_downloader.End();

	for (auto db : m_databases)
		db->End();
}

//-------------------------------------------------------------------------------------------------
int OnlineManager::SendRequest(SearchRequest* _request)
{
	int ID = 0;
	while (m_requests.find(ID) != m_requests.end())
		++ID;

	SearchRequest* request = _request->Clone();
	if (request)
	{
		m_requests[ID] = request;
		_request->copyTo(request);
		request->Init();
	}
		
	return ID;
}

//-------------------------------------------------------------------------------------------------
bool OnlineManager::IsRequestAvailable(int _requestID) const
{
	auto it = m_requests.find(_requestID);
	if (it != m_requests.end())
		return it->second->IsAvailable();
	return false;
}

//-------------------------------------------------------------------------------------------------
void OnlineManager::DeleteRequest(int _requestID)
{
	auto it = m_requests.find(_requestID);
	if (it != m_requests.end())
	{
		SearchRequest* request = it->second;
		request->End();
		delete request;
		m_requests.erase(it);
	}
}

//-------------------------------------------------------------------------------------------------
bool OnlineManager::GetRequestResult(const int _requestID, std::vector<SearchRequestResult*>& _result)
{
	if (!IsRequestAvailable(_requestID))
		return false;

	bool valid = true;

	auto it = m_requests.find(_requestID);
	if (it != m_requests.end())
	{
		SearchRequest* request = it->second;
		valid = request->GetResult(_result);
	}

	if (valid)
		DeleteRequest(_requestID);

	return valid;
}

//-------------------------------------------------------------------------------------------------
int OnlineManager::SendBasicHTTPRequest(const std::string& _request, bool _modifyUserAgent)
{
	return s_downloader.SendRequest(_request, RequestResultType::RequestResultType_String,_modifyUserAgent);
}

//-------------------------------------------------------------------------------------------------
int OnlineManager::SendBinaryHTTPRequest(const std::string& _request, const std::string& _writeFilePath, bool _modifyUserAgent)
{
	return s_downloader.SendRequest(_request, RequestResultType::RequestResultType_Binary, _modifyUserAgent, _writeFilePath);
}

//-------------------------------------------------------------------------------------------------
bool OnlineManager::IsHTTPRequestAvailable(int _requestID) const
{
	return s_downloader.IsRequestAvailable(_requestID);
}

//-------------------------------------------------------------------------------------------------
bool OnlineManager::GetBasicHTTPRequestResult(const int _requestID, std::string& _result)
{
	return s_downloader.GetResultString(_requestID, _result);
}

//-------------------------------------------------------------------------------------------------
bool OnlineManager::GetBinaryHTTPRequestResult(const int _requestID, unsigned char*& _result, int& _size)
{
	return s_downloader.GetResultBinary(_requestID, _result, _size);
}

//-------------------------------------------------------------------------------------------------
void OnlineManager::CancelBasicHTTPRequest(const int _requestID)
{
	s_downloader.CancelRequest(_requestID);
}

//-------------------------------------------------------------------------------------------------
void OnlineManager::DisplayDebug()
{
	if (!m_displayDebug)
		return;

	ImGui::Begin("OnlineManager Debug panel", &m_displayDebug);
	ImGui::Text("Number of active requests: %u", m_requests.size());
	ImGui::Text("Number of active HTTP requests: %d", s_downloader.GetNbRequests());
	ImGui::BeginChild("Tools", ImVec2(ImGui::GetWindowContentRegionWidth(), 60), false, ImGuiWindowFlags_NoScrollbar);

	// Debug request
	bool callCommand = ImGui::Button("Call") && strlen(m_inputDebug) > 0;
	ImGui::SameLine();
	ImGui::InputText("HTTP request", (char*)m_inputDebug, 2048);
	if ((m_testRequest == -1) && callCommand)
	{
		std::string str = m_inputDebug;
		m_testRequest = OnlineManager::getSingleton()->SendBasicHTTPRequest(str);
	}

	if (ImGui::Button("Clear"))
		m_testRequestResult.clear();

	ImGui::SameLine();
	if (ImGui::Button("Copy"))
		ImGui::SetClipboardText(m_testRequestResult.c_str());

	ImGui::Separator();

	ImGui::EndChild();

	if (m_testRequest == -1)
		ImGui::TextWrapped("%s", m_testRequestResult.c_str());
	else
	{
		ImGui::Text("Processing request...");
		if (IsHTTPRequestAvailable(m_testRequest))
		{
			GetBasicHTTPRequestResult(m_testRequest, m_testRequestResult);
			m_testRequest = -1;
		}
	}

	ImGui::End();
}
