#ifndef __AIM_POINTER_H__
#define __AIM_POINTER_H__

#include "common/Common.h"
#include "game/entities/DataUnit.h"
#include "scene/gui/silhouette/SILScale9Sprite.h"

USING_NS_CC;

NS_BEGIN

class AimPointer : public Node
{
public:
	static AimPointer* create();
	bool init() override;

	AimPointer();
	~AimPointer();

	void update(float delta) override;
	void setGlobalZOrder(float globalZOrder) override;

	void setDirection(float rad);
	float getDirection() const { return m_direction; }

	void setLength(float length);
	float getLength() const { return m_length; }
	void setWidth(float width);
	float getWidth() const { return m_width; }

	void reset();

private:
	float calcValidAngle(float angle);
	void updatePointer(float angle);

	float m_length;
	float m_width;
	float m_direction;
	float m_dstAngle;
	float m_currAngle;
	bool m_isFirstTick;

	SILScale9Sprite* m_pointerSp;
};


NS_END


#endif // __AIM_POINTER_H__
