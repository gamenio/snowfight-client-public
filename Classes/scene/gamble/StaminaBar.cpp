#include "StaminaBar.h"

NS_BEGIN


#define FRAMENAME_HEALTH_BG						"health_bg.png"
#define FRAMENAME_HEALTH_PROG					"health_prog.png"

#define FRAMENAME_STAMINA_BG					"stamina_bg.png"
#define FRAMENAME_STAMINA_PROG					"stamina_prog.png"
#define FRAMENAME_STAMINA_CYCLIC_ROLLING		"stamina_cyclic_rolling.png"

#define FULL_PERCENTAGE							100

StaminaBar::StaminaBar() :
	m_bgSp(nullptr),
	m_staminaProg(nullptr),
	m_staminaCr(nullptr)
{
}

StaminaBar::~StaminaBar()
{
	m_bgSp = nullptr;
	m_staminaProg = nullptr;
	m_staminaCr = nullptr;
}

StaminaBar* StaminaBar::create()
{
	StaminaBar* pRet = new StaminaBar();
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

bool StaminaBar::init()
{
	if (!Node::init())
		return false;

	this->setAnchorPoint(Point::ANCHOR_MIDDLE);
	this->setIgnoreAnchorPointForPosition(false);

	m_bgSp = SILSprite::createWithSpriteFrameName(FRAMENAME_STAMINA_BG);
	m_bgSp->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	this->addChild(m_bgSp);

	Sprite* staminaFg = Sprite::createWithSpriteFrameName(FRAMENAME_STAMINA_PROG);
	m_staminaProg = SILProgressTimer::create(staminaFg);
	m_staminaProg->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_staminaProg->setType(ProgressTimer::Type::BAR);
	m_staminaProg->setBarChangeRate(Vec2(1, 0));
	m_staminaProg->setMidpoint(Vec2(0, 0));
	this->addChild(m_staminaProg);

	m_staminaCr = CyclicRolling::create(FRAMENAME_STAMINA_CYCLIC_ROLLING, FRAMENAME_STAMINA_CYCLIC_ROLLING, -0.5f);
	m_staminaCr->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	this->addChild(m_staminaCr);
	m_staminaCr->setVisible(false);

	this->setContentSize(m_bgSp->getContentSize());

	return true;
}

void StaminaBar::setGlobalZOrder(float globalZOrder)
{
	m_bgSp->setGlobalZOrder(globalZOrder);
	m_staminaProg->setGlobalZOrder(globalZOrder);
	m_staminaCr->setGlobalZOrder(globalZOrder);

	Node::setGlobalZOrder(globalZOrder);
}

void StaminaBar::setStaminaProgress(int32 value, int32 max)
{
	float percentage = (float)value / max * FULL_PERCENTAGE;
	if (m_staminaProg->isVisible())
		m_staminaProg->setPercentage(percentage);
	else
		m_staminaCr->setPercentage(percentage);
}

void StaminaBar::setCyclicRollingEnabled(bool isEnabled)
{
	if (isEnabled)
	{
		if (m_staminaProg->isVisible())
			m_staminaCr->setPercentage(m_staminaProg->getPercentage());
	}
	else
	{
		if (m_staminaCr->isVisible())
			m_staminaProg->setPercentage(m_staminaCr->getPercentage());
	}

	m_staminaCr->setVisible(isEnabled);
	m_staminaProg->setVisible(!isEnabled);
}


NS_END

