//
//  HeroStatLayer.h
//  snowfight
//
//  Created by Luthier on 2020/06/23.
//

#ifndef __HERO_STAT_LAYER_H__
#define __HERO_STAT_LAYER_H__

#include "common/Common.h"
#include "StatUpgradePanel.h"
#include "StatDetailPanel.h"

USING_NS_CC;


NS_BEGIN

class HeroStatLayer: public LayerColor
{
public:
	enum StatUpgradeAction
	{
		STAT_UPGRADE_ACTION_UPGRADED,
		STAT_UPGRADE_ACTION_BUY_OK,
		STAT_UPGRADE_ACTION_BUY_CANCEL,
	};

	enum VisibleState
	{
		VISIBLE_STATE_SHOWN,
		VISIBLE_STATE_UPDATED,
		VISIBLE_STATE_WILL_HIDE,
	};

	typedef std::function<void(Ref*, StatUpgradeAction)> StatUpgradedCallback;
	typedef std::function<void(Ref*, VisibleState)> VisibleStateChangedCallback;

    static HeroStatLayer* create(cocos2d::Size const& contentSize);
    
    bool init(cocos2d::Size const& contentSize);
    
	HeroStatLayer();
    ~HeroStatLayer();

	void show(HeroID heroId);
	bool isShown() const { return m_shownHeroId != HERO_NONE; }
	void hide();

	void updateUpgradePanel();

	void setPanelCenteredSpacing(float spacing) { m_panelCenteredSpacing = spacing; }
	void setPanelBottom(float bottom);

	void onEnter() override;

	void setRecommendedUpgradeType(StatUpgradeType type);
	void resetRecommendedUpgradeTypes();
	bool isAnyOfUpgradeTypesRecommended() const;

	ui::Button* getUpgradeButton(StatUpgradeType type) const;

	void setStatUpgradedEventListener(StatUpgradedCallback const& callback) { m_statUpgradedCallback = callback; }
	void setVisibleStateChangedEventListener(VisibleStateChangedCallback const& callback) { m_visibleStateChangedCallback = callback; }
	void setUpgradePanelScrollEventListener(StatUpgradePanel::ListScrollCallback const& callback) { m_upgradePanel->setListScrollEventListener(callback); }

private:
	void slideInUpgrade(HeroID heroId);
	void slideOutUpgrade();

	void slideInDetail(HeroID heroId);
	void slideOutDetail();

	void updatePanelPosition();

	void statUpgradedCallback(Ref* sender, StatUpgradePanel::UpgradeState state, StatUpgradeData const& data);

	float m_panelCenteredSpacing;
	float m_panelBottom;
	HeroID m_shownHeroId;
	float m_slidingDuration;

	StatUpgradedCallback m_statUpgradedCallback;
	VisibleStateChangedCallback m_visibleStateChangedCallback;

	StatUpgradePanel* m_upgradePanel;
	StatDetailPanel* m_detailPanel;
};


NS_END


#endif // __HERO_STAT_LAYER_H__

