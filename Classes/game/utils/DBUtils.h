#ifndef __DB_UTILS_H__
#define __DB_UTILS_H__

#include "common/Common.h"
#include "SQLiteCpp/SQLiteCpp.h"

USING_NS_CC;

NS_BEGIN

class DBUtils
{
public:
	static bool isColumnExists(SQLite::Database* m_database, std::string const& tableName, std::string const& columnName);

	// Returns true if the database has been updated, otherwise returns false.
	// When the database update fails, dbStoragePath is empty. When the database is updated or does not need to be updated, 
	// dbStoragePath is the storage path of the database file.
	static bool updateDBIfNeeded(int32 oldDBVersion, int32 newDBVersion, std::string const& dbFileName, std::string& dbStoragePath);
};


NS_END


#endif // __DB_UTILS_H__