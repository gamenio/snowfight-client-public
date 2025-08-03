#ifndef __GAMBLE_DEFINES_H__
#define __GAMBLE_DEFINES_H__

#include "common/Common.h"

NS_BEGIN

// 
// 定义对象发生重叠时的渲染顺序
//
// 值的设定是相对对象当前TileZ值的一个增量。TileZ可以通过MapData::getTileZForPos()函数根据对象位置计算得出。
// 如果取正值则对象将显示在所在位置附着物的上方（如：树木、城墙），如果取负值则对象将显示在附着物的下方。
// 当对象重叠时取值大的显示在上方。
//
#define OVERLAY_AIM_POINTER						-10.f  // 在附着物的下方
#define OVERLAY_SLIDING_SNOWBALL				-20.f
#define OVERLAY_CORPSE							-40.f
#define OVERLAY_FOOTPRINT						-50.f


NS_END

#endif // __GAMBLE_DEFINES_H__

