#ifndef __GAMBLE_H__
#define __GAMBLE_H__

#include "common/Common.h"
#include "game/entities/DataBasic.h"

NS_BEGIN

// 场景中可以进行数据更新和状态同步的对象的接口类
class Gamble
{
public:
	Gamble() {}
	virtual ~Gamble() { }

	// 对象的活跃状态
	virtual void onActivated() = 0;
	virtual void onInactivated() = 0;
	virtual bool isActive() const = 0;

	// 清理数据的更新掩码，通常在对象更新后调用
	virtual void cleanUpdateMask() = 0;

	// 对象的数据
	virtual DataBasic* getData() const = 0;
};

NS_END

#endif // __GAMBLE_H__

