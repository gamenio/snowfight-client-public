#include "HealthBar.h"

NS_BEGIN


#define FRAMENAME_HEALTH_BG						"health_bg.png"
#define FRAMENAME_HEALTH_PROG					"health_prog.png"

#define FULL_PERCENTAGE							100

HealthBar::HealthBar() :
	m_bgSp(nullptr),
	m_valueLabel(nullptr),
	m_healthProg(nullptr)
{
}

HealthBar::~HealthBar()
{
	m_bgSp = nullptr;
	m_valueLabel = nullptr;
	m_healthProg = nullptr;
}

HealthBar* HealthBar::create()
{
	HealthBar* pRet = new HealthBar();
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

bool HealthBar::init()
{
	if (!Node::init())
		return false;

	this->setAnchorPoint(Point::ANCHOR_MIDDLE);
	this->setIgnoreAnchorPointForPosition(false);

	m_bgSp = SILSprite::createWithSpriteFrameName(FRAMENAME_HEALTH_BG);
	m_bgSp->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	this->addChild(m_bgSp);

	m_healthProg = SILProgressTimer::create(Sprite::createWithSpriteFrameName(FRAMENAME_HEALTH_PROG));
	m_healthProg->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_healthProg->setType(ProgressTimer::Type::BAR);
	m_healthProg->setBarChangeRate(Vec2(1, 0));
	m_healthProg->setMidpoint(Vec2(0, 0));
	this->addChild(m_healthProg);

#if NS_DEBUG
	m_valueLabel = SILLabel::createWithSystemFont("0/0", DEFAULT_SYSTEM_FONT, 5);
	m_valueLabel->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
	m_valueLabel->setTextColor(Color4B::WHITE);
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID) || (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
	m_valueLabel->enableOutline(Color4B(0, 0, 0, GLubyte(255 * 0.28f)), 1);
#endif
	m_valueLabel->enableShadow(Color4B(0, 0, 0, GLubyte(255 * 0.5f)), Size(0.5f, -0.5f), 0);
	m_valueLabel->setPosition(Vec2(m_healthProg->getContentSize().width / 2, m_healthProg->getContentSize().height / 2));
	this->addChild(m_valueLabel);
#endif // NS_DEBUG

	this->setContentSize(m_bgSp->getContentSize());

	return true;
}


void HealthBar::setGlobalZOrder(float globalZOrder)
{
	m_bgSp->setGlobalZOrder(globalZOrder);
	m_healthProg->setGlobalZOrder(globalZOrder);
	if(m_valueLabel)
		m_valueLabel->setGlobalZOrder(globalZOrder);

	Node::setGlobalZOrder(globalZOrder);
}

void HealthBar::setHealthProgress(int32 value, int32 max)
{
	float percentage = (float)value / max * FULL_PERCENTAGE;
	m_healthProg->setPercentage(percentage);
	if (m_valueLabel)
		m_valueLabel->setString(StringUtils::format("%d/%d", value, max));
}

NS_END

