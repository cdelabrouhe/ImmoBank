#include "DatabaseHelper.h"
#include "Types.h"

using namespace ImmoBank;

void DatabaseHelper::AddEntry(const std::string& _name, ColumnType _type)
{
	m_entries.push_back(Entry(_name, _type));
}

void DatabaseHelper::CreateTable()
{
	std::string str = "CREATE TABLE IF NOT EXISTS `" + m_databaseName + "` ";
	_Stringify(str, nullptr, true, false, true);
}

void DatabaseHelper::UpdateDataInternal(EntryData* _data)
{
	_data->Generate(this);

	std::string str = "INSERT OR REPLACE INTO " + m_databaseName;
	_Stringify(str, _data, false, true, false);
}

void DatabaseHelper::_Stringify(std::string& _str, EntryData* _data, bool _addQuote, bool _addValues, bool _addReturn)
{
	bool first = true;

	for (Entry& entry : m_entries)
	{
		if (!first)
		{
			if (_addReturn)
				_str += ",\n";
			else
				_str += ", ";
		}
		else
			_str += "(";

		first = false;
		if (_addQuote)	_str += "`";
		_str += entry.m_name;
		if (_addQuote)	_str += "`";
	}

	_str += ")";

	first = true;
	if (_addValues && (_data != nullptr))
	{
		int entryID = 0;
		for (Entry& entry : m_entries)
		{
			if (!first)
				_str += ", ";
			else
				_str += " VALUES(";

			first = false;

			switch (entry.m_type)
			{
			case ColumnType_INT:	_str += std::to_string(_data->m_data[entryID].m_iVal);	break;
			case ColumnType_FLOAT:	_str += std::to_string(_data->m_data[entryID].m_fVal);	break;
			case ColumnType_TEXT:	_str += "'" + _data->m_data[entryID].m_sVal + "'";	break;
			}

			++entryID;
		}

		_str += ")";
	}
}

void DatabaseHelper::UpdateEntryData(EntryData& _data)
{
	EntryData* data = GetEntryDataFromSource(&_data);
	if (data == nullptr)
	{
		data = GenerateEntryData();
		m_data.push_back(data);
	}

	_data.copyTo(data);

	UpdateDataInternal(data);
}

Entry* ImmoBank::DatabaseHelper::GetEntry(const int _ID)
{
	if (_ID >= m_entries.size())
		return nullptr;

	return &m_entries[_ID];
}
