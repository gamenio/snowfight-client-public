#ifndef __WICKET_H__
#define __WICKET_H__

#include "ui/UIButton.h"
#include "ui/UIScale9Sprite.h"

#include "common/Common.h"

USING_NS_CC;

NS_BEGIN

typedef std::function<void(Ref*)> WicketButtonClickCallback; 

class Wicket : public LayerColor
{
public:
	enum ResultCode
	{
		RESULT_CODE_OK,
		RESULT_CODE_CANCEL
	};
	typedef std::function<void(ResultCode resultCode, ValueMapIntKey const& data)> ResultCallback;

	enum ButtonType
	{
		BUTTON_POSITIVE,
		BUTTON_NEUTRAL,
		BUTTON_NEGATIVE
	};

	enum ButtonAlignment
	{
		BUTTON_ALIGNMENT_AVERAGE,
		BUTTON_ALIGNMENT_RIGHT
	};

	Wicket();
	virtual ~Wicket() = 0;

	virtual bool init(Node* owner, cocos2d::Size const& frameSize, std::string const& titleText, bool showCloseButton, bool showButtonPanel, ButtonAlignment buttonAlignment = BUTTON_ALIGNMENT_RIGHT);
	virtual bool init(Node* owner, cocos2d::Size const& frameSize, Node* title, Node* buttonPanel, ButtonAlignment buttonAlignment, Sprite* background, ui::Button* closeBtn);

	void addPositiveButton(std::string const& title, WicketButtonClickCallback const& callback) { this->addButton(BUTTON_POSITIVE, title, callback); }
	void addNeutralButton(std::string const& title, WicketButtonClickCallback const& callback) { this->addButton(BUTTON_NEUTRAL, title, callback); }
	void addNegativeButton(std::string const& title, WicketButtonClickCallback const& callback) { this->addButton(BUTTON_NEGATIVE, title, callback); }

	void addTickButton(WicketButtonClickCallback const& callback, ButtonType type = BUTTON_POSITIVE);
	void addCrossButton(WicketButtonClickCallback const& callback, ButtonType type = BUTTON_NEGATIVE);

	void setContent(Node* node);
	ui::Button* getCloseButton() const { return m_closeBtn; }

	virtual bool onTouchBegan(Touch* touch, Event* event) override;
	virtual void onTouchEnded(Touch* touch, Event* event) override;
	virtual void onTouchCancelled(Touch* touch, Event* event) override;
	virtual void onTouchMoved(Touch* touch, Event* event) override;

	void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event) override;

	virtual void onEnter() override;

	void show();
	void close();
	// Get a Wicket with the specified name that is currently displayed.
	static Wicket* getWicketByName(std::string const& name);

	void setResultListener(ResultCallback const& callbak) { m_resultCallback = callbak; }

protected:
	void setResult(ResultCode resultCode, ValueMapIntKey const& data);
	virtual void onClosed() {}

private:
	void initForm(Node* owner, cocos2d::Size const& frameSize, Node* title, Node* buttonPanel, ButtonAlignment buttonAlignment, Sprite* background, ui::Button* closeBtn);

	void buttonCloseCallback(Ref* sender);

	void setButtons(Vector<ui::Button*> const& m_buttons);
	void addButton(ButtonType buttonType, std::string const& title, WicketButtonClickCallback const& callback);

	void alignButtons();
	void layoutIfNeeded();
	void layoutElements();

	Node* m_owner;
	bool m_firstLayout;
	EventListenerTouchOneByOne* m_touchListener;

	Node* m_mainPanel;

	Node* m_title;
	Node* m_content;
	Sprite* m_background;
	Node* m_buttonPanel;
	ButtonAlignment m_buttonAlignment;
	ui::Button* m_closeBtn;

	ValueMapIntKey m_data;
	ResultCode m_resultCode;
	ResultCallback m_resultCallback;

};

NS_END


#endif // __WICKET_H__