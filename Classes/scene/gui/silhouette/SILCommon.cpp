#include "SILCommon.h"


NS_BEGIN

const uint32_t FLAGS_SILHOUETTE_RENDER_EFFECT = (1 << 14);
const uint32_t FLAGS_SILHOUETTE_RENDER_MASK = (1 << 15);
const uint32_t FLAGS_RENDER_SILHOUETTE = FLAGS_SILHOUETTE_RENDER_EFFECT | FLAGS_SILHOUETTE_RENDER_MASK;

const Vec3 SILHOUETTE_NODE_MASK_COLOR = Vec3(1.0, 0.0, 1.0);
const GLubyte SILHOUETTE_EFFECT_OPACITY_DEFAULT = (GLubyte)(0.3f * 255);

Vec2 getScreenPixelSize()
{
	GLView* glView = Director::getInstance()->getOpenGLView();
	auto screenSize = glView->getFrameSize();
#if (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
	screenSize = screenSize * glView->getFrameZoomFactor() * glView->getRetinaFactor();
#endif
	Vec2 screenPixelSize;
	screenPixelSize.x = 1.0f / screenSize.width;
	screenPixelSize.y = 1.0f / screenSize.height;

	return screenPixelSize;
}

NS_END