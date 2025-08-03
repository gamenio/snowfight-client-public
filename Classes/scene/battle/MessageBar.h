#ifndef __MESSAGE_BAR_H__
#define __MESSAGE_BAR_H__

#include "common/Common.h"

USING_NS_CC;

NS_BEGIN

#define MESSAGEBAR_DURATION_FOREVER				0
#define MESSAGEBAR_DURATION_SHORT				2.0f
#define MESSAGEBAR_DURATION_LONG				5.0f

// Toast
class MessageBar : public Node
{
public:
	enum Severity
	{
		INFO		= 0,
		NOTICE		= 1,
		ALERT		= 2,
	};

	static MessageBar* create();

	MessageBar();
	virtual ~MessageBar();

	bool init() override;

	void show(std::string const& text, Severity severity, std::string const& icon = "", float duration = MESSAGEBAR_DURATION_SHORT);
	bool isShown() const { return m_isShown; }
	void hide();

	virtual void onEnter() override;

private:
	void setText(std::string const& text);
	void setIcon(std::string const& frameName);
	void setSeverity(Severity severity);

	void layoutElements();

	bool m_isShown;
	Sprite* m_iconSp;
	Label* m_textLabel;
};

NS_END

#endif // __MESSAGE_BAR_H__
