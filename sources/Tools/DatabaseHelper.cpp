#include "DatabaseHelper.h"
#include "Types.h"
#include <Database/DatabaseManager.h>
#include <Database/SQLDatabase.h>
#include "extern/sqlite/include/sqlite3.h"

using namespace ImmoBank;

void DatabaseHelper::AddEntry(const std::string& _name, ColumnType _type)
{
	m_entries.push_back(Entry(_name, _type));
}

void DatabaseHelper::CreateTableRequest(std::string& _request)
{
	_request = "CREATE TABLE IF NOT EXISTS `" + m_databaseName + "` ";
	_Stringify(_request, nullptr, true, true, false, true, true, false, false);
}

void DatabaseHelper::Load()
{
	auto* table = DatabaseManager::getSingleton()->GetTable(m_databaseName);
	if (table == nullptr)
		return;

	std::string request = "SELECT * FROM `" + m_databaseName + "`";
	SQLExecuteSelect(table, request.c_str(), [this](sqlite3_stmt* _stmt)
	{
		_InternalLoad(_stmt);
	});
}

void DatabaseHelper::UpdateDataInternal(EntryData* _data)
{
	_data->Generate(this);

	std::string str = "DELETE FROM `" + m_databaseName + "` " + "WHERE ";
	_Stringify(str, _data, false, false, false, false, false, true, true);

	DatabaseManager::getSingleton()->TriggerSQLCommand(m_databaseName, str);

	str = "INSERT INTO " + m_databaseName + " ";
	_Stringify(str, _data, true, false, true, false, false, false, false);

	DatabaseManager::getSingleton()->TriggerSQLCommand(m_databaseName, str);
}

void DatabaseHelper::_Stringify(std::string& _str, EntryData* _data, bool _addBrackets, bool _addQuote, bool _addValues, bool _addReturn, bool _addType, bool _addAnd, bool _affectValues)
{
	bool first = true;

	int entryID = 0;
	for (Entry& entry : m_entries)
	{
		if (!first)
		{
			if (_addAnd)
				_str += " AND ";
			else
				_str += ",";

			if (_addReturn)
				_str += "\n";
		}
		else if (_addBrackets)
			_str += "(";

		first = false;
		if (_addQuote)	_str += "`";
		_str += entry.m_name;
		if (_addQuote)	_str += "`";

		if (_addType)
		{
			switch (entry.m_type)
			{
			case ColumnType_FLOAT: _str += " REAL"; break;
			case ColumnType_INT: _str += " INTEGER"; break;
			case ColumnType_TEXT: _str += " TEXT"; break;
			}
		}

		if ((_data != nullptr) && (_affectValues))
		{
			switch (entry.m_type)
			{
			case ColumnType_INT:	_str += "=" + std::to_string(_data->m_data[entryID].m_iVal);	break;
			case ColumnType_FLOAT:	_str += "=" + std::to_string(_data->m_data[entryID].m_fVal);	break;
			case ColumnType_TEXT:	_str += "='" + _data->m_data[entryID].m_sVal + "'";	break;
			}
		}
		++entryID;
	}

	if (_addBrackets)
		_str += ")";

	first = true;
	if (_data != nullptr)
	{
		if (_addValues)
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
}

void DatabaseHelper::UpdateEntryData(EntryData& _data)
{
	EntryData* data = _GetEntryDataFromSource(&_data);
	if (data == nullptr)
	{
		data = _GenerateEntryData();
		m_data.push_back(data);
	}

	_data.copyTo(data);

	UpdateDataInternal(data);
}

Entry* DatabaseHelper::GetEntry(const int _ID)
{
	if (_ID >= m_entries.size())
		return nullptr;

	return &m_entries[_ID];
}

void DatabaseHelper::_InternalLoad(sqlite3_stmt* _stmt)
{
	EntryData* data = _GenerateEntryData();
	int index = 0;
	EntryValue val;
	for (auto& entry : m_entries)
	{
		bool valid = true;
		EntryValue val;
		switch (entry.m_type)
		{
		case ColumnType_FLOAT:	val.m_fVal = (float)sqlite3_column_double(_stmt, index++);		break;
		case ColumnType_INT:	val.m_iVal = sqlite3_column_int(_stmt, index++);				break;
		case ColumnType_TEXT:	val.m_sVal = (const char*)sqlite3_column_text(_stmt, index++);	break;
		default:	valid = false;	break;
		}

		if (valid)
			data->m_data.push_back(val);
	}

	data->Load(this);
	m_data.push_back(data);
}