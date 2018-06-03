#pragma once

#include "extern/sqlite/include/sqlite3.h"
#include "extern/imstr/Str.h"
#include <functional>

sqlite3* SQLOpenDB(const char* _path);
void     SQLCloseDB(sqlite3* _db);
bool     SQLExecute(sqlite3* _db, const char* _sql, ...);
bool     SQLExecuteSelect(sqlite3* _db, const char* _sql, std::function<void(sqlite3_stmt*)> _column_getter);