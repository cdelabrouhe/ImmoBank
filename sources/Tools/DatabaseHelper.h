#pragma once

#include <string>
#include <vector>
#include <map>

struct sqlite3_stmt;

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
		virtual void Load(DatabaseHelper* _db)		{}
		virtual void copyTo(EntryData* _target) = 0;

		std::vector<EntryValue>	m_data;
	};

	class DatabaseHelper
	{
	public:
		void SetDatabaseName(const std::string& _name)				{ m_databaseName = _name;	}
		const std::string& GetDatabaseName() const					{ return m_databaseName;	}
		void AddEntry(const std::string& _name, ColumnType _type);
		size_t GetEntryCount() { return m_entries.size(); }
		Entry* GetEntry(const int _ID);
		void UpdateEntryData(EntryData& _data);
		void CreateTableRequest(std::string& _request);
		void UpdateDataInternal(EntryData* _data, bool _affectServerData = true);
		void UpdateFromExternalDatabase();

		virtual void Load();

	protected:
		virtual EntryData* _GetEntryDataFromSource(EntryData* _source) const	{ return nullptr;	}
		virtual EntryData* _GetEntryDataFromFullKey(void* _key) const			{ return nullptr;	}
		virtual EntryData* _GetEntryDataFromCityName(const std::string& _name) const { return nullptr; }
		virtual EntryData* _GenerateEntryData() { return nullptr; }
		void _ClearAllData();

	private:
		void _Stringify(std::string& _str, EntryData* _data, bool _addBrackets, bool _addQuote, bool _addValues, bool _addReturn, bool _addType, bool _addAnd, bool _affectValues);
		void _InternalLoad(sqlite3_stmt*);

	protected:
		std::string				m_databaseName;
		std::vector<Entry>		m_entries;
		std::vector<EntryData*>	m_data;
	};
}