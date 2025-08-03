#include "Share.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)

#include "platform/android/jni/JniHelper.h"

NS_BEGIN


static const std::string shareHelperClassName = "io/gamen/snowfight/ShareHelper";

enum ErrorCode
{
    ERROR_NO_APP,
    ERROR_UNKNOWN,
};

static void onSharePresented();
static void onShareCompleted();
static void onShareCanceled();
static void onShareFailed(int32 errorCode);
extern "C"{
    JNIEXPORT void
    JNICALL Java_io_gamen_snowfight_ShareHelper_onSharePresented(JNIEnv* env, jclass) {
        onSharePresented();
    }
    JNIEXPORT void
    JNICALL Java_io_gamen_snowfight_ShareHelper_onShareCompleted(JNIEnv* env, jclass) {
        onShareCompleted();
    }
    JNIEXPORT void
    JNICALL Java_io_gamen_snowfight_ShareHelper_onShareCanceled(JNIEnv* env, jclass) {
        onShareCanceled();
    }
    JNIEXPORT void
    JNICALL Java_io_gamen_snowfight_ShareHelper_onShareFailed(JNIEnv* env , jclass type, jint errorCode) {
        onShareFailed(errorCode);
    }
}

void Share::share(const ShareInfo &info, Vec2 const& popoverAnchor)
{
    JniHelper::callStaticVoidMethod(shareHelperClassName, "share", info.title, info.text, info.image, info.link);
}

void onSharePresented()
{
    if(sShare->getListener())
    {
        ShareResponse resp;
        resp.state = SHARE_STATE_PRESENTED;
        sShare->getListener()->onShareState(resp);
    }
}

void onShareCompleted()
{
    if(sShare->getListener())
    {
        ShareResponse resp;
        resp.state = SHARE_STATE_DONE;
        sShare->getListener()->onShareState(resp);
    }
}

void onShareCanceled()
{
    if(sShare->getListener())
    {
        ShareResponse resp;
        resp.state = SHARE_STATE_CANCELLED;
        sShare->getListener()->onShareState(resp);
    }
}

void onShareFailed(int32 errorCode)
{
    if(sShare->getListener())
    {
        ShareResponse resp;
        switch(errorCode)
        {
            case ERROR_NO_APP:
                resp.errorCode = ShareErrorCode::SHARE_ERROR_NO_APP;
                break;
            default:
                resp.errorCode = ShareErrorCode::SHARE_ERROR_UNKNOWN;
                break;
        }
        resp.state = SHARE_STATE_FAIL;
        sShare->getListener()->onShareState(resp);
    }
}

NS_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID