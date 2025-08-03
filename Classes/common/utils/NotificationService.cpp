#include "NotificationService.h"

NS_BEGIN

NotificationService* NotificationService::instance()
{
	static NotificationService instance;
	return &instance;
}

NotificationService::NotificationService() :
	m_scheduler(nullptr)
{
	m_scheduler = cocos2d::Director::getInstance()->getScheduler();
	CC_SAFE_RETAIN(m_scheduler);
	m_scheduler->scheduleUpdate(this, UPDATE_PRIORITY_WORLD, false);
}

NotificationService::~NotificationService()
{
	if (m_scheduler)
	{
		m_scheduler->unscheduleUpdate(this);
		CC_SAFE_RELEASE_NULL(m_scheduler);
	}
}

void NotificationService::update(float dt)
{
	this->dispatch();
}

NS_END