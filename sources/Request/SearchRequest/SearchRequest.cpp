#include "SearchRequest.h"

using namespace ImmoBank;

//---------------------------------------------------------------------------------------------------------------------------------
void SearchRequest::copyTo(SearchRequest* _target)
{
	_target->m_requestType = m_requestType;
}