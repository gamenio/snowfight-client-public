#ifndef __GM_COMMAND_WICKET_H__
#define __GM_COMMAND_WICKET_H__

#include "ui/UIEditBox/UIEditBox.h"

#include "common/Common.h"
#include "scene/gui/Wicket.h"

USING_NS_CC;

NS_BEGIN

class GMCommandWicket : public Wicket, ui::EditBoxDelegate
{
public:
	static GMCommandWicket* create(Node* owner);

	virtual bool init(Node* owner);

	GMCommandWicket();
	~GMCommandWicket();

	virtual void onEnter() override;
	virtual void update(float delta) override;

	// EditBoxDelegate
	void editBoxTextChanged(ui::EditBox* editBox, std::string const& text) override;
	virtual void editBoxReturn(ui::EditBox* editBox) override;

	void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event) override;

private:
	void sendCommand();
	int32 addCmdToHistoryList(std::string const& cmd);
	void nextCommand();
	void prevCommand();
	void delCommand();

	void buttonDelCallback(Ref* sender);
	void buttonPrevCallback(Ref* sender);
	void buttonNextCallback(Ref* sender);
	void buttonCleanInputCallback(Ref* sender);
	void buttonEnterCallback(Ref* sender);

	void updateCleanButtonVisibility();

	Node* m_main;
	ui::EditBox* m_editBox;
	ui::Button* m_cleanInputBtn;
	int32 m_historyCmdIndex;
};


NS_END

#endif // __GM_COMMAND_WICKET_H__
