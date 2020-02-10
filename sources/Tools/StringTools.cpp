#include "StringTools.h"
#include <algorithm>

using namespace ImmoBank;

//------------------------------------------------------------------------------------------------
void StringTools::ReplaceBadSyntax(std::string& _str, const std::string& _lookFor, const std::string& _replaceBy)
{
	int ID = (int)_str.find(_lookFor);
	while (ID > -1)
	{
		_str.replace(ID, _lookFor.size(), _replaceBy);
		ID = (int)_str.find(_lookFor);
	}
}

//-------------------------------------------------------------------------------------------------
void StringTools::FindAndReplaceAll(std::string& _str, std::string _toSearch, std::string _replaceStr)
{
	// Get the first occurrence
	size_t pos = _str.find(_toSearch);

	// Repeat till end is reached
	while (pos != std::string::npos)
	{
		// Replace this occurrence of Sub String
		_str.replace(pos, _toSearch.size(), _replaceStr);
		// Get the next occurrence from the current position
		pos = _str.find(_toSearch, pos + _toSearch.size());
	}
}

//-------------------------------------------------------------------------------------------------
void StringTools::RemoveEOL(std::string& _str)
{
	FindAndReplaceAll(_str, "\r\n", "");
	FindAndReplaceAll(_str, "\n", "");
}

//-------------------------------------------------------------------------------------------------
void StringTools::RemoveSpecialCharacters(std::string& _str)
{
	// Correct some bad characters
	StringTools::ReplaceBadSyntax(_str, "&#39;", "'");
	StringTools::ReplaceBadSyntax(_str, "&#039;", "'");
	StringTools::ReplaceBadSyntax(_str, "&amp;", "&");
	StringTools::ReplaceBadSyntax(_str, "&quot;", "`");
	StringTools::ReplaceBadSyntax(_str, "Â½", "oe");
	StringTools::ReplaceBadSyntax(_str, "Ã©", "e");// "é");
	StringTools::ReplaceBadSyntax(_str, "Ã¨", "e");//  "è");
	StringTools::ReplaceBadSyntax(_str, "Ã´", "o");//  "è");
	StringTools::ReplaceBadSyntax(_str, "Ã€", "A");//  "À");
	StringTools::ReplaceBadSyntax(_str, "Ã¢", "a");//  "â");
	StringTools::ReplaceBadSyntax(_str, "Â²", "2");
	StringTools::ReplaceBadSyntax(_str, "Â", "");
	StringTools::ReplaceBadSyntax(_str, "Ã ", "a");//  "à");
	StringTools::ReplaceBadSyntax(_str, " ", " ");//   "à");
	StringTools::ReplaceBadSyntax(_str, "àˆ", "E");//  "È");
	StringTools::ReplaceBadSyntax(_str, "àª", "e");//  "ê");
	StringTools::ReplaceBadSyntax(_str, "à‰", "E");//  "É");
	StringTools::ReplaceBadSyntax(_str, "Ã‰", "E");//  "É");
	StringTools::ReplaceBadSyntax(_str, "Ã§", "c");//  "É");
	StringTools::ReplaceBadSyntax(_str, "%", "pourcent");

	/*static bool s_test = false;
	if (s_test)
	{
	FILE* fp = 0;
	errno_t err = fopen_s(&fp, "test3.xml", "w");
	if (!err)
	{
	fwrite(_result.c_str(), sizeof(char), _result.size(), fp);
	fclose(fp);
	}
	}*/
}

std::string StringTools::GetXMLBaliseContent(const std::string& _str, const std::string& _balise)
{
	std::string start = "<" + _balise + ">";
	std::string stop = "</" + _balise + ">";

	int startID = (int)_str.find(start);
	int stopID = (int)_str.find(stop);
	if ((startID >= 0) && (stopID >= 0))
		return _str.substr(startID + start.size(), stopID - startID - start.size());
	return "";
}

void StringTools::GetXMLBaliseArrayContent(const std::string& _str, const std::string& _balise, std::vector<std::string>& _list)
{
	std::string str = _str;

	std::string start = "<" + _balise + ">";
	std::string stop = "</" + _balise + ">";

	int startID = (int)str.find(start);
	int stopID = (int)str.find(stop);

	while (startID > -1)
	{
		if ((startID >= 0) && (stopID >= 0))
			_list.push_back(str.substr(startID + start.size(), stopID - startID - start.size()));

		str = str.substr(stopID + stop.size(), str.size());
		startID = (int)str.find(start);
		stopID = (int)str.find(stop);
	}
}

void StringTools::GetXMLBaliseArray(const std::string& _str, const std::string& _arrayName, const std::string& _entryName, std::vector<std::string>& _list)
{
	std::string str = GetXMLBaliseContent(_str, _arrayName);

	GetXMLBaliseArrayContent(str, _entryName, _list);
}

void StringTools::TransformToLower(std::string& _str)
{
	std::transform(_str.begin(), _str.end(), _str.begin(), ::tolower);
}

void StringTools::ConvertToImGuiText(std::string& _text)
{
	StringTools::ReplaceBadSyntax(_text, "É", "E");
	StringTools::ReplaceBadSyntax(_text, "é", "e");
	StringTools::ReplaceBadSyntax(_text, "è", "e");
	StringTools::ReplaceBadSyntax(_text, "ê", "e");
}

constexpr unsigned int HashStrRecur(unsigned int _hash, const char* _str)
{
	return (*_str == 0) ? _hash : HashStrRecur(((_hash << 5) + _hash) + *_str, _str + 1);
}

unsigned int StringTools::GenerateHash(const std::string& _str)
{
	const char* str = _str.c_str();
	return GenerateHash(str);
}

unsigned int StringTools::GenerateHash(const char* _str)
{
	return HashStrRecur(5381, _str);
}

void StringTools::FixName(std::string& _name)
{
	StringTools::ReplaceBadSyntax(_name, "Ã‰", "É");
	StringTools::ReplaceBadSyntax(_name, "Ã©", "é");
	StringTools::ReplaceBadSyntax(_name, "Ã¨", "è");
	StringTools::ReplaceBadSyntax(_name, "Ã®", "î");
}

void StringTools::UnFixName(std::string& _name)
{
	StringTools::ReplaceBadSyntax(_name, "É", "Ã‰");
	StringTools::ReplaceBadSyntax(_name, "é", "Ã©");
	StringTools::ReplaceBadSyntax(_name, "è", "Ã¨");
	StringTools::ReplaceBadSyntax(_name, "î", "Ã®");
}

std::string StringTools::ExtractStringFromPosition(const std::string& _str, size_t _position, char _lookForBordersChar)
{
	while ((_position > 0) && (_str[_position] != _lookForBordersChar))
		--_position;

	if (_position == 0)
		return "";

	std::string tmp = _str.substr(_position + 1, _str.size());
	auto findID = tmp.find_first_of("\"");
	tmp = tmp.substr(0, findID);
	return tmp;
}