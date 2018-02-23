#include "StringTools.h"
#include <algorithm>

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
	StringTools::ReplaceBadSyntax(_str, "Ã©", "e");// "é");
	StringTools::ReplaceBadSyntax(_str, "Ã¨", "e");//  "è");
	StringTools::ReplaceBadSyntax(_str, "Ã´", "o");//  "è");
	StringTools::ReplaceBadSyntax(_str, "Ã€", "A");//  "À");
	StringTools::ReplaceBadSyntax(_str, "Â²", "2");
	StringTools::ReplaceBadSyntax(_str, "Â", "");
	StringTools::ReplaceBadSyntax(_str, "Ã ", "a");//  "à");
	StringTools::ReplaceBadSyntax(_str, " ", " ");//  "à");
	StringTools::ReplaceBadSyntax(_str, "àˆ", "E");//  "È");
	StringTools::ReplaceBadSyntax(_str, "àª", "e");//  "ê");
	StringTools::ReplaceBadSyntax(_str, "à‰", "E");//  "É");
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