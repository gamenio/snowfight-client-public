#include "CyclicRolling.h"


NS_BEGIN


CyclicRolling* CyclicRolling::create(std::string const& spriteFrameName, std::string const& stencilFrameName, float rollingSpeed)
{
	CyclicRolling* ret = new (std::nothrow) CyclicRolling();
	if (ret && ret->init(spriteFrameName, stencilFrameName, rollingSpeed))
	{
		ret->autorelease();
		return ret;
	}
	else
	{
		CC_SAFE_DELETE(ret);
		return nullptr;
	}
}

bool CyclicRolling::init(std::string const& spriteFrameName, std::string const& stencilFrameName, float rollingSpeed)
{
	if (!Node::init())
		return false;

	m_rollingSpeed = rollingSpeed;

	this->setAnchorPoint(Point::ANCHOR_MIDDLE);
	this->setIgnoreAnchorPointForPosition(false);

	m_stencil = Sprite::createWithSpriteFrameName(stencilFrameName);
	m_stencil->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_stencilOriginalSize = m_stencil->getContentSize();

	m_clippingNode = SILClippingNode::create(m_stencil);
	this->addChild(m_clippingNode);

	m_part1 = SILSprite::createWithSpriteFrameName(spriteFrameName);
	m_part1->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_part1->setPosition(0.0f, 0.0f);
	m_clippingNode->addChild(m_part1);

	m_part2 = SILSprite::createWithSpriteFrameName(spriteFrameName);
	m_part2->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_part2->setPosition(m_part1->getBoundingBox().getMaxX(), 0.0f);
	m_clippingNode->addChild(m_part2);

	this->setContentSize(m_part1->getContentSize());

	return true;
}

void CyclicRolling::update(float dt)
{
	m_part1->setPosition(m_part1->getPosition().x + m_rollingSpeed, m_part1->getPosition().y);
	m_part2->setPosition(m_part1->getBoundingBox().getMaxX(), m_part2->getPosition().y);

	if (std::abs(m_part1->getPosition().x) >= m_part1->getBoundingBox().size.width)
	{
		m_part1->setPosition(m_part2->getBoundingBox().getMaxX(), m_part1->getPosition().y);
		std::swap(m_part1, m_part2);
	}
}

void CyclicRolling::onEnter()
{
	Node::onEnter();

	if (this->isVisible())
	{
		this->scheduleUpdate();
	}

	else
	{
		this->unscheduleUpdate();
	}
}

void CyclicRolling::setGlobalZOrder(float globalZOrder)
{
	Node::setGlobalZOrder(globalZOrder);

	m_clippingNode->setGlobalZOrder(globalZOrder);
	m_stencil->setGlobalZOrder(globalZOrder);
	m_part1->setGlobalZOrder(globalZOrder);
	m_part2->setGlobalZOrder(globalZOrder);
}

void CyclicRolling::setVisible(bool visible)
{
	if (this->isVisible() != visible)
	{
		if (visible)
		{
			this->scheduleUpdate();
		}

		else
		{
			this->unscheduleUpdate();
		}

		Node::setVisible(visible);
	}
}

void CyclicRolling::setPercentage(float percentage)
{
	if (m_percentage != percentage)
	{
		m_stencil->setContentSize(Size(m_stencilOriginalSize.width * (percentage / 100.f), m_stencilOriginalSize.height));
		m_percentage = percentage;
	}
}

CyclicRolling::CyclicRolling() :
	m_clippingNode(nullptr),
	m_stencil(nullptr),
	m_part1(nullptr),
	m_part2(nullptr),
	m_rollingSpeed(0),
	m_percentage(100)
{
}

CyclicRolling::~CyclicRolling()
{
	this->unscheduleUpdate();
	m_clippingNode = nullptr;
	m_stencil = nullptr;
	m_part1 = nullptr;
	m_part2 = nullptr;
}


NS_END