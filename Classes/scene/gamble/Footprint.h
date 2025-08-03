#ifndef __FOOTPRINT_H__
#define __FOOTPRINT_H__

#include "common/Common.h"
#include "scene/ObjectPools.h"

USING_NS_CC;

NS_BEGIN

class Footprint: public Sprite
{
public:
	static Footprint* create();
	bool init() override;

	Footprint();
	virtual ~Footprint();

	void reset();
	void run(cocos2d::Point const& pos, float orient, float zOrder);

private:
	void recycle();

	bool m_recycled;
};

typedef ObjectPool<Footprint> FootprintPool;

NS_END

#endif // __FOOTPRINT_H__
