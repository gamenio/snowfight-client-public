#include "BattleTimer.h"

#include "common/utils/TimeUtil.h"
#include "scene/Utils.h"

NS_BEGIN

#define TIME_FORMAT									"%02d:%02d"

BattleTimer::BattleTimer() :
	m_duration(0),
	m_startTime(0),
	m_currTimeLeft(0),
	m_isCountdownStopped(true),
	m_timeLabel(nullptr)
{
}

BattleTimer::~BattleTimer()
{
	m_timeLabel = nullptr;
}

BattleTimer* BattleTimer::create()
{
	BattleTimer* pRet = new BattleTimer();
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

bool BattleTimer::init()
{
	if (!Node::init())
		return false;
	
	this->setIgnoreAnchorPointForPosition(false);

	m_timeLabel = Label::createWithSystemFont("00:00", DEFAULT_SYSTEM_FONT, 16);
	m_timeLabel->setTextColor(Color4B(102, 108, 124, 255));
	m_timeLabel->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_timeLabel->setAlignment(TextHAlignment::CENTER);
	Utils::enableBoldForLabel(m_timeLabel);
	this->addChild(m_timeLabel);

	this->setContentSize(m_timeLabel->getContentSize());

	this->scheduleUpdate();

	return true;
}

void BattleTimer::setDuration(NSTime milliseconds)
{
	m_duration = milliseconds;
	this->updateTimeLeft((int32)std::ceil(m_duration / 1000.0f));
}

void BattleTimer::start()
{
	if (!m_isCountdownStopped)
		this->stop();

	if (m_duration > 0)
	{
		m_startTime = time_util::getUptimeMillis();
		m_isCountdownStopped = false;
	}
}

void BattleTimer::stop()
{
	if (m_isCountdownStopped)
		return;

	m_startTime = 0;
	m_currTimeLeft = 0;
	m_isCountdownStopped = true;
}

void BattleTimer::onEnter()
{
	Node::onEnter();
}

void BattleTimer::update(float delta)
{
	if (!m_isCountdownStopped)
	{
		NSTime elapsedTime = time_util::getUptimeMillis() - m_startTime;
		int32 timeLeft = std::ceil((m_duration - elapsedTime) / 1000.0f);
		if (timeLeft >= 0)
			this->updateTimeLeft(timeLeft);
		else
		{
			this->updateTimeLeft(0);
			this->stop();
		}
	}
}

void BattleTimer::updateTimeLeft(int32 seconds)
{
	if (m_currTimeLeft == seconds)
		return;

	m_currTimeLeft = seconds;
	int32 min = (int32)(seconds / 60);
	int32 sec = seconds % 60;
	m_timeLabel->setString(StringUtils::format(TIME_FORMAT, min, sec));
	//CCLOG("Time left: %d", m_currTimeLeft);
}

NS_END