#include "SignalIndicator.h"

#include "common/utils/TimeUtil.h"
#include "scene/Utils.h"

NS_BEGIN

#define FRAMENAME_SIGNAL_INDICATOR_BG		"signal_indicator_bg.png"
#define FRAMENAME_SIGNAL_LEVEL_LOST_CONN	"signal_level_lost_conn.png"

#define FLICKER_INTERVAL					0.25f

SignalIndicator::SignalIndicator() :
	m_bgSp(nullptr),
	m_levelSp(nullptr)
{
}

SignalIndicator::~SignalIndicator()
{
	m_bgSp = nullptr;
	m_levelSp = nullptr;
}

SignalIndicator* SignalIndicator::create()
{
	SignalIndicator* pRet = new SignalIndicator();
	if (pRet && pRet->init())
	{
		pRet->autorelease();
		return pRet;
	}
	else
	{
		CC_SAFE_DELETE(pRet);
		return nullptr;
	}
}

bool SignalIndicator::init()
{
	if (!Node::init())
		return false;

	m_bgSp = Sprite::createWithSpriteFrameName(FRAMENAME_SIGNAL_INDICATOR_BG);
	m_bgSp->setPosition(Point::ZERO);
	m_bgSp->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_bgSp->setVisible(false);
	this->addChild(m_bgSp);

	m_levelSp = Sprite::createWithSpriteFrameName(FRAMENAME_SIGNAL_LEVEL_LOST_CONN);
	m_levelSp->setPosition(Point::ZERO);
	m_levelSp->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_levelSp->setVisible(false);
	this->addChild(m_levelSp);

	this->setContentSize(m_bgSp->getContentSize());

	return true;
}

void SignalIndicator::onEnter()
{
	Node::onEnter();
}

void SignalIndicator::update(float delta)
{
	m_flickerTimer.update(delta);
	if (m_flickerTimer.passed())
	{
		m_levelSp->setVisible(!m_levelSp->isVisible());
		m_flickerTimer.reset();
	}
}

void SignalIndicator::show(SignalLevel level)
{
	m_flickerTimer.setInterval(FLICKER_INTERVAL);
	m_bgSp->setVisible(true);
	m_levelSp->setVisible(true);

	this->unscheduleUpdate();
	this->scheduleUpdate();
}

void SignalIndicator::hide()
{
	m_bgSp->setVisible(false);
	m_levelSp->setVisible(false);

	this->unscheduleUpdate();
}

NS_END
