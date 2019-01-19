#include "Types.h"
#include "StringTools.h"

using namespace ImmoBank;

void sCity::FixName()
{
	sCity::FixName(m_name);
}

void sCity::FixName(std::string& _name)
{
	StringTools::FixName(_name);
}

void sCity::UnFixName()
{
	sCity::UnFixName(m_name);
}

void sCity::UnFixName(std::string& _name)
{
	StringTools::UnFixName(_name);
}