#ifndef __OBJECT_MGR_H__
#define __OBJECT_MGR_H__

#include "SQLiteCpp/SQLiteCpp.h"

#include "common/Common.h"
#include "game/entities/DataPlayer.h"
#include "game/entities/DataItem.h"

NS_BEGIN

typedef std::unordered_map<uint32/* TemplateId */, PlayerTemplate> PlayerTemplateContainer;
typedef std::unordered_map<uint32/* TemplateId */, ItemTemplate> ItemTemplateContainer;
typedef std::unordered_map<uint32/* TemplateId */, ItemApplicationTemplate> ItemApplicationTemplateContainer;

class ObjectMgr
{
public:
	static ObjectMgr* instance();

	void loadAsync(std::function<void(bool)> callback = nullptr);
	bool load();

	PlayerTemplate const* getPlayerTemplate(uint32 id) const;

	ItemTemplate const* getItemTemplate(uint32 id) const;
	ItemApplicationTemplate const* getItemApplicationTemplate(uint32 id) const;

private:
	ObjectMgr();
	~ObjectMgr();

	bool loadPlayerData();
	bool loadItemData();

	std::string getDBFilePath(std::string const& dbFilename) const;

	bool m_isLoading;

	PlayerTemplateContainer m_playerTemplateStore;

	ItemTemplateContainer m_itemTemplateStore;
	ItemApplicationTemplateContainer m_itemApplicationTemplateStore;
};

#define sObjectMgr ObjectMgr::instance()


NS_END

#endif // __OBJECT_MGR_H__
