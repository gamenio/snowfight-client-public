#include "Share.h"


NS_BEGIN

Share::Share() :
    m_listener(nullptr)
{

}

Share::~Share()
{
    m_listener = nullptr;
}

Share* Share::instance()
{
	static Share instance;
	return &instance;
}

#if ((CC_TARGET_PLATFORM != CC_PLATFORM_IOS) && (CC_TARGET_PLATFORM != CC_PLATFORM_ANDROID))
void Share::share(ShareInfo const& info, Vec2 const& popoverAnchor) { }
#endif

void Share::setListener(ShareListener* listener)
{
    m_listener = listener;
}

void Share::removeListener()
{
    m_listener = nullptr;
}

ShareListener *Share::getListener() const
{
    return m_listener;
}

NS_END

