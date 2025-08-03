#include "ObjectMgr.h"

#include "common/utils/TimeUtil.h"
#include "game/firservice/FirebaseService.h"
#include "LocaleMgr.h"
#include "UserPreferences.h"
#include "utils/DBUtils.h"

NS_BEGIN

#define PLAYER_DB_FILENAME			"player.db"
#define PLAYER_DB_VERSION			2

#define ITEM_DB_FILENAME			"item.db"
#define ITEM_DB_VERSION				3

ObjectMgr* ObjectMgr::instance()
{
	static ObjectMgr instance;
	return &instance;
}

void ObjectMgr::loadAsync(std::function<void(bool)> callback)
{
	if (m_isLoading)
	{
		CCLOG("ObjectMgr: Object data is already loading.");
		Director::getInstance()->getScheduler()->performFunctionInCocosThread(std::bind(callback, false));
		return;
	}

	m_isLoading = true;

	AsyncTaskPool::getInstance()->enqueue(AsyncTaskPool::TaskType::TASK_IO, [](void*) {}, nullptr, [this, callback]() {
		bool ret = this->load();
		Director::getInstance()->getScheduler()->performFunctionInCocosThread([this, callback, ret] {
			m_isLoading = false;
			if (callback)
				callback(ret);

			if(!ret)
				sAnalytics->logException("object", "Load data failed");
		});
	});
}

bool ObjectMgr::load()
{
	NSTime currTime = time_util::getUptimeMillis();

	if (!this->loadPlayerData())
		return false;

	if (!this->loadItemData())
		return false;

	NSTime diff = time_util::getUptimeMillis() - currTime;
    NS_UNUSED_VARIABLE(diff);
	CCLOG("ObjectMgr: Object data load time: %dms.", diff);

	return true;
}

PlayerTemplate const* ObjectMgr::getPlayerTemplate(uint32 id) const
{
	auto it = m_playerTemplateStore.find(id);
	if (it != m_playerTemplateStore.end())
		return &((*it).second);

	return nullptr;
}

ItemTemplate const* ObjectMgr::getItemTemplate(uint32 id) const
{
	auto it = m_itemTemplateStore.find(id);
	if (it != m_itemTemplateStore.end())
		return &((*it).second);

	return nullptr;
}

ItemApplicationTemplate const* ObjectMgr::getItemApplicationTemplate(uint32 id) const
{
	auto it = m_itemApplicationTemplateStore.find(id);
	if (it != m_itemApplicationTemplateStore.end())
		return &((*it).second);

	return nullptr;
}

ObjectMgr::ObjectMgr() :
	m_isLoading(false)
{
}

ObjectMgr::~ObjectMgr()
{
}

bool ObjectMgr::loadPlayerData()
{
	std::string dbFile = this->getDBFilePath(PLAYER_DB_FILENAME);
	std::string dbStoragePath;
	if (DBUtils::updateDBIfNeeded(sUserPreferences->getPlayerDBVersion(), PLAYER_DB_VERSION, dbFile, dbStoragePath))
		sUserPreferences->setPlayerDBVersion(PLAYER_DB_VERSION);

	if (dbStoragePath.empty())
	{
		CCLOG("ObjectMgr: Failed to update DB file %s. ", dbFile.c_str());
		return false;
	}

	try
	{
		SQLite::Database db(dbStoragePath);
		try
		{
			SQLite::Statement query(db, "SELECT * FROM player_template");
			while (query.executeStep())
			{
				uint32 id = query.getColumn("id");
				PlayerTemplate& tmpl = m_playerTemplateStore[id];
				tmpl.id = id;
			}

			CCLOG("ObjectMgr: Loaded %zu player templates.", m_playerTemplateStore.size());
		}
		catch (std::exception const& e)
		{
			CCLOG("ObjectMgr: Load player template failed. error: %s", e.what());
			return false;
		}

		try
		{
			for (auto it = m_playerTemplateStore.begin(); it != m_playerTemplateStore.end(); ++it)
			{
				PlayerTemplate& tmpl = (*it).second;

				SQLite::Statement countQuery(db, "SELECT count(*) FROM player_stage_stats WHERE template_id=:template_id");
				countQuery.bind(":template_id", (*it).first);
				if (countQuery.executeStep())
					tmpl.stageStatsList.resize(countQuery.getColumn(0).getInt());

				SQLite::Statement query(db, "SELECT * FROM player_stage_stats WHERE template_id=:template_id ORDER BY stage ASC");
				query.bind(":template_id", (*it).first);
				while (query.executeStep())
				{
					uint8 stage = query.getColumn("stage");
					tmpl.stageStatsList[stage][STAT_MAX_HEALTH].value = query.getColumn("max_health").getInt();
					tmpl.stageStatsList[stage][STAT_HEALTH_REGEN_RATE].value = (float)query.getColumn("health_regen_rate").getDouble();
					tmpl.stageStatsList[stage][STAT_ATTACK_RANGE].value = (float)query.getColumn("attack_range").getDouble();
					tmpl.stageStatsList[stage][STAT_MOVE_SPEED].value = query.getColumn("move_speed").getInt();
					tmpl.stageStatsList[stage][STAT_MAX_STAMINA].value = query.getColumn("max_stamina").getInt();
					tmpl.stageStatsList[stage][STAT_STAMINA_REGEN_RATE].value = (float)query.getColumn("stamina_regen_rate").getDouble();
					tmpl.stageStatsList[stage][STAT_ATTACK_TAKES_STAMINA].value = query.getColumn("attack_takes_stamina").getInt();
					tmpl.stageStatsList[stage][STAT_DAMAGE].value = query.getColumn("damage").getInt();
				}

			}
		}
		catch (std::exception const& e)
		{
			CCLOG("ObjectMgr: Load player stage stats failed. error: %s", e.what());
			return false;
		}

		return true;
	}
	catch (std::exception const& ex)
	{
		CCLOG("ObjectMgr: Failed to load DB file %s. error: %s", dbStoragePath.c_str(), ex.what());
	}

	return false;
}

bool ObjectMgr::loadItemData()
{
	std::string dbFile = this->getDBFilePath(ITEM_DB_FILENAME);
	std::string dbStoragePath;
	if (DBUtils::updateDBIfNeeded(sUserPreferences->getItemDBVersion(), ITEM_DB_VERSION, dbFile, dbStoragePath))
		sUserPreferences->setItemDBVersion(ITEM_DB_VERSION);

	if (dbStoragePath.empty())
	{
		CCLOG("ObjectMgr: Failed to update DB file %s. ", dbFile.c_str());
		return false;
	}

	try
	{
		SQLite::Database db(dbStoragePath);
		try
		{
			SQLite::Statement query(db, "SELECT * FROM item_template");

			while (query.executeStep())
			{
				uint32 id = query.getColumn("id");
				ItemTemplate& tmpl = m_itemTemplateStore[id];
				tmpl.id = id;
				tmpl.displayId = query.getColumn("display_id");
				tmpl.itemClass = query.getColumn("class");
				tmpl.itemSubClass = query.getColumn("subclass");
				tmpl.level = (uint8)query.getColumn("level").getUInt();
				tmpl.stackable = query.getColumn("stackable");
				tmpl.appId = query.getColumn("app_id");
//				int32 index = query.getColumnIndex("stat_type1");
				for (int32 i = 0; i < MAX_ITEM_STATS; ++i)
				{
					ItemStat stat;
					stat.type = query.getColumn(StringUtils::format("stat_type%d", (i + 1)).c_str());
					if (stat.type != ITEM_STAT_NONE)
					{
						stat.value = query.getColumn(StringUtils::format("stat_value%d", (i + 1)).c_str());
						tmpl.itemStats.emplace_back(stat);
					}
				}
			}

			CCLOG("ObjectMgr: Loaded %zu item templates.", m_itemTemplateStore.size());
		}
		catch (std::exception& e)
		{
			CCLOG("ObjectMgr: Load item template failed. error: %s", e.what());
			return false;
		}

		try
		{
			SQLite::Statement query(db, "SELECT * FROM item_application_template");

			while (query.executeStep())
			{
				uint32 id = query.getColumn("id");
				ItemApplicationTemplate& tmpl = m_itemApplicationTemplateStore[id];
				tmpl.id = id;
				tmpl.flags = query.getColumn("flags");
				tmpl.visualId = query.getColumn("visual_id");
				tmpl.duration = query.getColumn("duration");
				tmpl.recoveryTime = query.getColumn("recovery_time");
//				int32 index = query.getColumnIndex("effect_type1");
				for (int32 i = 0; i < MAX_ITEM_EFFECTS; ++i)
				{
					ItemEffect effect;
					effect.type = query.getColumn(StringUtils::format("effect_type%d", (i + 1)).c_str());
					if (effect.type != ITEM_EFFECT_NONE)
					{
						effect.value = query.getColumn(StringUtils::format("effect_value%d", (i + 1)).c_str());
						tmpl.effects.emplace_back(effect);
					}
				}
			}

			CCLOG("ObjectMgr: Loaded %zu item application templates.", m_itemApplicationTemplateStore.size());
		}
		catch (std::exception& e)
		{
			CCLOG("ObjectMgr: Load item application template failed. error: %s", e.what());
			return false;
		}

		return true;
	}
	catch (std::exception const& ex)
	{
		CCLOG("ObjectMgr: Failed to load DB file %s. error: %s", dbStoragePath.c_str(), ex.what());
	}

	return false;
}

std::string ObjectMgr::getDBFilePath(std::string const& dbFilename) const
{
	return StringUtils::format("odb/%s", dbFilename.c_str());
}

NS_END
