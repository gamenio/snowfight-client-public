#ifndef __MODAL_DIALOG_H__
#define __MODAL_DIALOG_H__

#include "ui/UIButton.h"
#include "ui/UIScale9Sprite.h"

#include "common/Common.h"
#include "game/LocaleMgr.h"

USING_NS_CC;

NS_BEGIN

class ModalDialog : public LayerColor
{
public:
	static ModalDialog* create(cocos2d::Size const& frameSize);
	bool init(cocos2d::Size const& frameSize);

	ModalDialog();
	virtual ~ModalDialog();

	Node* getMainPanel() const { return m_mainPanel; }

	void setTitleBar(Node* titleBar);
	void setContent(Node* content);
	void setButtonPanel(Node* buttonPanel);
	void setBackground(Sprite* background);
	void setButtons(Vector<ui::Widget*> const& buttons);

	virtual bool onTouchBegan(Touch* touch, Event* event) override;
	virtual void onTouchEnded(Touch* touch, Event* event) override;
	virtual void onTouchCancelled(Touch* touch, Event* event) override;
	virtual void onTouchMoved(Touch* touch, Event* event) override;

	void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event) override;

	void show();
	void dismiss();
	// Get a dialog with the specified name that is currently displayed.
	static ModalDialog* getDialogByName(std::string const& name);

protected:
	virtual void showIn(Scene* scene);

	virtual void onWillAppear() {}
	virtual void onAppeared() {}
	virtual void onWillDisappear() {}
	virtual void onDisappeared() {}
    
	Node* m_owner;

	Node* m_mainPanel;

	Node* m_titleBar;
	Node* m_content;
	Sprite* m_background;
	Node* m_buttonPanel;
};

typedef std::function<void(Ref*)> DialogCallback;

class MessageDialog: public ModalDialog
{
public:
	enum ButtonType
	{
		BUTTON_POSITIVE,
		BUTTON_NEUTRAL,
		BUTTON_NEGATIVE,
		BUTTON_TYPES
	};


	static MessageDialog* create();
	bool init() override;

	MessageDialog();
	~MessageDialog();

	void setTitle(std::string const& title);
	void setMessage(std::string const& message);

	void addPositiveButton(std::string const& title, DialogCallback const& callback = nullptr) { this->setButton(BUTTON_POSITIVE, title, callback); }
	void addNeutralButton(std::string const& title, DialogCallback const& callback = nullptr) { this->setButton(BUTTON_NEUTRAL, title, callback); }
	void addNegativeButton(std::string const& title, DialogCallback const& callback = nullptr) { this->setButton(BUTTON_NEGATIVE, title, callback); }

	void addOkButton(DialogCallback const& callback, ButtonType type = BUTTON_POSITIVE) { this->setButton(type, sLocaleMgr->getString("message_dialog_positive_button"), callback); }
	void addCancelButton(DialogCallback const& callback, ButtonType type = BUTTON_NEGATIVE) { this->setButton(type, sLocaleMgr->getString("message_dialog_negative_button"), callback); }
	void addTickButton(DialogCallback const& callback, ButtonType type = BUTTON_POSITIVE);
	void addCrossButton(DialogCallback const& callback, ButtonType type = BUTTON_NEGATIVE);

	void onEnter() override;

private:
	void setButton(ButtonType type, std::string const& title, DialogCallback const& callback);
	void layoutTitleBar();

	ui::Scale9Sprite* m_titleBg;
	Label* m_titleLabel;
	Label* m_messageLabel;

	Menu* m_menu;
	ui::Button* m_buttons[BUTTON_TYPES];
};


class ProgressDialog : public ModalDialog
{
public:

};

NS_END

#endif // __MODAL_DIALOG_H__
