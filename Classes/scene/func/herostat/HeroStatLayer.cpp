//
//  HeroStatLayer.cpp
//  snowfight
//
//  Created by Luthier on 2020/06/23.
//

#include "HeroStatLayer.h"

#include "game/UserPreferences.h"
#include "scene/gui/ModalDialog.h"
#include "scene/review/StoreReview.h"

NS_BEGIN

#define ACTION_TAG_SLIDING_UPGRADE			0
#define ACTION_TAG_SLIDING_DETAIL			0

#define SLIDING_DISTANCE_PER_SECOND		438.f

HeroStatLayer* HeroStatLayer::create(Size const& contentSize)
{
    auto ret = new (std::nothrow) HeroStatLayer();
    if (ret && ret->init(contentSize))
    {
        ret->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(ret);
    }
    
    return ret;
}

bool HeroStatLayer::init(Size const& contentSize)
{
    if (!LayerColor::initWithColor(Color4B(0, 0, 0, 0)))
        return false;

	this->setContentSize(contentSize);

	// Statistic upgrade
	m_upgradePanel = StatUpgradePanel::create();
	m_upgradePanel->setAnchorPoint(Point::ANCHOR_BOTTOM_RIGHT);
	m_upgradePanel->setStatUpgradeEventListener(CC_CALLBACK_3(HeroStatLayer::statUpgradedCallback, this));
	m_upgradePanel->setVisible(false);
	this->addChild(m_upgradePanel);

	// Statistic detail
	m_detailPanel = StatDetailPanel::create();
	m_detailPanel->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_detailPanel->setVisible(false);
	this->addChild(m_detailPanel);

	this->updatePanelPosition();

    return true;
}

HeroStatLayer::HeroStatLayer() :
	m_panelCenteredSpacing(0),
	m_panelBottom(0),
	m_shownHeroId(HERO_NONE),
	m_slidingDuration(0),
	m_upgradePanel(nullptr),
	m_detailPanel(nullptr)
{
    
}

HeroStatLayer::~HeroStatLayer()
{
	m_upgradePanel = nullptr;
	m_detailPanel = nullptr;
}

void HeroStatLayer::show(HeroID heroId)
{
	if (m_shownHeroId == heroId)
		return;

	HeroTemplate const* tmpl = sShopMgr->getHeroTemplateByHeroId(heroId);
	if (tmpl->id == HERO_NONE)
		return;

	m_upgradePanel->setHeroTemplate(tmpl);
	m_detailPanel->setHeroTemplate(tmpl);

	bool isShowing = false;
	if (m_shownHeroId == HERO_NONE)
	{
		float offX = this->getContentSize().width / 2 - m_panelCenteredSpacing / 2;
		m_slidingDuration = offX / SLIDING_DISTANCE_PER_SECOND;

		this->slideInUpgrade(heroId);
		this->slideInDetail(heroId);
		isShowing = true;
	}

	m_shownHeroId = heroId;

	if (!isShowing)
	{
		if (m_visibleStateChangedCallback)
			m_visibleStateChangedCallback(this, VISIBLE_STATE_UPDATED);
	}
}

void HeroStatLayer::hide()
{
	if (m_shownHeroId == HERO_NONE)
		return;

	m_shownHeroId = HERO_NONE;

	m_upgradePanel->stopAnimation();
	m_detailPanel->stopAnimation();

	this->slideOutUpgrade();
	this->slideOutDetail();
}

void HeroStatLayer::updateUpgradePanel()
{
	m_upgradePanel->updatePanel();
}

void HeroStatLayer::setPanelBottom(float bottom)
{
	if (m_panelBottom != bottom)
	{
		m_panelBottom = bottom;
		this->updatePanelPosition();
	}
}

void HeroStatLayer::onEnter()
{
	Node::onEnter();
}

void HeroStatLayer::setRecommendedUpgradeType(StatUpgradeType type)
{
	m_upgradePanel->setRecommendedUpgradeType(type);
}

void HeroStatLayer::resetRecommendedUpgradeTypes()
{
	m_upgradePanel->resetRecommendedUpgradeTypes();
}

bool HeroStatLayer::isAnyOfUpgradeTypesRecommended() const
{
	return m_upgradePanel->isAnyOfUpgradeTypesRecommended();
}

ui::Button* HeroStatLayer::getUpgradeButton(StatUpgradeType type) const
{
	return m_upgradePanel->getUpgradeButton(type);
}

void HeroStatLayer::slideInUpgrade(HeroID heroId)
{
	if (Action* action = m_upgradePanel->getActionByTag(ACTION_TAG_SLIDING_UPGRADE))
		m_upgradePanel->stopAction(action);

	EaseCubicActionOut* move = EaseCubicActionOut::create(MoveTo::create(m_slidingDuration, Vec2(this->getContentSize().width / 2 - m_panelCenteredSpacing / 2, m_upgradePanel->getPositionY())));
	auto* callFunc = CallFunc::create([this]() {
		if (m_visibleStateChangedCallback)
			m_visibleStateChangedCallback(this,VISIBLE_STATE_SHOWN);
	});
	Action* action = Sequence::create(Show::create(), move, callFunc, nullptr);
	action->setTag(ACTION_TAG_SLIDING_UPGRADE);
	m_upgradePanel->runAction(action);
}

void HeroStatLayer::slideOutUpgrade()
{
	if (Action* action = m_upgradePanel->getActionByTag(ACTION_TAG_SLIDING_UPGRADE))
		m_upgradePanel->stopAction(action);

	auto* callFunc = CallFunc::create([this]() {
		if (m_visibleStateChangedCallback)
			m_visibleStateChangedCallback(this, VISIBLE_STATE_WILL_HIDE);
	});
	EaseCubicActionIn* move = EaseCubicActionIn::create(MoveTo::create(m_slidingDuration, Vec2(0, m_upgradePanel->getPositionY())));
	Action* action = Sequence::create(callFunc, move, Hide::create(), nullptr);
	action->setTag(ACTION_TAG_SLIDING_UPGRADE);
	m_upgradePanel->runAction(action);
}

void HeroStatLayer::slideInDetail(HeroID heroId)
{
	if (Action* action = m_detailPanel->getActionByTag(ACTION_TAG_SLIDING_UPGRADE))
		m_detailPanel->stopAction(action);

	EaseCubicActionOut* move = EaseCubicActionOut::create(MoveTo::create(m_slidingDuration, Vec2(this->getContentSize().width / 2 + m_panelCenteredSpacing / 2, m_detailPanel->getPositionY())));
	Action* action = Sequence::create(Show::create(), move, nullptr);
	action->setTag(ACTION_TAG_SLIDING_DETAIL);
	m_detailPanel->runAction(action);

}

void HeroStatLayer::slideOutDetail()
{
	if (Action* action = m_detailPanel->getActionByTag(ACTION_TAG_SLIDING_UPGRADE))
		m_detailPanel->stopAction(action);

	EaseCubicActionIn* move = EaseCubicActionIn::create(MoveTo::create(m_slidingDuration, Vec2(this->getContentSize().width, m_detailPanel->getPositionY())));
	Action* action = Sequence::create(move, Hide::create(), nullptr);
	action->setTag(ACTION_TAG_SLIDING_DETAIL);
	m_detailPanel->runAction(action);
}

void HeroStatLayer::updatePanelPosition()
{
	m_upgradePanel->setPosition(0, m_panelBottom);
	m_detailPanel->setPosition(this->getContentSize().width, m_panelBottom);
}

void HeroStatLayer::statUpgradedCallback(Ref* sender, StatUpgradePanel::UpgradeState state ,StatUpgradeData const& data)
{
	switch (state)
	{
	case StatUpgradePanel::UPGRADE_STATE_UPGRADED:
		m_detailPanel->updateStats();

		if (!sStoreReview->isProcessCompleted(REVIEW_PROCESS_UPGRADE_STATS_OR_UNLOCK_HERO))
		{
			int32 count = sUserPreferences->getUpgradeStatsCount() + 1;
			if (count >= REVIEW_UPGRADE_STAT_TIMES)
				sStoreReview->setProcessCompleted(REVIEW_PROCESS_UPGRADE_STATS_OR_UNLOCK_HERO);
			sUserPreferences->setUpgradeStatCount(count);
		}

		if (m_statUpgradedCallback)
			m_statUpgradedCallback(this, STAT_UPGRADE_ACTION_UPGRADED);
		break;
	case StatUpgradePanel::UPGRADE_STATE_NOT_ENOUGH_GOLDS:
	{
		MessageDialog* dialog = MessageDialog::create();
		dialog->addPositiveButton(sLocaleMgr->getString("herostat_dlg_btn_buy"), [this](Ref* sender) {
			if (m_statUpgradedCallback)
				m_statUpgradedCallback(this, STAT_UPGRADE_ACTION_BUY_OK);
		});
		dialog->addCancelButton([this](Ref* sender) {
			if (m_statUpgradedCallback)
				m_statUpgradedCallback(this, STAT_UPGRADE_ACTION_BUY_CANCEL);
		});
		dialog->setTitle(sLocaleMgr->getString("herostat_dlg_title_not_enough_gold"));
		dialog->setMessage(sLocaleMgr->getString("herostat_dlg_msg_not_enough_gold"));
		dialog->show();
		break;
	}
	default:
		break;
	}
}


NS_END