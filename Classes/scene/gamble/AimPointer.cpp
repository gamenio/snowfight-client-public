#include "AimPointer.h"

#include "common/utils/MathTools.h"


NS_BEGIN

#define ROTATION_SPEED				10.0f			// Rotation speed. Unit: degrees/second
#define ROTATION_MAX_OFFSET			1.0f			// Maximum offset during rotation. Unit: degrees

#define FRAMENAME_AIM_POINTER		"aim_pointer.png"

AimPointer* AimPointer::create()
{
	AimPointer* pRet = new AimPointer();
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

bool AimPointer::init()
{
	if (!Node::init())
		return false;

	this->setIgnoreAnchorPointForPosition(false);
	this->setAnchorPoint(Point::ANCHOR_MIDDLE);

	m_pointerSp = SILScale9Sprite::createWithSpriteFrameName(FRAMENAME_AIM_POINTER);
	m_pointerSp->setAnchorPoint(Point::ANCHOR_MIDDLE_RIGHT);
	this->addChild(m_pointerSp);

	return true;
}

AimPointer::AimPointer() :
	m_length(0),
	m_width(0),
	m_direction(FLT_MAX),
	m_dstAngle(0),
	m_currAngle(0),
	m_isFirstTick(true),
	m_pointerSp(nullptr)
{
}

AimPointer::~AimPointer()
{
	m_pointerSp = nullptr;
}

void AimPointer::update(float delta)
{
	if (m_currAngle != m_dstAngle)
	{
		if (m_isFirstTick)
		{
			m_currAngle = m_dstAngle;
			m_isFirstTick = false;
		}
		else
		{
			float offset = delta * ROTATION_SPEED;
			float diff = calcValidAngle(m_dstAngle - m_currAngle);
			if (diff > 0)
			{
				if (diff - offset > 0)
					m_currAngle += offset;
				else
					m_currAngle = m_dstAngle;
			}
			else
			{
				if (diff + offset < 0)
					m_currAngle -= offset;
				else
					m_currAngle = m_dstAngle;
			}
			m_currAngle = calcValidAngle(m_currAngle);
		}
		this->updatePointer(m_currAngle);
	}
}

void AimPointer::setGlobalZOrder(float globalZOrder)
{
	m_pointerSp->setGlobalZOrder(globalZOrder);

	Node::setGlobalZOrder(globalZOrder);
}


void AimPointer::setDirection(float rad)
{
	if (m_direction == rad)
		return;

	m_direction = rad;
	m_dstAngle = MathTools::radians2Degrees(rad);

	float diff = calcValidAngle(m_dstAngle - m_currAngle);
	if (std::abs(diff) > ROTATION_MAX_OFFSET)
	{
		if (diff < 0)
			diff += ROTATION_MAX_OFFSET;
		else
			diff -= ROTATION_MAX_OFFSET;
		m_currAngle = calcValidAngle(m_currAngle + diff);
	}
	assert(std::round(calcValidAngle(m_dstAngle - m_currAngle)) <= ROTATION_MAX_OFFSET);

	//CCLOG("AimPointer: DstAngle: %f CurrAngle: %f", m_dstAngle, m_currAngle);
}

void AimPointer::setLength(float length)
{
	if (m_length != length)
	{
		m_length = length;
		m_pointerSp->setContentSize(Size(m_length, m_pointerSp->getContentSize().height));
	}
}

void AimPointer::setWidth(float width)
{
	if (m_width != width)
	{
		m_width = width;
		m_pointerSp->setContentSize(Size(m_pointerSp->getContentSize().width, m_width));
	}
}

void AimPointer::reset()
{
	m_length = 0;
	m_width = 0;
	m_direction = FLT_MAX;
	m_dstAngle = 0;
	m_currAngle = 0;
	m_isFirstTick = true;
}

float AimPointer::calcValidAngle(float angle)
{
	if (angle > 180)
		angle -= 360;
	else if (angle < -180)
		angle += 360;

	return angle;
}

void AimPointer::updatePointer(float angle)
{
	if (angle < 0)
		angle += 360;

	float r = 180.f - angle;
	//CCLOG("angle:%f  r:%f", angle, r);
	m_pointerSp->setRotation(r);

	const float MAX_SKEW_ANGLE = 27;
	float maxSkewX = -36;
	float a = ((int32)(angle) % 90);
	if ((int32)(angle / 90) % 2 != 0)
	{
		maxSkewX = -maxSkewX;
		a = 90 - a;
	}

	float skewX = 0;
	if (a > MAX_SKEW_ANGLE)
		skewX = (1.0 - (a - MAX_SKEW_ANGLE) / (90 - MAX_SKEW_ANGLE)) * maxSkewX;
	else
		skewX = a / MAX_SKEW_ANGLE * maxSkewX;
	m_pointerSp->setSkewX(skewX);
	//CCLOG("skew:%f a:%f r: %f", skewX, a, r);
}

NS_END
