//
//  StatUpgradePanel.h
//  snowfight
//
//  Created by Luthier on 2020/06/23.
//

#ifndef __STAT_UPGRADE_PANEL_H__
#define __STAT_UPGRADE_PANEL_H__

#include <bitset>

#include "ui/UIListView.h"

#include "common/Common.h"
#include "game/ShopMgr.h"
#include "StatUpgradeItem.h"

USING_NS_CC;

NS_BEGIN


class StatUpgradePanel : public LayerColor
{
public:
	enum UpgradeState
	{
		UPGRADE_STATE_UPGRADED,
		UPGRADE_STATE_NOT_ENOUGH_GOLDS,
	};

	enum ListScrollEventType
	{
		LIST_SCROLLING,
		LIST_SCROLLING_BEGAN,
		LIST_SCROLLING_ENDED,
	};

	typedef std::function<void(Ref*, UpgradeState, StatUpgradeData const&)> StatUpgradedCallback;
	typedef std::function<void(Ref*, ListScrollEventType)> ListScrollCallback;

	static StatUpgradePanel* create();

	bool init() override;

	StatUpgradePanel();
	~StatUpgradePanel();

	void setHeroTemplate(HeroTemplate const* tmpl);
	void updatePanel();
	void setStatUpgradeEventListener(StatUpgradedCallback const& callback) { m_statUpgradedCallback = callback; }
	void setListScrollEventListener(ListScrollCallback const& callback) { m_listScrollCallback = callback; }

	void setRecommendedUpgradeType(StatUpgradeType type) { m_recommendedUpgradeTypes.set(type); }
	void resetRecommendedUpgradeTypes() { m_recommendedUpgradeTypes.reset(); }
	bool isAnyOfUpgradeTypesRecommended() const { return m_recommendedUpgradeTypes.any(); }

	ui::Button* getUpgradeButton(StatUpgradeType type) const;

	void stopAnimation();

private:
	void reloadData();
	void updateListItems();
	ui::Widget* dequeueItem();
	void selectedItemEvent(Ref* pSender, ui::ListView::EventType type);
	void scrollListEvent(Ref* pSender, ui::ScrollView::EventType type);
	bool canUpgradeStat(HeroTemplate const* tmpl, StatType type) const;

	StatUpgradeData createHealthUpgradeData(HeroTemplate const* tmpl, bool isLocked);
	void updateHealthUpgradeData(StatUpgradeData& data, HeroTemplate const* tmpl);

	StatUpgradeData createDamageUpgradeData(HeroTemplate const* tmpl, bool isLocked);
	void updateDamageUpgradeData(StatUpgradeData& data, HeroTemplate const* tmpl);

	StatUpgradeData createMoveSpeedUpgradeData(HeroTemplate const* tmpl, bool isLocked);
	void updateMoveSpeedUpgradeData(StatUpgradeData& data, HeroTemplate const* tmpl);

	StatUpgradeData createAttackRangeUpgradeData(HeroTemplate const* tmpl, bool isLocked);
	void updateAttackRangeUpgradeData(StatUpgradeData& data, HeroTemplate const* tmpl);

	void setLocalPlayerStatStage(StatUpgradeData const& data, uint8 stage);
	void updateStatUpgradeData(StatUpgradeData& data);
	void upgradeStat(StatUpgradeItem* item, StatUpgradeData& data);

	HeroTemplate const* m_template;
	std::bitset<MAX_STAT_UPGRADE_TYPES> m_recommendedUpgradeTypes;
	std::vector<StatUpgradeData> m_dataSet;

	StatUpgradedCallback m_statUpgradedCallback;
	ListScrollCallback m_listScrollCallback;
	Label* m_nameLabel;
	ui::ListView* m_listView;
	Vector<ui::Widget*> m_freedItems;
};


NS_END


#endif // __STAT_UPGRADE_PANEL_H__

