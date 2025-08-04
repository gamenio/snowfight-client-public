#include "ShopMgr.h"

#include "common/utils/TimeUtil.h"
#include "game/firservice/FirebaseService.h"
#include "LocaleMgr.h"
#include "UserPreferences.h"
#include "utils/DBUtils.h"

NS_BEGIN

// Shop database configuration
#define SHOP_DB_VERSION			2
#define SHOP_DB_FILE			"shop.db"
// Database key
char const* DB_KEY = "4TCbLDNQVVmbpmXo0McAz4we1fEfKDyb";

// In-app purchase product ID
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
std::unordered_map<int32, std::string> g_gameCoinProductIds = {
    { GAME_COIN_PILE_OF_GOLDS,      "io.gamen.snowfight.ios.pile_of_golds"  },
    { GAME_COIN_BAG_OF_GOLDS,       "io.gamen.snowfight.ios.bag_of_golds"   },
    { GAME_COIN_SACK_OF_GOLDS,      "io.gamen.snowfight.ios.sack_of_golds"  },
    { GAME_COIN_BOX_OF_GOLDS,       "io.gamen.snowfight.ios.box_of_golds"   },
};
#else
std::unordered_map<int32, std::string> g_gameCoinProductIds = {
    { GAME_COIN_PILE_OF_GOLDS,      "io.gamen.snowfight.pile_of_golds"  },
    { GAME_COIN_BAG_OF_GOLDS,       "io.gamen.snowfight.bag_of_golds"   },
    { GAME_COIN_SACK_OF_GOLDS,      "io.gamen.snowfight.sack_of_golds"  },
    { GAME_COIN_BOX_OF_GOLDS,       "io.gamen.snowfight.box_of_golds"   },
};
#endif
std::unordered_map<uint32, std::string> g_heroProductIds = {
    { HERO_PENGUIN,                 "io.gamen.snowfight.penguin"  },
    { HERO_RABBIT,                  "io.gamen.snowfight.rabbit"   },
};
std::string g_removeAdsProductId = "io.gamen.snowfight.remove_ads";

const HeroTemplate HeroTemplate::Null;
const GameCoinTemplate GameCoinTemplate::Null;
const IAPItem IAPItem::Null;
const DailyRewardTemplate DailyRewardTemplate::Null;

ShopMgr* ShopMgr::instance()
{
	static ShopMgr instance;
	return &instance;
}

void ShopMgr::loadAsync(std::function<void(bool)> callback)
{
	if (m_isLoading)
	{
		CCLOG("ShopMgr: Shop data is already loading.");
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
				sAnalytics->logException("shop", "Load data failed");
		});
	});
}

bool ShopMgr::load()
{
	std::string dbStoragePath;
	if (DBUtils::updateDBIfNeeded(sUserPreferences->getShopDBVersion(), SHOP_DB_VERSION, SHOP_DB_FILE, dbStoragePath))
		sUserPreferences->setShopDBVersion(SHOP_DB_VERSION);

	if (dbStoragePath.empty())
	{
		CCLOG("ShopMgr: Failed to update DB file %s. ", SHOP_DB_FILE);
		return false;
	}

	NSTime currTime = time_util::getUptimeMillis();
	try
	{
		SQLite::Database db(dbStoragePath);
		db.key(DB_KEY);

		if (!this->loadHeroTemplate(db))
			return false;

		if (!this->loadShopTemplate(db))
			return false;

		if (!this->loadDailyRewardTemplate(db))
			return false;
	}
	catch (std::exception const& ex)
	{
		CCLOG("ShopMgr: Failed to load DB file %s. error: %s", dbStoragePath.c_str(), ex.what());
		return false;
	}

	NSTime diff = time_util::getUptimeMillis() - currTime;
    NS_UNUSED_VARIABLE(diff);
	CCLOG("ShopMgr: Shop data load time: %dms.", diff);

	return true;
}

HeroTemplate const* ShopMgr::getHeroTemplateByProductId(std::string const& productId) const
{
	auto it = m_iapItems.find(productId);
	if (it != m_iapItems.end())
	{
		auto const& item = (*it).second;
		if (item.templateType == TEMPLATE_TYPE_HERO)
        {
		    if(item.templatePos >= 0)
                return &m_heroTemplateList[item.templatePos];
        }
	}

	return &HeroTemplate::Null;
}

HeroTemplate const* ShopMgr::getHeroTemplateByHeroId(uint32 heroId) const
{
	if (heroId != HERO_NONE)
	{
		for (auto it = m_heroTemplateList.begin(); it != m_heroTemplateList.end(); ++it)
		{
			HeroTemplate const& tmpl = *it;
			if (tmpl.id == heroId)
				return &tmpl;
		}
	}

	return &HeroTemplate::Null;
}

GameCoinTemplate const* ShopMgr::getGameCoinTemplateByGameCoinId(GameCoinID gameCoinId) const
{
	for (auto it = m_gameCoinTemplateList.begin(); it != m_gameCoinTemplateList.end(); ++it)
	{
		if ((*it).id == gameCoinId)
		{
			return &(*it);
		}
	}
	return &GameCoinTemplate::Null;
}

GameCoinTemplate const* ShopMgr::getGameCoinTemplateByProductId(std::string const& productId) const
{
	auto it = m_iapItems.find(productId);
	if (it != m_iapItems.end())
	{
		auto const& item = (*it).second;
		if(item.templateType == TEMPLATE_TYPE_GAME_COIN)
        {
		    if(item.templatePos >= 0)
                return &m_gameCoinTemplateList[item.templatePos];
        }
	}

	return &GameCoinTemplate::Null;
}

DailyRewardTemplate const* ShopMgr::getDailyRewardTemplateByDay(int32 day) const
{
	if(day > 0 && day <= (int32)m_dailyRewardTemplateList.size())
		return &m_dailyRewardTemplateList[day - 1];

	return  &DailyRewardTemplate::Null;
}

IAPItem const* ShopMgr::getIAPItem(std::string const& productId) const
{
	auto it = m_iapItems.find(productId);
	if (it != m_iapItems.end())
		return &(*it).second;

	return &IAPItem::Null;
}

std::vector<std::string> ShopMgr::getProductIdList() const
{
	std::vector<std::string> productIdList;
	for (auto const& item : m_iapItems)
	{
		productIdList.emplace_back(item.first);
	}

	return productIdList;
}

ShopMgr::ShopMgr() :
	m_isLoading(false)
{
	this->initRemoveAdsTemplate();
}

ShopMgr::~ShopMgr()
{
}

bool ShopMgr::loadHeroTemplate(SQLite::Database& db)
{
	try
	{
		SQLite::Statement query(db, "SELECT * FROM hero_template ORDER BY display_order ASC");
        int32 index = 0;
		while (query.executeStep())
		{
			HeroTemplate tmpl;
			HeroID id = (HeroID)query.getColumn("id").getUInt();
			tmpl.id = id;
			tmpl.name = sLocaleMgr->getHeroName(id);
			tmpl.productType = (ProductType)query.getColumn("product_type").getInt();
			tmpl.price = query.getColumn("price");
			if (tmpl.productType != PRODUCT_TYPE_PRIZE)
			{
                auto it = g_heroProductIds.find(id);
                NS_ASSERT_LOG(it != g_heroProductIds.end(), "The product ID for the IAP item was not found.");
                
                tmpl.productId = (*it).second;
                IAPItem& item = m_iapItems[tmpl.productId];
                item.templateType = TEMPLATE_TYPE_HERO;
                item.productType = tmpl.productType;
                item.templatePos = index;
			}
            m_heroTemplateList.push_back(tmpl);
            index++;
		}
	}
	catch (std::exception const& ex)
	{
		CCLOG("ShopMgr: Load hero template failed. error: %s", ex.what());
		return false;
	}

	try
	{
		for (auto it = m_heroTemplateList.begin(); it != m_heroTemplateList.end(); ++it)
		{
			HeroTemplate& tmpl = *it;

			SQLite::Statement maxQuery(db, "SELECT max(stage) FROM hero_stats_upgrade WHERE template_id=:template_id;");
			maxQuery.bind(":template_id", tmpl.id);
			if (maxQuery.executeStep())
				tmpl.statUpgradePricesList.resize(maxQuery.getColumn(0).getInt() + 1);

			uint8 nStages = (uint8)tmpl.statUpgradePricesList.size();
			for (uint8 stage = 0; stage < nStages; ++stage)
			{
				SQLite::Statement query(db, "SELECT * FROM hero_stats_upgrade WHERE template_id=:template_id AND stage=:stage");
				query.bind(":template_id", tmpl.id);
				query.bind(":stage", stage);
				while (query.executeStep())
				{
					int32 statType = query.getColumn("stat_type");
					int32 price = query.getColumn("price");
					tmpl.statUpgradePricesList[stage][statType] = price;
				}
			}
		}
	}
	catch (std::exception const& ex)
	{
		CCLOG("ShopMgr: Load hero stats upgrade failed. error: %s", ex.what());
		return false;
	}


	return true;
}

bool ShopMgr::loadShopTemplate(SQLite::Database& db)
{
	try
	{
		SQLite::Statement query(db, "SELECT * FROM game_coin_template ORDER BY display_order ASC");
        int32 index = 0;
		while (query.executeStep())
		{
			GameCoinTemplate tmpl;
			GameCoinID id = (GameCoinID)query.getColumn("id").getInt();
			tmpl.id = id;
			tmpl.name = sLocaleMgr->getGameCoinName(id);
			tmpl.amount = query.getColumn("amount");
			tmpl.productType = (ProductType)query.getColumn("product_type").getInt();
			if (tmpl.productType != PRODUCT_TYPE_PRIZE)
			{
                auto it = g_gameCoinProductIds.find(id);
                NS_ASSERT_LOG(it != g_gameCoinProductIds.end(), "The product ID for the IAP item was not found.");

                tmpl.productId = (*it).second;
                IAPItem& item = m_iapItems[tmpl.productId];
                item.templateType = TEMPLATE_TYPE_GAME_COIN;
                item.productType = tmpl.productType;
                item.templatePos = index;
			}
            m_gameCoinTemplateList.push_back(tmpl);
            index++;
		}
	}
	catch (std::exception const& ex)
	{
		CCLOG("ShopMgr: Load game coin template failed. error: %s", ex.what());
		return false;
	}

	try
	{
		for (auto it = m_gameCoinTemplateList.begin(); it != m_gameCoinTemplateList.end(); ++it)
		{
			GameCoinTemplate& tmpl = *it;
			if (tmpl.productType == PRODUCT_TYPE_PRIZE)
			{
				SQLite::Statement countQuery(db, "SELECT count(*) FROM game_coin_stage_rewards WHERE template_id=:template_id");
				countQuery.bind(":template_id", tmpl.id);
				if (countQuery.executeStep())
					tmpl.stageRewardList.resize(countQuery.getColumn(0).getInt());

				SQLite::Statement query(db, "SELECT * FROM game_coin_stage_rewards WHERE template_id=:template_id");
				query.bind(":template_id", tmpl.id);
				while (query.executeStep())
				{
					uint8 stage = query.getColumn("stage");
					tmpl.stageRewardList[stage].amount = query.getColumn("amount").getInt();
					tmpl.stageRewardList[stage].waitTime = query.getColumn("wait_time").getInt();
				}
			}
		}
	}
	catch (std::exception const& ex)
	{
		CCLOG("ShopMgr: Load game coin stage rewards failed. error: %s", ex.what());
		return false;
	}

	return true;
}


bool ShopMgr::loadDailyRewardTemplate(SQLite::Database& db)
{
	try
	{
		SQLite::Statement countQuery(db, "SELECT max(day) FROM daily_reward_template;");
		if (countQuery.executeStep())
			m_dailyRewardTemplateList.resize(countQuery.getColumn(0).getInt());

		SQLite::Statement query(db, "SELECT * FROM daily_reward_template ORDER BY day ASC");
		while (query.executeStep())
		{
			int32 day = query.getColumn("day");;
			DailyRewardTemplate& tmpl = m_dailyRewardTemplateList[day - 1];
			tmpl.day = day;
			tmpl.rewardType1 = (DailyRewardType)query.getColumn("reward_type1").getInt();
			tmpl.value1 = query.getColumn("value1");
			tmpl.rewardType2 = (DailyRewardType)query.getColumn("reward_type2").getInt();
			tmpl.value2 = query.getColumn("value2");
			tmpl.isMilestone = query.getColumn("milestone").getInt() == 1;
		}
	}
	catch (std::exception const& ex)
	{
		CCLOG("ShopMgr: Load daily reward template failed. error: %s", ex.what());
		return false;
	}

	return true;
}

void ShopMgr::initRemoveAdsTemplate()
{
	m_removeAdsTemplate.productId = g_removeAdsProductId;
	m_removeAdsTemplate.name = "Remove Ads";
	m_removeAdsTemplate.productType = PRODUCT_TYPE_NON_CONSUMABLE;
	IAPItem& item = m_iapItems[m_removeAdsTemplate.productId];
	item.templateType = TEMPLATE_TYPE_REMOVE_ADS;
	item.productType = m_removeAdsTemplate.productType;
}


NS_END