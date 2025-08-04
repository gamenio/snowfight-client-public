#ifndef __UNIT_LOCATOR_MOVE_SPLINE_H__
#define __UNIT_LOCATOR_MOVE_SPLINE_H__

#include "common/Common.h"
#include "game/movement/MoveSpline.h"
#include "game/entities/LocationInfo.h"

NS_BEGIN

template<typename T> class MoveSegment;
class UnitLocator;


class UnitLocatorMoveSpline: public MoveSpline
{
public:
	UnitLocatorMoveSpline(UnitLocator* owner);
	~UnitLocatorMoveSpline();

	virtual void update(float delta) override;

	void moveBy(LocationInfo const& location);
	virtual bool isFinished() const override { return m_isFinished; }
	virtual bool stop() override { return this->stop(false); }
	// When isCorrectPosition=true if there is an undone move segment then jump directly to the segment end position.
	virtual bool stop(bool isCorrectPosition);

private:
	void process(LocationInfo const& location);
	void stopSegment();

	UnitLocator* m_owner;
	
	bool m_isFinished;
	std::vector<LocationInfo> m_locationQueue;
	MoveSegment<UnitLocator>* m_currSegment;
};

NS_END

#endif // __UNIT_LOCATOR_MOVE_SPLINE_H__
