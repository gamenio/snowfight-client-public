#include "Nameplate.h"

#include "game/LocaleMgr.h"
#include "game/entities/updates/ObjectUpdateFields.h"
#include "game/entities/DataPlayer.h"
#include "game/entities/DataRobot.h"
#include "game/GameConfig.h"
#include "game/World.h"
#include "scene/Utils.h"


NS_BEGIN

#define FULL_PERCENTAGE							100
#define NAME_UNKNOW								"Unknow"

Nameplate::Nameplate() :
	m_data(nullptr),
	m_isLayoutDirty(false),
	m_nameLabel(nullptr),
	m_healthBar(nullptr),
	m_staminaBar(nullptr),
	m_levelPlate(nullptr),
	m_aiActionLabel(nullptr),
	m_debugDraw(nullptr)
{
}

Nameplate::~Nameplate()
{
	CC_SAFE_RELEASE_NULL(m_data);

	m_nameLabel = nullptr;
	m_healthBar = nullptr;
	m_staminaBar = nullptr;
	m_levelPlate = nullptr;
	m_aiActionLabel = nullptr;
    
	m_debugDraw = nullptr;
}

Nameplate *Nameplate::create(DataUnit* data)
{
	Nameplate *node = new (std::nothrow) Nameplate();
	if (node && node->init(data))
	{
		node->autorelease();
		return node;
	}
	CC_SAFE_DELETE(node);
	return nullptr;
}

bool Nameplate::init(DataUnit* data)
{
	if (!Node::init())
		return false;

	m_data = data;
	CC_SAFE_RETAIN(data);

	this->initElements();
	this->layoutElements();

	return true;
}

void Nameplate::initElements()
{
	DataPlayer* myChar = World::getInstance()->getMyself();

	// 昵称
	m_nameLabel = SILLabel::createWithSystemFont("", DEFAULT_SYSTEM_FONT, 14);
	m_nameLabel->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
	m_nameLabel->setString(NAME_UNKNOW);
	if(m_data == myChar)
		m_nameLabel->setTextColor(Color4B(63, 183, 0, 255));
	else
	{
		m_nameLabel->setTextColor(Color4B(224, 41, 27, 255));
	}
	Utils::enableBoldForLabel(m_nameLabel);
	this->addChild(m_nameLabel);

	// 生命条
	m_healthBar = HealthBar::create();
	m_healthBar->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
	this->addChild(m_healthBar);

	if (m_data == myChar)
	{
		// 体力条
		m_staminaBar = StaminaBar::create();
		m_staminaBar->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
		this->addChild(m_staminaBar);
	}
	else
	{
		// 等级
		if (m_data->getLevel() > 0)
		{
			m_levelPlate = LevelPlate::create();
			m_levelPlate->setAnchorPoint(Point::ANCHOR_TOP_RIGHT);
			this->addChild(m_levelPlate);
		}
	}

#if NS_DEBUG
	if (m_data->isType(DATA_TYPEMASK_ROBOT))
	{
		World* world = World::getInstance();
		WorldSession* session = world->getSession() ? world->getSession()->asWorldSession() : nullptr;
		if (session && session->hasGMPermission())
		{
			m_aiActionLabel = SILLabel::createWithSystemFont("", DEFAULT_SYSTEM_FONT, 9);
			m_aiActionLabel->setAnchorPoint(Point::ANCHOR_MIDDLE_TOP);
			m_aiActionLabel->setTextColor(Color4B::GREEN);
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID) || (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
			m_aiActionLabel->enableOutline(Color4B(0, 0, 0, GLubyte(255 * 0.28f)), 1);
#endif
			m_aiActionLabel->enableShadow(Color4B(0, 0, 0, GLubyte(255 * 0.5f)), Size(0.5f, -0.5f), 0);
			this->addChild(m_aiActionLabel);
		}
	}
#endif // NS_DEBUG
}


void Nameplate::layoutElements()
{
	float nextY = 0;
	float maxWidth = 0;

	if(m_staminaBar)
		maxWidth = MAX(maxWidth, m_staminaBar->getBoundingBox().size.width);
	maxWidth = MAX(maxWidth, m_healthBar->getBoundingBox().size.width);
	maxWidth = MAX(maxWidth, m_nameLabel->getBoundingBox().size.width);

	// AI动作
#if NS_DEBUG
	if (m_aiActionLabel)
		m_aiActionLabel->setPosition(maxWidth / 2, nextY);
#endif

	// 体力条
	if (m_staminaBar)
	{
		m_staminaBar->setPosition(maxWidth / 2, nextY);
		nextY = m_staminaBar->getBoundingBox().getMaxY();
	}

	// 生命条
	m_healthBar->setPosition(maxWidth / 2, nextY);
	nextY = m_healthBar->getBoundingBox().getMaxY() + 2;

	// 等级
	if (m_levelPlate)
	{
		m_levelPlate->setPosition(m_healthBar->getBoundingBox().getMinX() + 2, m_healthBar->getBoundingBox().getMaxY() + 5);
		nextY = m_levelPlate->getBoundingBox().getMaxY();
	}

	// 昵称
	m_nameLabel->setPosition(maxWidth / 2, nextY);
	nextY = m_nameLabel->getBoundingBox().getMaxY();

	this->setContentSize(Size(maxWidth, nextY));
}

void Nameplate::setGlobalZOrder(float globalZOrder)
{
	m_nameLabel->setGlobalZOrder(globalZOrder);
	m_healthBar->setGlobalZOrder(globalZOrder);

	if (m_staminaBar)
		m_staminaBar->setGlobalZOrder(globalZOrder);

	if(m_levelPlate)
		m_levelPlate->setGlobalZOrder(globalZOrder);

	Node::setGlobalZOrder(globalZOrder);
}

void Nameplate::setStaminaCyclicRollingEnabled(bool isEnabled)
{
	m_staminaBar->setCyclicRollingEnabled(isEnabled);
}

void Nameplate::updateElements(bool force)
{
	this->updateHealth(force);
	this->updateName(force);
	this->updateStamina(force);
	this->updateLevel(force);
#if NS_DEBUG
	this->updateAIAction(force);
#endif

	if (m_isLayoutDirty)
	{
		this->layoutElements();
		m_isLayoutDirty = false;
	}
}

void Nameplate::updateName(bool force)
{
	if (!force && !m_data->hasUpdatedField(CUNIT_FIELD_NAME)
	    && !m_data->hasUpdatedField(CPLAYER_FIELD_IS_GM))
	{
		return;
	}

	std::string name = m_data->getName();
	if (name.empty())
		name = NAME_UNKNOW;

#if NS_DEBUG
	World* world = World::getInstance();
	WorldSession* session = world->getSession() ? world->getSession()->asWorldSession() : nullptr;

	if (session && session->hasGMPermission())
	{
		std::stringstream ss;
		ss << name;
		ss << " ("; 
		DataPlayer* player = m_data->asDataPlayer();
		if (player && player->isGM())
			ss << "GM";
		else
			ss << m_data->getGuid().getTypeName();
		ss  << "ID:" << m_data->getGuid().getCounter() << ")";
		m_nameLabel->setString(ss.str());
	}
	else
		m_nameLabel->setString(name);
#else
	m_nameLabel->setString(name);
#endif // NS_DEBUG

	m_isLayoutDirty = true;
}

void Nameplate::updateStamina(bool force)
{
	if (!m_staminaBar)
		return;

	if (!force && !m_data->hasUpdatedField(CUNIT_FIELD_STAMINA))
		return;

	m_staminaBar->setStaminaProgress(m_data->getStamina(), m_data->getMaxStamina());
}

void Nameplate::updateLevel(bool force)
{
	if (!m_levelPlate)
		return;

	if (!force && !m_data->hasUpdatedField(CUNIT_FIELD_LEVEL))
		return;

	m_levelPlate->setLevel(m_data->getLevel());
}

void Nameplate::updateAIAction(bool force)
{
	if (!m_aiActionLabel)
		return;

	if (!force && !m_data->hasUpdatedField(CROBOT_FIELD_AIACTION_TYPE) && !m_data->hasUpdatedField(CROBOT_FIELD_AIACTION_STATE))
		return;

	DataRobot* robot = m_data->asDataRobot();
	std::stringstream ss;
	ss << "[" << robot->getAIActionTypeName();
	ss << "|";
	ss << robot->getAIActionStateName() << "]";
	m_aiActionLabel->setString(ss.str());
}

void Nameplate::updateHealth(bool force)
{
	if (!force && !m_data->hasUpdatedField(CUNIT_FIELD_MAX_HEALTH) && !m_data->hasUpdatedField(CUNIT_FIELD_HEALTH))
		return;

	m_healthBar->setHealthProgress(m_data->getHealth(), m_data->getMaxHealth());
}

NS_END
