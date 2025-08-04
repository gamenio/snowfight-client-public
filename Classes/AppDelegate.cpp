#include "AppDelegate.h"

#include <vector>
#include <string>

#include "asio/impl/src.hpp"

#include "common/Machine.h"
#include "common/utils/StringUtility.h"
#include "game/World.h"
#include "game/store/Store.h"
#include "game/UserPreferences.h"
#include "scene/SplashScene.h"


USING_NS_CC;
USING_NS;
using namespace std;

AppDelegate::AppDelegate() :
        m_isAppInBackground(false)
{
}

AppDelegate::~AppDelegate() 
{
}

// if you want a different context, modify the value of glContextAttrs
// it will affect all platforms
void AppDelegate::initGLContextAttrs()
{
    // set OpenGL context attributes: red,green,blue,alpha,depth,stencil
    GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8};

    GLView::setGLContextAttrs(glContextAttrs);
}

// if you want to use the package manager to install more packages,  
// don't modify or remove this function
static int register_all_packages()
{
	return 0; //flag for packages manager
}

bool AppDelegate::applicationDidFinishLaunching() {

	// initialize director
    auto director = Director::getInstance();
    auto glview = director->getOpenGLView();
    if(!glview) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
		Size screenSize = sUserPreferences->getScreenSize(narrowDesignResolutionSize);
		float frameZoomFactor = 1.0f;
#if USE_DEBUG_OPTION
		if (sUserPreferences->isDebugOptionEnabled(DEBUG_OPTION_ZOOM_IN_WINDOW))
			frameZoomFactor = 2.0f;
#endif // USE_DEBUG_OPTION
        glview = GLViewImpl::createWithRect("SnowFight", cocos2d::Rect(0, 0, screenSize.width, screenSize.height), frameZoomFactor);
#else
        glview = GLViewImpl::create("SnowFight");
#endif
        director->setOpenGLView(glview);
    }
    
	director->setDepthTest(false);
	director->setAlphaBlending(true);
	director->setProjection(Director::Projection::DEFAULT);

    // turn on display FPS
#if NS_DEBUG
    director->setDisplayStats(true);
#endif

    // set FPS. the default value is 1.0/60 if you don't call this
    director->setAnimationInterval(1.0f / 60);

    // Set the design resolution
	if (Machine::instance()->isNarrowScreen())
		glview->setDesignResolutionSize(narrowDesignResolutionSize.width, narrowDesignResolutionSize.height, ResolutionPolicy::FIXED_HEIGHT);
	else
		glview->setDesignResolutionSize(designResolutionSize.width, designResolutionSize.height, ResolutionPolicy::FIXED_HEIGHT);

    // Disable multiple touch
	Machine::instance()->setMultipleTouchEnabled(false);

	// Resource configuration
	int maxTexSize;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
    auto frameSize = glview->getFrameSize();
	vector<string> searchPaths;
	if (frameSize.height - mediumResource.size.height > DESIGN_RESOLUTION_MATCHING_RANGE && maxTexSize >= largeResource.requiredTexSize)
	{
		searchPaths.push_back(largeResource.directory);
		director->setContentScaleFactor(MIN(largeResource.size.height / designResolutionSize.height, largeResource.size.width / designResolutionSize.width));
	}
	else
	{
		searchPaths.push_back(mediumResource.directory);
		director->setContentScaleFactor(MIN(mediumResource.size.height / designResolutionSize.height, mediumResource.size.width / designResolutionSize.width));
	}
	if(!searchPaths.empty())
		FileUtils::getInstance()->setSearchPaths(searchPaths);

	// PVR file encryption key
	ZipUtils::setPvrEncryptionKey(PVR_ENCRYPTION_KEY[0], PVR_ENCRYPTION_KEY[1], PVR_ENCRYPTION_KEY[2], PVR_ENCRYPTION_KEY[3]);
	// PVR images are premultiplied
	Image::setPVRImagesHavePremultipliedAlpha(true);

	this->startSplashScene();

    return true;
}

void AppDelegate::applicationDidEnterBackground() {
    if(m_isAppInBackground)
    {
        CCLOG("Application are already in background.");
        return;
    }

	Director::getInstance()->stopAnimation();
	World::getInstance()->pause();
	Store::instance()->onEnterBackground();

    m_isAppInBackground = true;
}

void AppDelegate::applicationWillEnterForeground() {
    if(!m_isAppInBackground)
    {
        CCLOG("Application are already in foreground.");
        return;
    }

	World::getInstance()->resume();
	Director::getInstance()->startAnimation();
	Store::instance()->onEnterForeground();

    m_isAppInBackground = false;
}

void AppDelegate::startSplashScene()
{
	auto director = Director::getInstance();
	auto scene = SplashScene::createScene();
	director->runWithScene(scene);
}