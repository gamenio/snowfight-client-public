#ifndef __GAMBLE_DEFINES_H__
#define __GAMBLE_DEFINES_H__

#include "common/Common.h"

NS_BEGIN

// 
// Define the rendering order when objects overlap.
//
// The value setting is an increment relative to the object's current TileZ value. 
// TileZ can be calculated based on the object's position using the MapData::getTileZForPos() function.
// If a positive value is taken, the object will be displayed above the static object at its position (e.g., trees, walls). 
// If a negative value is taken, the object will be displayed below the static object.
//
#define OVERLAY_AIM_POINTER						-10.f  // Below the static object
#define OVERLAY_SLIDING_SNOWBALL				-20.f
#define OVERLAY_CORPSE							-40.f
#define OVERLAY_FOOTPRINT						-50.f


NS_END

#endif // __GAMBLE_DEFINES_H__

