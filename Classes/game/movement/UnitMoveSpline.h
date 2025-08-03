#ifndef __UNIT_MOVE_SPLINE_H__
#define __UNIT_MOVE_SPLINE_H__

#include "common/Common.h"
#include "common/utils/Timer.h"
#include "game/movement/MoveSpline.h"
#include "game/entities/MovementInfo.h"
#include "game/tiles/TileCoord.h"


NS_BEGIN

template<typename T> class MoveSegment;
class Unit;

class UnitMoveSpline: public MoveSpline
{
public:
	UnitMoveSpline(Unit* owner);
	~UnitMoveSpline();

	virtual void update(float delta) override;

	void moveBy(MovementInfo const& movement);
	MoveSegment<Unit>* getMoveSegment() const { return m_currSegment; }

	virtual bool isFinished() const override { return m_isFinished; }
	bool isCleanupMoveFlags() const { return m_isCleanupMoveFlags; }
	void setCleanupMoveFlags(bool isCleanup) { m_isCleanupMoveFlags = isCleanup; }
	virtual bool stop() override { return this->stop(false); }
	// 当isCorrectPosition=true时如果有未完成的移动分段则直接跳到分段结束位置
	virtual bool stop(bool isCorrectPosition);

private:
	void process(MovementInfo const& movement);
	void stopSegment();
	void finish(bool isCorrectPosition);

	Unit* m_owner;
	
	bool m_isFinished;
	bool m_isCleanupMoveFlags;
	std::vector<MovementInfo> m_movementQueue;
	MoveSegment<Unit>* m_currSegment;
	int32 m_realMoveSpeed;
};

NS_END

#endif //__UNIT_MOVE_SPLINE_H__
