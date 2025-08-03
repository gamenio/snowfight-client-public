#ifndef __SHARE_H__
#define __SHARE_H__

#include "common/Common.h"

USING_NS_CC;

NS_BEGIN


struct ShareInfo
{
    std::string title;
    std::string text;
	std::string image;
    std::string link;
};

enum ShareState
{
    SHARE_STATE_NONE,
    SHARE_STATE_PRESENTED,
    SHARE_STATE_DONE,
    SHARE_STATE_FAIL,
    SHARE_STATE_CANCELLED,
};

enum ShareErrorCode
{
	SHARE_ERROR_NONE,
	SHARE_ERROR_NO_APP,
	SHARE_ERROR_UNKNOWN,
};

struct ShareResponse
{
	ShareResponse() :
		state(SHARE_STATE_NONE),
		description(""),
		errorCode(SHARE_ERROR_NONE) { }
    ShareState state;
    std::string description;
	ShareErrorCode errorCode;
};

class ShareListener
{
public:
    virtual void onShareState(ShareResponse const& response) {}
};

class Share
{
public:
	static Share* instance();

    void share(ShareInfo const& info, Vec2 const& popoverAnchor);

    void setListener(ShareListener* listener);
    ShareListener* getListener() const;
    void removeListener();
    
private:
	Share();
	~Share();
    
    ShareListener* m_listener;
};

#define sShare Share::instance()

NS_END



#endif // __SHARE_H__
