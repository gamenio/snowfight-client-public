#include "MyMoveSpline.h"

#include "common/utils/MathTools.h"
#include "game/movement/MoveCollisionImpl.h"
#include "game/behaviors/MyCharacter.h"


NS_BEGIN

MyMoveSpline::MyMoveSpline(MyCharacter* owner):
	m_owner(owner),
	m_isFinished(true),
	m_moveCollision(nullptr),
	m_isMoveTurnEnabled(true),
	m_heartbeatCounter(0),
	m_isPaused(false),
	m_heartbeatTime(0),
	m_latency(0)
{
}

MyMoveSpline::~MyMoveSpline()
{
	CC_SAFE_DELETE(m_moveCollision);
	m_owner = nullptr;
}

void MyMoveSpline::update(float delta)
{
	if (!m_moveCollision)
		return;

	if (!m_isPaused)
	{
		float diff = m_heartbeatTimer.getInterval() - m_heartbeatTimer.getCurrent();
		float updateDt = std::min(std::max(0.f, diff), delta);
		m_moveCollision->step(updateDt);

		// 定时发送心跳包
		m_heartbeatTimer.update(delta);
		if (m_heartbeatTimer.passed())
		{
			// CCLOG("heartbeatCounter:%d", m_heartbeatCounter);
			//  如果有心跳包没有得到服务器确认则暂停移动
			if (m_heartbeatCounter > 0)
				m_isPaused = true;
			else
				this->sendHeartbeat();

			// 更新心跳的间隔时间
			this->updateHeartbeatInterval();
		}
	}
}


void MyMoveSpline::setMoveTurnEnabled(bool isEnabled)
{
	if (m_isMoveTurnEnabled != isEnabled)
	{
		m_isMoveTurnEnabled = isEnabled;
		if (isEnabled && m_moveCollision)
		{
			m_owner->setMoveTurnAngle(m_moveCollision->getDirection());
		}
	}
}

void MyMoveSpline::moveByDirection(float rad)
{
	m_isFinished = false;

	// 是否可以旋转角色
	if (m_isMoveTurnEnabled)
	{
		m_owner->setMoveTurnAngle(rad);
	}

	m_owner->getData()->addMovementFlag(MOVEMENT_FLAG_WALKING);

	// 创建碰撞移动
	if (!m_moveCollision)
	{
		m_moveCollision = new MoveCollision<MyCharacter>(m_owner);
	}
	m_moveCollision->setDirection(rad);
}

void MyMoveSpline::ackHeartbeat()
{
	if(m_heartbeatCounter > 0)
		--m_heartbeatCounter;

	if (m_heartbeatTime > 0)
	{
		NSTime nowTime = time_util::getUptimeMillis();
		m_latency = nowTime - m_heartbeatTime;
	}

	if (m_isPaused)
	{
		if (m_heartbeatCounter <= 0)
		{
			m_isPaused = false;
			this->sendHeartbeat();
		}
	}
	//CCLOG("HEARTBEAT ACK counter: %d", m_heartbeatCounter);
}

void MyMoveSpline::sendHeartbeat()
{
	if (!m_owner->getSession())
		return;

	MovementInfo movement;
	m_owner->buildMovementInfo(movement);

	WorldPacket packet(world::MSG_MOVE_HEARTBEAT, std::move(movement));
	m_owner->getSession()->sendPacket(std::move(packet));
		
	++m_heartbeatCounter;
	m_heartbeatTime = time_util::getUptimeMillis();
	//CCLOG("MSG_MOVE_HEARTBEAT counter: %d", m_heartbeatCounter);
}

void MyMoveSpline::updateHeartbeatInterval()
{
	float interval = MOVING_STEP_LENGTH_MIN / m_owner->getData()->getMoveSpeed();
	float delay = m_latency / 1000.f;
	interval = std::max(interval, delay);
	interval = std::max(MOVING_HEARTBEAT_INTERVAL_MIN, std::min(MOVING_HEARTBEAT_INTERVAL_MAX, interval));
	m_heartbeatTimer.setInterval(interval);
	//CCLOG("heartbeat interval: %f latency: %f", interval, delay);
}

void MyMoveSpline::finish()
{
	if (m_isFinished)
		return;

	CC_SAFE_DELETE(m_moveCollision);
	m_owner->getData()->clearMovementFlag(MOVEMENT_FLAG_WALKING);

	m_isFinished = true;
}

bool MyMoveSpline::stop()
{
	this->finish();
	return true;
}

void MyMoveSpline::resetSyncState()
{
	m_isPaused = false;
	m_heartbeatCounter = 0;
	m_latency = 0;
	m_heartbeatTime = 0;
	this->updateHeartbeatInterval();
}

NS_END
