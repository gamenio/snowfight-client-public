#include "PickupProgressBar.h"

#include "common/utils/TimeUtil.h"
#include "scene/SoundMgr.h"
#include "scene/Utils.h"

NS_BEGIN

#define FULL_PERCENTAGE								100
#define ACTION_TAG_PROGRESS							1

#define FRAMENAME_PICKUP_PROG_FG					"pickup_prog_fg.png"
#define FRAMENAME_PICKUP_PROG_BG					"pickup_prog_bg.png"

PickupProgressBar::PickupProgressBar() :
	m_isStopped(true),
	m_progBg(nullptr),
	m_progTimer(nullptr)
{
}

PickupProgressBar::~PickupProgressBar()
{
	m_progBg = nullptr;
	m_progTimer = nullptr;
}

PickupProgressBar* PickupProgressBar::create()
{
	PickupProgressBar* pRet = new PickupProgressBar();
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

bool PickupProgressBar::init()
{
	if (!Node::init())
		return false;

	m_progBg = SILSprite::createWithSpriteFrameName(FRAMENAME_PICKUP_PROG_BG);
	m_progBg->setPosition(Point::ZERO);
	m_progBg->setSilhouetted(false);
	m_progBg->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	this->addChild(m_progBg);

	m_progTimer = SILProgressTimer::create(Sprite::createWithSpriteFrameName(FRAMENAME_PICKUP_PROG_FG));
	m_progTimer->setSilhouetted(false);
	m_progTimer->setPosition(m_progBg->getPosition());
	m_progTimer->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_progTimer->setType(ProgressTimer::Type::RADIAL);
	m_progTimer->setPercentage(0);
	this->addChild(m_progTimer);

	this->setContentSize(m_progBg->getContentSize());

	return true;
}

void PickupProgressBar::onEnter()
{
	Node::onEnter();
}

void PickupProgressBar::start(NSTime elapsedTime, NSTime duration)
{
	NS_ASSERT(elapsedTime < duration);

	if (!m_isStopped)
		this->stop();

	float percentage = elapsedTime / (float)duration * FULL_PERCENTAGE;
	float durInSec = time_util::toGameTimeSeconds(duration - elapsedTime);
	this->startProgressAction(durInSec, percentage, FULL_PERCENTAGE);

	m_isStopped = false;
}

void PickupProgressBar::stop()
{
	if (m_isStopped)
		return;

	this->stopProgressAction();

	m_progTimer->setPercentage(0);

	m_isStopped = true;
}

void PickupProgressBar::setGlobalZOrder(float globalZOrder)
{
	Node::setGlobalZOrder(globalZOrder);

	m_progTimer->setGlobalZOrder(globalZOrder);
	m_progBg->setGlobalZOrder(globalZOrder);
}

void PickupProgressBar::startProgressAction(float duration, float fromPercentage, float toPercentage)
{
	this->stopProgressAction();

	m_progTimer->setPercentage(fromPercentage);
	CallFunc* finishCallback = CallFunc::create([](){
		//sSoundMgr->play(SOUND_PICKED_UP_ITEM);
	});
	ProgressFromTo* progress = ProgressFromTo::create(duration, fromPercentage, toPercentage);
	Sequence* action = Sequence::createWithTwoActions(progress, finishCallback);
	action->setTag(ACTION_TAG_PROGRESS);
	m_progTimer->runAction(action);
}


void PickupProgressBar::stopProgressAction()
{
	if (Action* action = m_progTimer->getActionByTag(ACTION_TAG_PROGRESS))
		m_progTimer->stopAction(action);
}

NS_END
