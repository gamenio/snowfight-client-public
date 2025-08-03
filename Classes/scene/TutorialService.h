#ifndef __TUTORIAL_SERVICE_H__
#define __TUTORIAL_SERVICE_H__

#include "common/Common.h"
#include "game/entities/DataPlayer.h"

USING_NS_CC;

NS_BEGIN

enum TutorialEventType
{
	TUTORIAL_EVENT_PICKED_UP_ALL_MAGICBEANS,
	TUTORIAL_EVENT_CONTROL_ATTACK,
	TUTORIAL_EVENT_ITEMBOX_OPENED,
	TUTORIAL_EVENT_PICKED_UP_CONSUMABLE_ITEM,
	TUTORIAL_EVENT_PICKED_UP_EQUIPMENT,
	TUTORIAL_EVENT_CONTROL_SUPER_ATTACK,
	TUTORIAL_EVENT_ITEM_USED,
	TUTORIAL_EVENT_ENEMY_FOUND,
	TUTORIAL_EVENT_BATTLE_ENDING,
	TUTORIAL_EVENT_EQUIPMENT_BUTTON_TAPPED,
	TUTORIAL_EVENT_SMILEY_BOX_TAPPED,
};

class TutorialEventListener
{
public:
	virtual void onTutorialEventTriggered(TutorialEventType eventType) {}
};

class TutorialService
{
public:
	static TutorialService* getInstance();
	static void destoryInstance();

	bool isEnabled() const { return m_isEnabled; }
	void setEnabled(bool enabled) { m_isEnabled = enabled; }

	void setEventListener(TutorialEventListener* listener) { m_eventListener = listener; }

	bool isEventTriggered(TutorialEventType eventType) const;
	void triggerEvent(TutorialEventType eventType);
private:
	TutorialService();
	~TutorialService();

	bool m_isEnabled;
	TutorialEventListener* m_eventListener;
	std::unordered_set<int32> m_triggeredEvents;
};

NS_END

#endif // __TUTORIAL_SERVICE_H__