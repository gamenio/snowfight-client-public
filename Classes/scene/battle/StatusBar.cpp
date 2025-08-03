//
// StatusBars.cpp
// snowfight
// 
// Created by Luthier on 11/27/2019.
//

#include "StatusBar.h"

#include "game/entities/updates/ObjectUpdateFields.h"
#include "game/LocaleMgr.h"
#include "game/utils/UnitHelper.h"
#include "scene/review/StoreReview.h"
#include "scene/Utils.h"
#include "scene/SoundMgr.h"

using namespace cocos2d::ui;

NS_BEGIN

#define FRAMENAME_BACKGROUND			"statusbars_bg.png"
#define FRAMENAME_HEAD_BG				"statusbars_head_bg.png"
#define FRAMENAME_HEAD_MASK				"statusbars_head_mask.png"
#define FRAMENAME_MONEYBAG				"statusbars_moneybag.png"
#define FRAMENAME_XP_PROGRESS_FG		"statusbars_xp_progress_fg.png"
	
#define HEAD_FRAME_FORMAT				"hero_head%d.png"

#define MONEY_BAG_SIZE					Size(59, 34)

#define FULL_PERCENTAGE					100

#define ACTION_TAG_MONEYBAG					0

StatusBar* StatusBar::create()
{
    auto ret = new (std::nothrow) StatusBar();
    if (ret && ret->init())
    {
        ret->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(ret);
    }
    
    return ret;
}

StatusBar::StatusBar() :
	m_data(nullptr),
	m_headSp(nullptr),
	m_levelLabel(nullptr),
	m_xpProg(nullptr),
	m_xpLabel(nullptr),
	m_moneybagSp(nullptr),
	m_moneyLabel(nullptr)
{
}

StatusBar::~StatusBar()
{
	CC_SAFE_RELEASE_NULL(m_data);

	m_headSp = nullptr;
	m_levelLabel = nullptr;
	m_xpProg = nullptr;
	m_xpLabel = nullptr;

	m_moneybagSp = nullptr;
	m_moneyLabel = nullptr;

}

bool StatusBar::init()
{
	if (!Node::init())
		return false;

	Sprite* headBg = Sprite::createWithSpriteFrameName(FRAMENAME_HEAD_BG);
	headBg->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	headBg->setPosition(0, 0);
	this->addChild(headBg);

	Sprite* stencil = Sprite::createWithSpriteFrameName(FRAMENAME_HEAD_MASK);
	stencil->setAnchorPoint(headBg->getAnchorPoint());
	stencil->setPosition(headBg->getPosition());
	ClippingNode* headClipper = ClippingNode::create(stencil);
	headClipper->setAlphaThreshold(0.05f);
	this->addChild(headClipper);

	m_headSp = Sprite::createWithSpriteFrameName(StringUtils::format(HEAD_FRAME_FORMAT, HeroID::HERO_BOY));
	m_headSp->setAnchorPoint(stencil->getAnchorPoint());
	m_headSp->setPosition(stencil->getPosition());
	headClipper->addChild(m_headSp);

	Sprite* background = Sprite::createWithSpriteFrameName(FRAMENAME_BACKGROUND);
	background->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	this->addChild(background);

	this->setContentSize(background->getContentSize());

	this->initXPProgress();
	this->initMoneybag();

	return true;
}

void StatusBar::update(float delta)
{
	if (!m_data)
		return;

	if (m_data->hasUpdatedField(CPLAYER_FIELD_MONEY))
		this->setMoney(m_data->getMoney(), true);

	if (m_data->hasUpdatedField(CUNIT_FIELD_LEVEL))
		this->setLevel(m_data->getLevel());

	if (m_data->hasUpdatedField(CPLAYER_FIELD_EXPERIENCE) || m_data->hasUpdatedField(CPLAYER_FIELD_NEXTLEVEL_XP))
		this->setExperience(m_data->getExperience(), m_data->getNextLevelXP());
}

void StatusBar::setData(DataPlayer* data)
{
	CC_SAFE_RETAIN(data);
	CC_SAFE_RELEASE_NULL(m_data);
	m_data = data;

	this->setHead(m_data->getDisplayId());
	this->setMoney(m_data->getMoney(), false);

	this->setLevel(m_data->getLevel());
	this->setExperience(m_data->getExperience(), m_data->getNextLevelXP());
}

void StatusBar::initMoneybag()
{
	m_moneybagSp = Sprite::createWithSpriteFrameName(FRAMENAME_MONEYBAG);
	m_moneybagSp->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_moneybagSp->setContentSize(MONEY_BAG_SIZE);
	m_moneybagSp->setPosition(this->getContentSize().width - m_moneybagSp->getContentSize().width / 2, m_moneybagSp->getContentSize().height / 2);
	this->addChild(m_moneybagSp);

	m_moneyLabel = Label::createWithSystemFont("0", DEFAULT_SYSTEM_FONT, 10);
	m_moneyLabel->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_moneyLabel->setPosition(this->getContentSize().width / 2, this->getContentSize().height / 2 - 0.5f);
	m_moneyLabel->setTextColor(Color4B(232, 222, 214, 255));
	Utils::enableBoldForLabel(m_moneyLabel);
	this->addChild(m_moneyLabel);
}

void StatusBar::initXPProgress()
{
	Sprite* progFg = Sprite::createWithSpriteFrameName(FRAMENAME_XP_PROGRESS_FG);
	m_xpProg = ProgressTimer::create(progFg);
	m_xpProg->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_xpProg->setPosition(0, 0);
	m_xpProg->setType(ProgressTimer::Type::RADIAL);
	m_xpProg->setPercentage(80);
	this->addChild(m_xpProg);

	m_levelLabel = Label::createWithSystemFont(StringUtils::format(sLocaleMgr->getString("statusbars_level").c_str(), 0), DEFAULT_SYSTEM_FONT, 8);
	m_levelLabel->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
	m_levelLabel->setPosition(m_xpProg->getBoundingBox().getMidX(), 2.0f);
	m_levelLabel->setTextColor(Color4B(237, 235, 242, 255));
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID) || (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
	m_levelLabel->enableOutline(Color4B(0, 0, 0, GLubyte(255 * 0.28f)), 1);
#endif
	m_levelLabel->enableShadow(Color4B(0, 0, 0, GLubyte(255 * 0.5f)), Size(0.5f, -0.5f), 0);
	this->addChild(m_levelLabel);

#if NS_DEBUG
	m_xpLabel = Label::createWithSystemFont("0/0", DEFAULT_SYSTEM_FONT, 8);
	m_xpLabel->setAnchorPoint(Point::ANCHOR_MIDDLE_TOP);
	m_xpLabel->setPosition(m_xpProg->getBoundingBox().getMidX(), m_xpProg->getBoundingBox().getMinY());
	m_xpLabel->setTextColor(Color4B(237, 235, 242, 255));
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID) || (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
	m_xpLabel->enableOutline(Color4B(0, 0, 0, GLubyte(255 * 0.28f)), 1);
#endif
	m_xpLabel->enableShadow(Color4B(0, 0, 0, GLubyte(255 * 0.5f)), Size(0.5f, -0.5f), 0);
	this->addChild(m_xpLabel);
#endif // NS_DEBUG
}

void StatusBar::setHead(uint32 displayId)
{
	m_headSp->setSpriteFrame(StringUtils::format(HEAD_FRAME_FORMAT, displayId));
}

void StatusBar::setMoney(int32 money, bool animated)
{
	if(animated)
		this->startIncomeAnimation();

	m_moneyLabel->setString(StringUtils::format("%d", money));
}

void StatusBar::setExperience(int32 xp, int32 nextLevelXP)
{
	if(nextLevelXP > 0)
		m_xpProg->setPercentage(((float)xp / nextLevelXP) * FULL_PERCENTAGE);
	else
		m_xpProg->setPercentage(FULL_PERCENTAGE);

#if NS_DEBUG
	m_xpLabel->setString(StringUtils::format("%d/%d", xp, nextLevelXP));
#endif
}

void StatusBar::setLevel(int32 level)
{
	m_levelLabel->setString(StringUtils::format(sLocaleMgr->getString("statusbars_level").c_str(), level));
	if (!sStoreReview->isProcessCompleted(REVIEW_PROCESS_HERO_LEVEL))
	{
		if (level >= REVIEW_UPGRADE_HERO_TO_LEVEL)
			sStoreReview->setProcessCompleted(REVIEW_PROCESS_HERO_LEVEL);
	}
}

void StatusBar::startIncomeAnimation()
{
	if (Action* action = m_moneybagSp->getActionByTag(ACTION_TAG_MONEYBAG))
		m_moneybagSp->stopAction(action);

	if(sSoundMgr->isPlaying(SOUND_GIVE_MONEY))
		sSoundMgr->stop(SOUND_GIVE_MONEY);

	sSoundMgr->play(SOUND_GIVE_MONEY);

	Sequence* moneybagSeq = Sequence::create(ScaleTo::create(0.2f, 1.4f), ScaleTo::create(0.2f, 1.0f), nullptr);
	moneybagSeq->setTag(ACTION_TAG_MONEYBAG);
	m_moneybagSp->runAction(moneybagSeq);
}

NS_END


