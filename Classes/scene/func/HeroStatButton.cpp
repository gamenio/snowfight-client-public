//
//  HeroStatButton.cpp
//  snowfight
//
//  Created by Luthier on 2019/11/19.
//

#include "HeroStatButton.h"
#include "game/LocaleMgr.h"
#include "scene/Utils.h"

using namespace cocos2d::ui;

NS_BEGIN

#define FRAMENAME_HEROSTAT_BTN_ICON						"func_herostat_btn_icon.png"
#define FRAMENAME_HEROSTAT_BTN_SHADOW					"func_herostat_btn_shadow.png"

#define INFO_BUBBLE_ARROW_POSITION				45

#define ACTION_TAG_JUMPING				1

#define SCHEDULE_KEY_TIPS_DELAYED			"TipsDelayed"

HeroStatButton* HeroStatButton::create()
{
    auto ret = new (std::nothrow) HeroStatButton();
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

bool HeroStatButton::init()
{
    if (!Node::init())
        return false;
    
	SpriteFrame* shadowFrame = SpriteFrameCache::getInstance()->getSpriteFrameByName(FRAMENAME_HEROSTAT_BTN_SHADOW);
	this->setContentSize(shadowFrame->getOriginalSize());

	m_shadowSp = Sprite::createWithSpriteFrame(shadowFrame);
	m_shadowSp->setPosition(Vec2(this->getContentSize().width / 2, this->getContentSize().height / 2));
	this->addChild(m_shadowSp);
    
    m_iconBtn = Button::create(FRAMENAME_HEROSTAT_BTN_ICON, "", "", Widget::TextureResType::PLIST);
	m_iconBtn->setPosition(m_shadowSp->getPosition());
    m_iconBtn->addClickEventListener(CC_CALLBACK_1(HeroStatButton::buttonClickCallback, this));
    this->addChild(m_iconBtn);

	m_infoBubble = InfoBubble::create(InfoBubble::ARROW_DOWN);
	m_infoBubble->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_infoBubble->setArrowPosition(INFO_BUBBLE_ARROW_POSITION);
	this->addChild(m_infoBubble);

    return true;
}

HeroStatButton::HeroStatButton() :
	m_shownTipsType(TipsType::TIPS_NONE),
    m_iconBtn(nullptr),
	m_shadowSp(nullptr),
	m_infoBubble(nullptr)
{
}

HeroStatButton::~HeroStatButton()
{
	this->unschedule(SCHEDULE_KEY_TIPS_DELAYED);
	this->stopJumpAnimation();

    m_iconBtn = nullptr;
	m_shadowSp = nullptr;
	m_infoBubble = nullptr;
}

void HeroStatButton::showTips(TipsType type, float delay)
{
	if (m_shownTipsType == type)
		return;

	this->hideTips();

	m_shownTipsType = type;
	this->scheduleOnce([this, type](float dt) {
		switch (type)
		{
		case HeroStatButton::TIPS_VIEW_STATS:
			this->startJumpAnimation(8.0f);
			break;
		case HeroStatButton::TIPS_UPGRADE_STATS:
			m_infoBubble->show(sLocaleMgr->getString("func_tips_upgrade_stats"));
			break;
		default:
			break;
		}

	}, delay, SCHEDULE_KEY_TIPS_DELAYED);

}

void HeroStatButton::hideTips()
{
	if (m_shownTipsType == TIPS_NONE)
		return;

	this->unschedule(SCHEDULE_KEY_TIPS_DELAYED);

	switch (m_shownTipsType)
	{
	case HeroStatButton::TIPS_VIEW_STATS:
		this->startFinishJumpAnimation();
		break;
	case HeroStatButton::TIPS_UPGRADE_STATS:
		m_infoBubble->hide();
		break;
	default:
		break;
	}

	m_shownTipsType = TIPS_NONE;
}

void HeroStatButton::startJumpAnimation(float interval)
{
	this->stopJumpAnimation();

	Vector<FiniteTimeAction*> iconActions, shadowActions;
	
	auto jumpSeq = Sequence::create(JumpBy::create(0.9f, Point::ZERO, 8, 3), DelayTime::create(interval), nullptr);
	iconActions.pushBack(Repeat::create(jumpSeq, pow(2, 30)));
	auto iconAction = Sequence::create(iconActions);
	iconAction->setTag(ACTION_TAG_JUMPING);
	m_iconBtn->runAction(iconAction);

	auto scale = ScaleBy::create(0.15f, 0.8f);
	auto scaleSeq = Sequence::create(scale, scale->reverse(), nullptr);
	auto repeatScaleSeq = Sequence::create(Repeat::create(scaleSeq, 3), DelayTime::create(interval), nullptr);
	shadowActions.pushBack(Repeat::create(repeatScaleSeq, pow(2, 30)));
	auto shadowAction = Sequence::create(shadowActions);
	shadowAction->setTag(ACTION_TAG_JUMPING);
	m_shadowSp->runAction(shadowAction);
}


void HeroStatButton::setClickEventListener(HeroStatButtonClickCallback const& callback)
{
    m_clickCallback = callback;
}

void HeroStatButton::setScale(float scale)
{
	Node::setScale(scale);
	// 保持气泡的大小不变
	m_infoBubble->setScale(1.0f / scale);
}

void HeroStatButton::onEnter()
{
	Node::onEnter();
	m_infoBubble->setPosition(this->getContentSize().width / 2 - m_infoBubble->getArrowPosition() * m_infoBubble->getScale(), this->getContentSize().height - 4);
}

void HeroStatButton::buttonClickCallback(Ref* sender)
{
	if (m_clickCallback)
		m_clickCallback(this);
}

void HeroStatButton::startFinishJumpAnimation()
{
	this->stopJumpAnimation();

	auto jump = JumpTo::create(0.15f, m_shadowSp->getPosition(), 0, 1);
	jump->setTag(ACTION_TAG_JUMPING);
	m_iconBtn->runAction(jump);

	auto scale = ScaleTo::create(0.15f, 1.0f);
	scale->setTag(ACTION_TAG_JUMPING);
	m_shadowSp->runAction(scale);
}

void HeroStatButton::stopJumpAnimation()
{
	if (Action* action = m_iconBtn->getActionByTag(ACTION_TAG_JUMPING))
		m_iconBtn->stopAction(action);

	if (Action* action = m_shadowSp->getActionByTag(ACTION_TAG_JUMPING))
		m_shadowSp->stopAction(action);

}

NS_END
