#include "SQLDatabase.h"

#pragma comment(lib, "sqlite3.lib")

static void SQLLog(const char* _log, ...)
{
	va_list args;
	va_start(args, _log);
	Str256 log;
	log.setfv(_log, args);
	va_end(args);

	printf(log.c_str());
}

sqlite3* SQLOpenDB(const char* _path)
{
	// Make sure the directory exists, or sqlite_open will fail to create the file.
	//Pasta::FileMgr::getSingleton()->createDirectories(_path);

	sqlite3* db;
	int ret = sqlite3_open(_path, &db);
	if (ret != SQLITE_OK)
	{
		SQLLog("SQL Error opening DB %s (%d)\n", _path, ret);
		return nullptr;
	}

	sqlite3_busy_timeout(db, 20 * 1000); // timeout after X sec 

										 // These pragmas make the DB a LOT faster, especially when accessed concurrently by several processes.
										 // - journal_mode OFF:
										 // The ROLLBACK command no longer works. 
										 // If the application crashes in the middle of a transaction when the OFF journaling mode is set, 
										 // then the database file will very likely go corrupt. ([Jeremy] It's OK-ish, we don't use transactions.)
	SQLExecute(db, "PRAGMA journal_mode = OFF");
	// - synchronous OFF:
	// With synchronous OFF (0), SQLite continues without syncing as soon as it has handed data off
	// to the operating system. If the application running SQLite crashes, the data will be safe, 
	// but the database might become corrupted if the operating system crashes or the computer 
	// loses power before that data has been written to the disk surface. 
	// On the other hand, commits can be orders of magnitude faster with synchronous OFF. 
	SQLExecute(db, "PRAGMA synchronous = OFF");

	return db;
}

void SQLCloseDB(sqlite3* _db)
{
	sqlite3_close(_db);
}

bool SQLExecute(sqlite3* _db, const char* _sql, ...)
{
	Str256 sql;

	va_list args;
	va_start(args, _sql);
	sql.setfv(_sql, args);
	va_end(args);

	char* error;
	int ret = sqlite3_exec(_db, sql.c_str(), nullptr, nullptr, &error);

	bool corrupted = ret == SQLITE_CORRUPT;
	if (corrupted)
		printf("Database corrupted!\n"
		"\n"
		"Close MonsieurCuisinier and delete:\n"
		"%s\n"
		"\n"
		"That will teach it!",
		sqlite3_db_filename(_db, "main"));

	if (ret != SQLITE_OK)
	{
		SQLLog("SQLExecute ERROR: %s\n  for statement:\n %s\n\n", error, sql.c_str());
		return false;
	}

	return true;
}

bool SQLExecuteSelect(sqlite3* _db, const char* _sql, std::function<void(sqlite3_stmt*)> _column_getter)
{
	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(_db, _sql, -1, &stmt, nullptr) != SQLITE_OK)
	{
		SQLLog("SQLExecute ERROR: %s\n  for statement:\n %s\n\n", sqlite3_errmsg(_db), _sql);
		return false;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		if (_column_getter)
			_column_getter(stmt);
	}

	if (sqlite3_finalize(stmt) != SQLITE_OK)
	{
		SQLLog("SQLExecute ERROR: %s\n  for statement:\n %s\n\n", sqlite3_errmsg(_db), _sql);
		return false;
	}

	return true;
}
