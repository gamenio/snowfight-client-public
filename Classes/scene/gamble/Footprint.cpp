#include "Footprint.h"

#include "common/utils/MathTools.h"
#include "scene/ObjectPools.h"

NS_BEGIN

Footprint::Footprint() :
	m_recycled(true)
{
}

Footprint::~Footprint()
{
}

Footprint* Footprint::create()
{
	Footprint* sprite = new (std::nothrow) Footprint();
	if (sprite && sprite->init())
	{
		sprite->autorelease();
		return sprite;
	}
	CC_SAFE_DELETE(sprite);
	return nullptr;
}

bool Footprint::init()
{
	if (!Sprite::initWithSpriteFrameName("footprint.png"))
		return false;

	return true;
}

void Footprint::reset()
{
	this->setOpacity(255);
	this->setGlobalZOrder(0);
	this->setPosition(Point::ZERO);

	this->removeFromParentAndCleanup(true);

}

void Footprint::recycle()
{
	FootprintPool::getInstance()->put(this);
	m_recycled = true;
}

void Footprint::run(Point const& pos, float orient, float zOrder)
{
	NS_ASSERT(m_recycled);

	m_recycled = false;


	this->setPosition(pos);
	this->setGlobalZOrder(zOrder);

	float angle = MathTools::radians2Degrees(orient);
	if (angle < 0)
		angle += 360;

	float r = -angle;
	if (angle > 90 && angle <= 270)
	{
		r = 180 - angle;
	}
	//CCLOG("angle:%f  r:%f", angle, r);
	this->setRotation(r);

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
	this->setSkewX(skewX);
	//CCLOG("skew:%f a:%f r: %f", skewX, a, r);


	FadeOut* fade = FadeOut::create(0.3f);
	DelayTime* delay = DelayTime::create(1.5f);
	CallFunc* callFunc = CallFunc::create([&]() {
		this->recycle();
	});
	Sequence* seq = Sequence::create(delay, fade, callFunc, nullptr);
	this->runAction(seq);
}


NS_END