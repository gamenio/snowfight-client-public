#include "PlayerCountBar.h"

#include "scene/Utils.h"
#include "game/entities/DataPlayer.h"
#include "game/World.h"

NS_BEGIN

#define FRAMENAME_ICON_ALIVE					"battle_icon_alive.png"
#define FRAMENAME_ICON_KILLED					"battle_icon_killed.png"

#define COUNT_LABEL_MAX_WIDTH					16

PlayerCountBar* PlayerCountBar::create()
{
	PlayerCountBar* pRet = new PlayerCountBar();
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

PlayerCountBar::PlayerCountBar() :
	m_aliveIconSp(nullptr),
	m_aliveCountLabel(nullptr),
	m_killedIconSp(nullptr),
	m_killCountLabel(nullptr)
{
}

PlayerCountBar::~PlayerCountBar()
{
	m_aliveIconSp = nullptr;
	m_aliveCountLabel = nullptr;
	m_killedIconSp = nullptr;
	m_killCountLabel = nullptr;
}

bool PlayerCountBar::init()
{
	if (!Node::init())
		return false;

	this->setIgnoreAnchorPointForPosition(false);
	this->setCascadeOpacityEnabled(true);

	m_aliveIconSp = Sprite::createWithSpriteFrameName(FRAMENAME_ICON_ALIVE);
	m_aliveIconSp->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	this->addChild(m_aliveIconSp);

	m_aliveCountLabel = Label::createWithSystemFont("0", DEFAULT_SYSTEM_FONT, 12);
	m_aliveCountLabel->setPosition(m_aliveIconSp->getBoundingBox().getMaxX() + 1, 0);
	m_aliveCountLabel->setTextColor(Color4B(63, 183, 0, 255));
	m_aliveCountLabel->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	Utils::enableBoldForLabel(m_aliveCountLabel);
	this->addChild(m_aliveCountLabel);

	m_killedIconSp = Sprite::createWithSpriteFrameName(FRAMENAME_ICON_KILLED);
	m_killedIconSp->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_killedIconSp->setPosition(m_aliveIconSp->getBoundingBox().getMaxX() + 24, 0);
	this->addChild(m_killedIconSp);

	m_killCountLabel = Label::createWithSystemFont("0", DEFAULT_SYSTEM_FONT, 12);
	m_killCountLabel->setPosition(m_killedIconSp->getBoundingBox().getMaxX() + 1, 0);
	m_killCountLabel->setTextColor(Color4B(224, 41, 27, 255));
	m_killCountLabel->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	Utils::enableBoldForLabel(m_killCountLabel);
	this->addChild(m_killCountLabel);

	float minLabelWidth = m_killCountLabel->getContentSize().width * 2;
	this->setContentSize(Size(m_killCountLabel->getBoundingBox().getMinX() + minLabelWidth, m_killedIconSp->getBoundingBox().getMaxY()));

	return true;
}

void PlayerCountBar::update(float delta)
{
	DataPlayer* myChar = World::getInstance()->getMyself();
	if (!myChar)
		return;

	if (myChar->hasUpdatedField(CPLAYER_FIELD_KILL_COUNT))
		this->setKillCount(myChar->getKillCount());
}

void PlayerCountBar::setAliveCount(int32 count)
{
	m_aliveCountLabel->setString(std::to_string(count));
}

void PlayerCountBar::setKillCount(int32 count)
{
	m_killCountLabel->setString(std::to_string(count));
}

NS_END