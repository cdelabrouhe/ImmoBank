#pragma once

#include <string>
#include <vector>
#include <map>

namespace ImmoBank
{
	enum ColumnType
	{
		ColumnType_NONE = -1,
		ColumnType_INT,
		ColumnType_FLOAT,
		ColumnType_TEXT,
		ColumnType_COUNT
	};

	struct Entry
	{
		Entry(const std::string& _name = "", ColumnType _type = ColumnType_NONE) : m_name(_name), m_type(_type)	{}

		std::string m_name;
		ColumnType	m_type;
	};

	class DatabaseHelper;

	struct EntryValue
	{
		std::string m_sVal;
		int m_iVal;
		float m_fVal;
	};

	struct EntryData
	{
		virtual void Generate(DatabaseHelper* _db)	{}
		virtual void copyTo(EntryData* _target) = 0;

		std::vector<EntryValue>	m_data;
	};

	class DatabaseHelper
	{
	public:
		void SetDatabaseName(const std::string& _name)				{ m_databaseName = _name;	}
		void AddEntry(const std::string& _name, ColumnType _type);
		size_t GetEntryCount() { return m_entries.size(); }
		Entry* GetEntry(const int _ID);
		void UpdateEntryData(EntryData& _data);
		void CreateTable();
		void UpdateDataInternal(EntryData* _data);
		virtual EntryData* GetEntryDataFromSource(EntryData* _source) { return nullptr; }
		virtual EntryData* GenerateEntryData() { return nullptr; }

	private:
		void _Stringify(std::string& _str, EntryData* _data, bool _addQuote, bool _addValues, bool _addReturn);

	protected:
		std::string				m_databaseName;
		std::vector<Entry>		m_entries;
		std::vector<EntryData*>	m_data;
	};
}