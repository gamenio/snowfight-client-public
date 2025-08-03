#include "DBUtils.h"

#include "platform/CCFileUtils.h"

NS_BEGIN


bool DBUtils::isColumnExists(SQLite::Database* m_database, std::string const& tableName, std::string const& columnName)
{
	SQLite::Statement query(*m_database, "SELECT * FROM " + tableName + " limit 1");
	for (int i = 0; i < query.getColumnCount(); ++i)
	{
		std::string name = query.getColumnName(i);
		if (columnName == name)
			return true;
	}
	return false;
}

bool DBUtils::updateDBIfNeeded(int32 oldDBVersion, int32 newDBVersion, std::string const& dbFileName, std::string& dbStoragePath)
{
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
	bool ret = false;
	std::string dbFile = FileUtils::getInstance()->getWritablePath() + dbFileName;
	bool isFileExist = FileUtils::getInstance()->isFileExist(dbFile);
	if (oldDBVersion < newDBVersion || !isFileExist)
	{
		if (isFileExist)
			FileUtils::getInstance()->removeFile(dbFile);

		Data data = FileUtils::getInstance()->getDataFromFile(dbFileName);
		ret = !data.isNull();
		if (ret)
		{
		    std::size_t found = dbFile.find_last_of('/');
		    if(found != std::string::npos)
            {
				std::string path = dbFile.substr(0, found);
                if(!FileUtils::getInstance()->isDirectoryExist(path))
                    ret = FileUtils::getInstance()->createDirectory(path);
            }
		    if(ret)
            {
                ret = FileUtils::getInstance()->writeDataToFile(data, dbFile);
                if (ret)
                    dbStoragePath = dbFile;
            }
		}
	}
	else
    {
#if NS_DEBUG
        if (isFileExist)
        {
            std::string newFileMD5 = utils::getFileMD5Hash(dbFileName);
            std::string oldFileMD5 = utils::getFileMD5Hash(dbFile);
            // ERROR: DB file has been updated, but the DB version has not been increased.
            assert(newFileMD5 == oldFileMD5);
        }
#endif // NS_DEBUG

        dbStoragePath = dbFile;
    }

	return ret;
#else
	dbStoragePath = FileUtils::getInstance()->fullPathForFilename(dbFileName);
	return false;
#endif // CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
}


NS_END

