#ifndef __REVIEW_DIALOG_H__
#define __REVIEW_DIALOG_H__

#include "scene/gui/ModalDialog.h"

#include "common/Common.h"
#include "game/LocaleMgr.h"

USING_NS_CC;

NS_BEGIN

class ReviewDialog: public ModalDialog
{
public:
	enum ButtonType
	{
		BUTTON_POSITIVE,
		BUTTON_NEUTRAL,
		BUTTON_NEGATIVE,
		BUTTON_TYPES
	};


	static ReviewDialog* create();
	bool init() override;

	ReviewDialog();
	~ReviewDialog();

	void setTitle(std::string const& title);
	void setMessage(std::string const& message);

	void addPositiveButton(std::string const& title, DialogCallback const& callback = nullptr) { this->setButton(BUTTON_POSITIVE, title, callback); }
	void addNeutralButton(std::string const& title, DialogCallback const& callback = nullptr) { this->setButton(BUTTON_NEUTRAL, title, callback); }
	void addNegativeButton(std::string const& title, DialogCallback const& callback = nullptr) { this->setButton(BUTTON_NEGATIVE, title, callback); }

	void addOkButton(DialogCallback const& callback, ButtonType type = BUTTON_POSITIVE) { this->setButton(type, sLocaleMgr->getString("message_dialog_positive_button"), callback); }
	void addCancelButton(DialogCallback const& callback, ButtonType type = BUTTON_NEGATIVE) { this->setButton(type, sLocaleMgr->getString("message_dialog_negative_button"), callback); }

	void onEnter() override;

private:
	void setButton(ButtonType type, std::string const& title, DialogCallback const& callback);

	ui::Scale9Sprite* m_titleBg;
	Label* m_titleLabel;
	Node* m_titleNode;
	Label* m_messageLabel;

	Menu* m_menu;
	ui::Button* m_buttons[BUTTON_TYPES];
};


NS_END

#endif // __REVIEW_DIALOG_H__
