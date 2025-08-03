#include "TutorialService.h"

NS_BEGIN

static TutorialService* s_instance = nullptr;

TutorialService* TutorialService::getInstance()
{
	if (!s_instance)
	{
		s_instance = new (std::nothrow) TutorialService;
		NS_ASSERT_LOG(s_instance, "FATAL: Not enough memory");
	}
	return s_instance;
}

void TutorialService::destoryInstance()
{
	CC_SAFE_DELETE(s_instance);
}

bool TutorialService::isEventTriggered(TutorialEventType eventType) const
{
	return m_triggeredEvents.find(eventType) != m_triggeredEvents.end();
}

void TutorialService::triggerEvent(TutorialEventType eventType)
{
	if (!m_isEnabled || !m_eventListener)
		return;

	if (m_triggeredEvents.find(eventType) == m_triggeredEvents.end())
		m_triggeredEvents.insert(eventType);
	m_eventListener->onTutorialEventTriggered(eventType);
}

TutorialService::TutorialService() :
	m_isEnabled(false),
	m_eventListener(nullptr)
{

}

TutorialService::~TutorialService()
{
	m_eventListener = nullptr;
}


NS_END