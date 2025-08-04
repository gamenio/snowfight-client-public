#ifndef __MY_MOVE_SPLINE_H__
#define __MY_MOVE_SPLINE_H__

#include "common/Common.h"
#include "common/utils/Timer.h"
#include "game/movement/MoveSpline.h"


NS_BEGIN

//
// The following configuration parameters need to be consistent with the server
//
#define MOVING_STEP_LENGTH_MIN						16.f // The minimum step length when moving. Unit: points
// Interval for reporting position information when moving. Unit: seconds
#define MOVING_HEARTBEAT_INTERVAL_MAX				0.2f
#define MOVING_HEARTBEAT_INTERVAL_MIN				0.1f

template<typename T> class MoveCollision;
class MyCharacter;

class MyMoveSpline : public MoveSpline
{
public:
	MyMoveSpline(MyCharacter* owner);
	~MyMoveSpline();

	void update(float delta) override;

	void setMoveTurnEnabled(bool isEnabled) override;

	void moveByDirection(float rad);
	bool isFinished() const override { return m_isFinished; }
	bool stop() override;
	void resetSyncState();

	void ackHeartbeat();

private:
	void finish();

	void sendHeartbeat();
	void updateHeartbeatInterval();

	MyCharacter* m_owner;

	bool m_isFinished;
	MoveCollision<MyCharacter>* m_moveCollision;
	bool m_isMoveTurnEnabled;

	IntervalTimer m_heartbeatTimer;
	int32 m_heartbeatCounter;
	bool m_isPaused;

	NSTime m_heartbeatTime;
	NSTime m_latency;
};

NS_END

#endif // __MY_MOVE_SPLINE_H__