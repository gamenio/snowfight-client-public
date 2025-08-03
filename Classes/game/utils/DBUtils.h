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

	// 如果数据库被更新则返回true，否则返回false。
	// 当数据库更新失败时dbStoragePath为空，当数据库被更新或者无需更新时dbStoragePath为数据库文件的存储路径。
	static bool updateDBIfNeeded(int32 oldDBVersion, int32 newDBVersion, std::string const& dbFileName, std::string& dbStoragePath);
};


NS_END


#endif // __DB_UTILS_H__