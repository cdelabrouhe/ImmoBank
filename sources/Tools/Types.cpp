#include "Types.h"
#include "StringTools.h"

void sCity::FixName()
{
	sCity::FixName(m_name);
}

void sCity::FixName(std::string& _name)
{
	StringTools::ReplaceBadSyntax(_name, "Ã‰", "É");
	StringTools::ReplaceBadSyntax(_name, "Ã©", "é");
	StringTools::ReplaceBadSyntax(_name, "Ã¨", "è");
}

void sCity::UnFixName()
{
	sCity::UnFixName(m_name);
}

void sCity::UnFixName(std::string& _name)
{
	StringTools::ReplaceBadSyntax(_name, "É", "Ã‰");
	StringTools::ReplaceBadSyntax(_name, "é", "Ã©");
	StringTools::ReplaceBadSyntax(_name, "è", "Ã¨");
}