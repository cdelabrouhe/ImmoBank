#pragma once

#include <string>
#include <vector>

class StringTools
{
public:
	static void ReplaceBadSyntax(std::string& _str, const std::string& _lookFor, const std::string& _replaceBy);
	static void RemoveEOL(std::string& _str);
	static void RemoveSpecialCharacters(std::string& _str);
	static void FindAndReplaceAll(std::string& _str, std::string _toSearch, std::string _replaceStr);
	static std::string GetXMLBaliseContent(const std::string& _str, const std::string& _balise);
	static void GetXMLBaliseArrayContent(const std::string& _str, const std::string& _balise, std::vector<std::string>& _list);
	static void GetXMLBaliseArray(const std::string& _str, const std::string& _arrayName, const std::string& _entryName, std::vector<std::string>& _list);
	static void TransformToLower(std::string& _str);
};