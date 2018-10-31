#include "Types.h"
#include "StringTools.h"

void sCity::FixName()
{
	sCity::FixName(m_name);
}

void sCity::FixName(std::string& _name)
{
	StringTools::ReplaceBadSyntax(_name, "É", "�");
	StringTools::ReplaceBadSyntax(_name, "é", "�");
	StringTools::ReplaceBadSyntax(_name, "è", "�");
}

void sCity::UnFixName()
{
	sCity::UnFixName(m_name);
}

void sCity::UnFixName(std::string& _name)
{
	StringTools::ReplaceBadSyntax(_name, "�", "É");
	StringTools::ReplaceBadSyntax(_name, "�", "é");
	StringTools::ReplaceBadSyntax(_name, "�", "è");
}