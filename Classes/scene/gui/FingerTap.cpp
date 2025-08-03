//
//  FingerTap.cpp
//  snowfight
//
//  Created by Luthier on 2021/10/19.
//

#include "FingerTap.h"

NS_BEGIN

#define	ACTION_TAG_SHOWING			1
#define ACTION_TAG_HIDING			2
#define ACTION_TAG_TAPPING			3

#define	ACTION_TAG_MOVING			1


#define SCHEDULE_KEY_SHOW_DELAYED	"ShowDelayed"

#define FINGER_MOVE_SPEED			412.0f		// 手指移动速度。单位：points/second

FingerTap* FingerTap::create()
{
    auto ret = new (std::nothrow) FingerTap();
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

bool FingerTap::init()
{
    if (!Node::init())
        return false;

	this->setIgnoreAnchorPointForPosition(false);

	m_mainSp = Sprite::createWithSpriteFrameName("finger.png");
	m_mainSp->setOpacity(0);
	m_mainSp->setVisible(false);
	this->addChild(m_mainSp);

	this->layoutElements();

    return true;
}

FingerTap::FingerTap() :
	m_isShown(false),
	m_mainSp(nullptr)
{
    
}

FingerTap::~FingerTap()
{
	m_mainSp = nullptr;
}


void FingerTap::onEnter()
{
	Node::onEnter();
}

void FingerTap::show(Point const& position, float delay)
{
	if (m_isShown)
		this->moveTo(position);
	else
	{
		if (delay > 0)
		{
			this->scheduleOnce([this, position](float dt) {
				this->setPosition(position);
				this->showInternal();
			}, delay, SCHEDULE_KEY_SHOW_DELAYED);
		}
		else
		{
			this->setPosition(position);
			this->showInternal();
		}
		m_isShown = true;
	}
}

void FingerTap::showInternal()
{
	if (Action* action = m_mainSp->getActionByTag(ACTION_TAG_HIDING))
		m_mainSp->stopAction(action);

	m_mainSp->setVisible(false);
	m_mainSp->setOpacity(0);

	this->stopTappingAnimation();

	Vector<FiniteTimeAction*> arrayOfActions;

	arrayOfActions.pushBack(Show::create());
	arrayOfActions.pushBack(FadeTo::create(0.1f, 255));

	auto* callFunc = CallFunc::create([this]() {
		this->startTappingAnimation();
	});
	arrayOfActions.pushBack(callFunc);

	auto* seq = Sequence::create(arrayOfActions);
	seq->setTag(ACTION_TAG_SHOWING);
	m_mainSp->runAction(seq);
}

void FingerTap::hide()
{
	if (!m_isShown)
		return;

	this->unschedule(SCHEDULE_KEY_SHOW_DELAYED);
	if (Action* action = m_mainSp->getActionByTag(ACTION_TAG_SHOWING))
		m_mainSp->stopAction(action);

	if (Action* action = m_mainSp->getActionByTag(ACTION_TAG_HIDING))
		m_mainSp->stopAction(action);

	this->stopMovingAnimation();
	this->stopTappingAnimation();

	Vector<FiniteTimeAction*> arrayOfActions;

	arrayOfActions.pushBack(FadeTo::create(0.1f, 0));
	arrayOfActions.pushBack(Hide::create());

	auto* seq = Sequence::create(arrayOfActions);
	seq->setTag(ACTION_TAG_HIDING);
	m_mainSp->runAction(seq);

	m_isShown = false;
}

void FingerTap::layoutElements()
{
	this->setContentSize(m_mainSp->getContentSize());
	m_mainSp->setPosition(this->getContentSize().width / 2, this->getContentSize().height / 2);
}

void FingerTap::moveTo(Point const& position)
{
	this->unschedule(SCHEDULE_KEY_SHOW_DELAYED);
	if (Action* action = m_mainSp->getActionByTag(ACTION_TAG_SHOWING))
		m_mainSp->stopAction(action);

	this->stopMovingAnimation();
	this->stopTappingAnimation();

	if (Action* action = m_mainSp->getActionByTag(ACTION_TAG_HIDING))
		m_mainSp->stopAction(action);

	m_mainSp->setVisible(true);
	m_mainSp->setOpacity(255);

	Vector<FiniteTimeAction*> arrayOfActions;

	float dist = this->getPosition().getDistance(position);
	float dur = dist / FINGER_MOVE_SPEED;
	auto* moveTo = EaseSineOut::create(MoveTo::create(dur, position));
	arrayOfActions.pushBack(moveTo);
	arrayOfActions.pushBack(DelayTime::create(0.1f));
	auto* callFunc = CallFunc::create([this]() {
		this->startTappingAnimation();
	});
	arrayOfActions.pushBack(callFunc);

	auto* seq = Sequence::create(arrayOfActions);
	seq->setTag(ACTION_TAG_MOVING);
	this->runAction(seq);
}

void FingerTap::stopMovingAnimation()
{
	if (Action* action = this->getActionByTag(ACTION_TAG_MOVING))
		this->stopAction(action);
}

void FingerTap::stopTappingAnimation()
{
	if (Action* action = m_mainSp->getActionByTag(ACTION_TAG_TAPPING))
		m_mainSp->stopAction(action);

	m_mainSp->setRotation(0);
}

void snowfight::FingerTap::startTappingAnimation()
{
	this->stopTappingAnimation();

	Vector<FiniteTimeAction*> arrayOfActions;

	auto* pressRotate = EaseCubicActionOut::create(RotateBy::create(0.2f, 18.f));
	auto* releaseRotate = RotateBy::create(0.05f, -18.f);
	auto* repeat = Repeat::create(Sequence::create(pressRotate, releaseRotate, DelayTime::create(0.4f), nullptr), pow(2, 30));
	arrayOfActions.pushBack(repeat);

	auto* seq = Sequence::create(arrayOfActions);
	seq->setTag(ACTION_TAG_TAPPING);

	m_mainSp->runAction(seq);
}


NS_END