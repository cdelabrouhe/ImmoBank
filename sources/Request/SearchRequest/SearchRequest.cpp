#include "SearchRequest.h"

//---------------------------------------------------------------------------------------------------------------------------------
void SearchRequest::copyTo(SearchRequest* _target)
{
	_target->m_requestType = m_requestType;
}