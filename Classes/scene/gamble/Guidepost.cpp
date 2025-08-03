#include "Guidepost.h"

#include "common/debugging/DebugDrawer.h"
#include "game/entities/DataPlayer.h"
#include "game/World.h"
#include "game/GameConfig.h"


NS_BEGIN

#if NS_DEBUG

#endif // NS_DEBUG

#define GUIDEPOST_HEAD_FRAME_FORMAT				"guidepost_head%d.png"

Guidepost* Guidepost::createWithData(DataLocatorObject* data)
{
	Guidepost *pRet = new Guidepost();
	if (pRet && pRet->initWithData(data))
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

Guidepost::Guidepost() :
	m_isActive(false),
	m_data(nullptr),
	m_brightState(BrightState::NORMAL)
{

}

Guidepost::~Guidepost()
{
	CC_SAFE_RELEASE_NULL(m_data);
}

bool Guidepost::initWithData(DataLocatorObject* data)
{
	if (!Node::init())
		return false;

	CC_SAFE_RETAIN(data);
	m_data = data;

	this->setAnchorPoint(Point::ANCHOR_MIDDLE);

	m_mainSp = Sprite::create();
	m_mainSp->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	this->addChild(m_mainSp);

	this->initLocatorObject();
	this->setContentSize(m_mainSp->getContentSize());

	return true;
}

void Guidepost::update(float delta)
{
	Node::update(delta);

	if (!this->isActive())
		return;

	DataPlayer* myChar = World::getInstance()->getMyself();
	if (!myChar)
		return;

	DataLocatorObject* data = static_cast<DataLocatorObject*>(this->getData());

	Rect rect;
	rect.origin.x = myChar->getPosition().x - (myChar->getViewport().width - this->getContentSize().width) / 2;
	rect.origin.y = myChar->getPosition().y - (myChar->getViewport().height - this->getContentSize().height) / 2;
	rect.size.width = myChar->getViewport().width - this->getContentSize().width;
	rect.size.height = myChar->getViewport().height - this->getContentSize().height;

	Point intersection = this->intersectionOnRect(data->getPosition(), rect);
	this->setPosition(intersection);

	//auto debugDraw = sDebugDrawer->getDrawByTag("Guidepost");
	//debugDraw->setClearing(true);
	//debugDraw->drawLine(myChar->getPosition(), data->getPosition(), Color4F::GREEN);
	//debugDraw->drawRect(Vec2(this->getBoundingBox().getMinX(), this->getBoundingBox().getMinY()), Vec2(this->getBoundingBox().getMaxX(), this->getBoundingBox().getMaxY()), Color4F::MAGENTA);

	this->updateLocatorObject(false);
}

void Guidepost::onActivated()
{
	if (m_isActive)
		return;

	this->updateLocatorObject(true);
	this->setVisible(true);

	m_isActive = true;
}

void Guidepost::onInactivated()
{
	if (!m_isActive)
		return;

	this->cleanUpdateMask();
	this->setVisible(false);

	m_isActive = false;
}

void Guidepost::cleanUpdateMask()
{
	if (m_data)
		m_data->clearUpdateFlags();
}

// http://stackoverflow.com/a/31254199/253468
Point Guidepost::intersectionOnRect(Point const& p, Rect const& rect)
{
	Point intersection;
	float x = p.x;
	float y = p.y;
	float minX = rect.getMinX();
	float minY = rect.getMinY();
	float maxX = rect.getMaxX();
	float maxY = rect.getMaxY();
	//assert minX <= maxX;
	//assert minY <= maxY;

	float midX = (minX + maxX) / 2;
	float midY = (minY + maxY) / 2;
	float dx = midX - x;
	float dy = midY - y;
	float m = dx == 0 ? 0 : dy / dx;

	if (x <= midX) // check "left" side
	{
		float minXy = m * (minX - x) + y;
		if (minY <= minXy && minXy <= maxY)
		{
			intersection.x = minX;
			intersection.y = minXy;
		}
	}

	if (x >= midX) // check "right" side
	{
		float maxXy = m * (maxX - x) + y;
		if (minY <= maxXy && maxXy <= maxY)
		{
			intersection.x = maxX;
			intersection.y = maxXy;
		}

	}

	if (y <= midY) // check "top" side
	{
		float minYx = m == 0 ? x : (minY - y) / m + x;
		if (minX <= minYx && minYx <= maxX)
		{
			intersection.x = minYx;
			intersection.y = minY;
		}

	}

	if (y >= midY) // check "bottom" side
	{
		float maxYx = m == 0 ? x : (maxY - y) / m + x;
		if (minX <= maxYx && maxYx <= maxX)
		{
			intersection.x = maxYx;
			intersection.y = maxY;
		}
	}

	if (x == midX && y == midY)
	{
		intersection.x = x;
		intersection.y = y;
	}

	return intersection;
}

void Guidepost::setBrightState(Guidepost::BrightState state)
{
	if (m_brightState != state) {
		GLProgramState *glState = nullptr;
		switch (state)
		{
		case BrightState::NORMAL:
			glState = GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR_NO_MVP, m_mainSp->getTexture());
			break;
		case BrightState::GRAY:
			glState = GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_GRAYSCALE, m_mainSp->getTexture());
		default:
			break;
		}

		m_mainSp->setGLProgramState(glState);
		m_brightState = state;
	}
}

void Guidepost::initLocatorObject()
{
	switch (m_data->getTypeID())
	{
	case DATA_TYPEID_UNIT_LOCATOR:
		this->initWithUnitLocator(m_data->asUnitLocator());
		break;
	default:
		// Unknown locator data type
		break;
	}
}

void Guidepost::updateLocatorObject(bool force)
{
	switch (m_data->getTypeID())
	{
	case DATA_TYPEID_UNIT_LOCATOR:
		this->updateWithUnitLocator(m_data->asUnitLocator(), force);
		break;
	default:
		// Unknown locator data type
		break;
	}
}


void Guidepost::initWithUnitLocator(DataUnitLocator* data)
{
	std::string frameName = StringUtils::format(GUIDEPOST_HEAD_FRAME_FORMAT, data->getDisplayId());
	m_mainSp->setSpriteFrame(frameName);
}

void Guidepost::updateWithUnitLocator(DataUnitLocator* data, bool force)
{
	if (data->hasUpdatedField(CUNIT_LOCATOR_FIELD_IS_ALIVE) || force)
	{
		if (data->isAlive())
			this->setBrightState(BrightState::NORMAL);
		else
			this->setBrightState(BrightState::GRAY);
	}
}


NS_END

