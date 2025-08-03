#include "PreparationTimer.h"

#include "common/utils/TimeUtil.h"
#include "scene/SoundMgr.h"
#include "scene/Utils.h"

NS_BEGIN

#define ACTION_TAG_SCALE							1

#define PREPARATION_NUM_FRAMENAME					"preparation_num%d.png"
#define PREPARATION_START_FRAMENAME					"preparation_start.png"

#define PREPARATION_NUM_MIN							1
#define PREPARATION_NUM_MAX							9

PreparationTimer::PreparationTimer() :
	m_duration(0),
	m_startTime(0),
	m_currTimeLeft(0),
	m_isStopped(true),
	m_numberSp(nullptr)
{
}

PreparationTimer::~PreparationTimer()
{
	m_numberSp = nullptr;
}

PreparationTimer* PreparationTimer::create()
{
	PreparationTimer* pRet = new PreparationTimer();
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

bool PreparationTimer::init()
{
	if (!Node::init())
		return false;

	m_numberSp = Sprite::createWithSpriteFrameName(StringUtils::format(PREPARATION_NUM_FRAMENAME, PREPARATION_NUM_MIN));
	m_numberSp->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_numberSp->setVisible(false);
	this->addChild(m_numberSp);

	return true;
}

void PreparationTimer::start(NSTime milliseconds)
{
	if (!m_isStopped)
		this->stop();

	m_startTime = time_util::getUptimeMillis();
	m_duration = milliseconds;

	this->updateTimeLeft((int32)std::ceil(milliseconds / 1000.0f));
	m_numberSp->setVisible(true);

	this->scheduleUpdate();

	m_isStopped = false;
}

void PreparationTimer::stop()
{
	if (m_isStopped)
		return;

	m_startTime = 0;
	m_currTimeLeft = 0;
	m_isStopped = true;
	this->unscheduleUpdate();
}

void PreparationTimer::onEnter()
{
	Node::onEnter();
}

void PreparationTimer::update(float delta)
{
	if (m_isStopped)
		return;

	NSTime elapsedTime = time_util::getUptimeMillis() - m_startTime;
	int32 timeLeft = std::ceil((m_duration - elapsedTime) / 1000.0f);
	if (timeLeft >= 0)
		this->updateTimeLeft(timeLeft);
	else
	{
		this->stop();
		m_numberSp->setVisible(false);
	}
}

void PreparationTimer::updateTimeLeft(int32 seconds)
{
	if (m_currTimeLeft == seconds)
		return;

	m_currTimeLeft = seconds;

	if (seconds <= 0)
	{
		m_numberSp->setSpriteFrame(PREPARATION_START_FRAMENAME);
		sSoundMgr->play(SOUND_BATTLE_START);
	}
	else
	{
		int32 num = std::max(PREPARATION_NUM_MIN, std::min(seconds, PREPARATION_NUM_MAX));
		m_numberSp->setSpriteFrame(StringUtils::format(PREPARATION_NUM_FRAMENAME, num));
		sSoundMgr->play(SOUND_BATTLE_COUNTDOWN);
	}

	if (Action* action = m_numberSp->getActionByTag(ACTION_TAG_SCALE))
		m_numberSp->stopAction(action);

	m_numberSp->setScale(0.35f);
	auto action = EaseBackOut::create(ScaleTo::create(0.5f, 1.0f));
	action->setTag(ACTION_TAG_SCALE);
	m_numberSp->runAction(action);

	//CCLOG("Time left: %d", m_currTimeLeft);
}

NS_END
