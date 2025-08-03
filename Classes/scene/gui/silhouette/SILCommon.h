#ifndef __SIL_COMMON_H__
#define __SIL_COMMON_H__

#include "common/Common.h"

USING_NS_CC;

NS_BEGIN

extern const uint32_t FLAGS_SILHOUETTE_RENDER_EFFECT;
extern const uint32_t FLAGS_SILHOUETTE_RENDER_MASK;
extern const uint32_t FLAGS_RENDER_SILHOUETTE;

extern const Vec3 SILHOUETTE_NODE_MASK_COLOR;
extern const GLubyte SILHOUETTE_EFFECT_OPACITY_DEFAULT;

Vec2 getScreenPixelSize();

NS_END

#endif // __SIL_COMMON_H__
