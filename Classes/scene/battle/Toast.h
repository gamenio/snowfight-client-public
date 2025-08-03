#ifndef __TOAST_H__
#define __TOAST_H__

#include "common/Common.h"
#include "game/GameConfig.h"
#include "game/entities/ObjectGuid.h"

USING_NS_CC;

NS_BEGIN

#define TOAST_DURATION_FOREVER				0
#define TOAST_DURATION_SHORT				2.0f
#define TOAST_DURATION_LONG					5.0f

// Toast
class Toast : public Node
{
public:
	Toast();
	virtual ~Toast();

	virtual bool init() override;

	void hide();
	bool isHiding() const { return m_isHiding; }
	void setFlickering(bool flickering) { m_flickering = flickering; }

	void setDuration(float duration) { m_duration = duration; }
	float getDuration() const { return m_duration; }

	virtual void onEnter() override;

protected:
	virtual void layoutElements() { }

private:
	void show();

	bool m_isHiding;
	bool m_flickering;
	float m_duration;
};

// MessageToast
class MessageToast: public Toast
{
public:
	enum Severity
	{
		INFO			= 0,
		NOTICE			= 1,
		ALERT			= 2,
	};

	MessageToast();
	~MessageToast();

	static MessageToast* create();
	bool init() override;

	void setText(std::string const& text);
	void setSeverity(Severity severity);

	void layoutElements() override;

private:
	Label* m_label;
};

// ActionToast
class ActionToast : public Toast
{
public:
	enum ActionType
	{
		ACTION_TYPE_ACTOR1_LOGGED_IN,
		ACTION_TYPE_ACTOR1_KILL_ACTOR2,
		ACTION_TYPE_ACTOR1_DIED,
	};

	ActionToast();
	~ActionToast();

	static ActionToast* create(ActionType type);
	bool init(ActionType type);

	void setActor1(std::string const& name, ObjectGuid const& guid);
	void setActor2(std::string const& name, ObjectGuid const& guid);

	void layoutElements() override;

private:
	Label* addName1Label();
	Label* addName2Label();
	Label* addActionLabel(std::string const& text);
	Sprite* addActionIcon(std::string const& iconFrameName);
	void setNameLabelTextColor(Label* label, ObjectGuid const& guid);

	Label* m_actionLabel;
	Sprite* m_actionIcon;
	Label* m_name1Label;
	Label* m_name2Label;
};

NS_END

#endif // __TOAST_H__
