#ifndef __SHOP_MGR_H__
#define __SHOP_MGR_H__

#include "SQLiteCpp/SQLiteCpp.h"

#include "common/Common.h"
#include "entities/DataUnit.h"

NS_BEGIN

enum GameCoinID
{
	GAME_COIN_NONE					= 0,
	GAME_COIN_PILE_OF_GOLDS			= 1,
	GAME_COIN_BAG_OF_GOLDS			= 2,
	GAME_COIN_SACK_OF_GOLDS		    = 3,
	GAME_COIN_BOX_OF_GOLDS			= 4,
	GAME_COIN_FREE_GOLDS			= 5,
};

enum TemplateType
{
	TEMPLATE_TYPE_NONE,
	TEMPLATE_TYPE_HERO,
	TEMPLATE_TYPE_GAME_COIN,
	TEMPLATE_TYPE_REMOVE_ADS,
};

enum ProductType
{
	PRODUCT_TYPE_PRIZE 			    = 0, // Products that are free or can be purchased with game currency
	PRODUCT_TYPE_NON_CONSUMABLE		= 1, // One-time purchase of non-consumable products
	PRODUCT_TYPE_CONSUMABLE			= 2, // One-time purchase of consumable products
	PRODUCT_TYPE_SUBSCRIPTION		= 3, // Subscription products
};

enum DailyRewardType
{
	DAILY_REWARD_TYPE_NONE,
	DAILY_REWARD_TYPE_HERO,
	DAILY_REWARD_TYPE_GOLD,
};

extern std::unordered_map<int32 /* GameCoinID */, std::string/* ProductID */> g_gameCoinProductIds;
extern std::unordered_map<uint32 /* HeroID */, std::string/* ProductID */> g_heroProductIds;
extern std::string g_removeAdsProductId;

struct ProductTemplate
{
	ProductTemplate() :
		name(""),
		productId(""),
		productType(PRODUCT_TYPE_PRIZE)
	{
	}

	std::string name;
	std::string productId;
	ProductType productType;
};

typedef std::array<int32, MAX_STAT_TYPES> StatUpgradePrices;

struct HeroTemplate: ProductTemplate
{

	HeroTemplate() :
		id(HERO_NONE),
		price(0)
	{
	}

	static const HeroTemplate Null;

	bool isNull() const { return id == HERO_NONE; }
	bool isFree() const { return productType == PRODUCT_TYPE_PRIZE && price <= 0; }


	// If the price is 0, you cannot upgrade to the stat value of this stage.
	int32 getStatUpgradePrice(uint8 stage, StatType type) const
	{
		if (stage < statUpgradePricesList.size())
			return statUpgradePricesList[stage][type];
		else
			return 0;
	}

	int32 getNextStageUpgradePrice(uint8 currStage, StatType type) const
	{
		uint8 newStage = currStage + 1;
		if (newStage < statUpgradePricesList.size())
			return statUpgradePricesList[newStage][type];
		else
			return 0;
	}

	HeroID id;
	int32 price;
	std::vector<StatUpgradePrices> statUpgradePricesList;
};

struct StageGameCoin
{
	StageGameCoin() :
		amount(0),
		waitTime(0)
	{
	}

	int32 amount;
	int32 waitTime; // Stage waiting time. Unit: minutes
};

struct GameCoinTemplate: ProductTemplate
{
	GameCoinTemplate() :
			id(GAME_COIN_NONE),
			amount(0)
	{
	}

	static const GameCoinTemplate Null;

	bool isNull() const { return id == GAME_COIN_NONE; }

	GameCoinID id;
	int32 amount;
	std::vector<StageGameCoin> stageRewardList;
};

struct IAPItem
{
	IAPItem() :
		productType(PRODUCT_TYPE_NON_CONSUMABLE),
		templateType(TEMPLATE_TYPE_NONE),
		templatePos(-1)
	{
	}
	static const IAPItem Null;

	bool isNull() const { return templateType == TEMPLATE_TYPE_NONE; }

	ProductType productType;
	TemplateType templateType;
	int32 templatePos;
};

struct DailyRewardTemplate
{
	DailyRewardTemplate() :
		day(0),
		rewardType1(DAILY_REWARD_TYPE_NONE),
		value1(0),
		rewardType2(DAILY_REWARD_TYPE_NONE),
		value2(0)
	{
	}

	static const DailyRewardTemplate Null;

	bool isNull() const { return rewardType1 == DAILY_REWARD_TYPE_NONE && rewardType2 == DAILY_REWARD_TYPE_NONE; }

	int32 day;
	DailyRewardType rewardType1;
	int32 value1;
	DailyRewardType rewardType2;
	int32 value2;
	bool isMilestone;
};

class ShopMgr
{
public:
	static ShopMgr* instance();

	void loadAsync(std::function<void(bool)> callback = nullptr);
	bool load();

	std::vector<HeroTemplate> const* getHeroTemplateList() const { return &m_heroTemplateList; }
	HeroTemplate const* getHeroTemplateByProductId(std::string const& productId) const;
	HeroTemplate const* getHeroTemplateByHeroId(uint32 heroId) const;

	std::vector<GameCoinTemplate> const* getGameCoinTemplateList() const { return &m_gameCoinTemplateList; }
	GameCoinTemplate const* getGameCoinTemplateByGameCoinId(GameCoinID gameCoinId) const;
	GameCoinTemplate const* getGameCoinTemplateByProductId(std::string const& productId) const;

	ProductTemplate const* getRemoveAdsTemplate() const { return &m_removeAdsTemplate;  }

	std::vector<DailyRewardTemplate> const* getDailyRewardTemplateList() const { return &m_dailyRewardTemplateList; }
	DailyRewardTemplate const* getDailyRewardTemplateByDay(int32 day) const;

	IAPItem const* getIAPItem(std::string const& productId) const;
	std::vector<std::string> getProductIdList() const;

private:
	ShopMgr();
	~ShopMgr();

	bool loadHeroTemplate(SQLite::Database& db);
	bool loadShopTemplate(SQLite::Database& db);
	bool loadDailyRewardTemplate(SQLite::Database& db);

	void initRemoveAdsTemplate();

	bool m_isLoading;
	std::unordered_map<std::string, IAPItem> m_iapItems;
	std::vector<HeroTemplate> m_heroTemplateList;
	std::vector<GameCoinTemplate> m_gameCoinTemplateList;
	ProductTemplate m_removeAdsTemplate;
	std::vector<DailyRewardTemplate> m_dailyRewardTemplateList;
};

#define sShopMgr ShopMgr::instance()


NS_END

#endif // __SHOP_MGR_H__
