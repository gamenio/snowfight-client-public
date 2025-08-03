#ifndef __NOTIFICATION_SERVICE_H__
#define __NOTIFICATION_SERVICE_H__

#include "cocos2d.h"
#include "MessageDispatcher.h"

NS_BEGIN

class NotificationService: public MessageDispatcher
{
public:
	static NotificationService* instance();

	void update(float dt);

private:
	NotificationService();
	~NotificationService();

	cocos2d::Scheduler* m_scheduler;
};

#define sNotificationService NotificationService::instance()

NS_END


#endif // __NOTIFICATION_SERVICE_H__