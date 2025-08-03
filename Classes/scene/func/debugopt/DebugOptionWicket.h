#ifndef __DEBUG_OPTION_WICKET_H__
#define __DEBUG_OPTION_WICKET_H__

#include "common/Common.h"
#include "game/LocaleMgr.h"
#include "scene/gui/Wicket.h"
#include "scene/gui/CheckLabelBox.h"
#include "scene/gui/OptionButton.h"


USING_NS_CC;

NS_BEGIN

class DebugOptionWicket : public Wicket
{
public:
	enum DataKey
	{
		DATA_KEY_HEROS_UNLOCKED,
		DATA_KEY_REWARD_TIME_RESET,
	};

	static DebugOptionWicket* create(Node* owner);

	virtual bool init(Node* owner);

	DebugOptionWicket();
	~DebugOptionWicket();

	void onClosed() override;
    
    void updateTime(float dt);

private:
	ui::Button* createButton(std::string const& title);
	CheckLabelBox* createCheckLabelBox(std::string const& title);

	void checkboxGameMasterCallback(Ref* render, CheckLabelBox::EventType eventType);
	void checkboxZoomInWindowCallback(Ref* render, CheckLabelBox::EventType eventType);
	void checkboxDisableSessionTimeoutCallback(Ref* render, CheckLabelBox::EventType eventType);
	void checkboxKeyboardMouseCallback(Ref* render, CheckLabelBox::EventType eventType);
	void optionButtonCountryCallback(Ref* sender);
	void selectCountryCallback(Ref* sender, CountryInfo const& data);
	void buttonUnlockHeroesClickCallback(Ref* sender);
	void buttonClearUserDataClickCallback(Ref* sender);
    void buttonResetRewardTimeClickCallback(Ref* sender);
    
	Node* m_main;

	ValueMapIntKey m_data;

	CheckLabelBox* m_gameMasterClb;
	CheckLabelBox* m_zoomInWindowClb;
	OptionButton* m_countryOptBtn;
	Label* m_countryLabel;
	CheckLabelBox* m_disableSessionTimeoutClb;
	CheckLabelBox* m_keyboardMouseClb;
	ui::Button* m_unlockHeroesBtn;
	ui::Button* m_clearUserDataBtn;
    ui::Button* m_resetRewardTimeBtn;
	Label* m_statusLabel;
    Label* m_timeLabel;
};

NS_END

#endif // __DEBUG_OPTION_WICKET_H__
