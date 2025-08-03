//
//  ShopRewardButton.h
//  snowfight
//
//  Created by Luthier on 2020/8/23.
//

#ifndef __SHOP_REWARD_BUTTON_H__
#define __SHOP_REWARD_BUTTON_H__

#include "ui/UIButton.h"

#include "common/Common.h"
#include "game/nts/TimeService.h"
#include "scene/gui/LoadingIndicator.h"

USING_NS_CC;


NS_BEGIN

typedef std::function<void(Ref*)> RewardTimerButtonClickCallback;

class ShopRewardButton: public ui::Widget
{
public:
    static ShopRewardButton* create();

	bool init() override;
    
    ShopRewardButton();
    ~ShopRewardButton();

	void updateState();
	bool isTimerEnabled() const { return m_isTimerEnabled; }
	bool isTimeUp() const { return m_isTimeUp; }

	void setTimeUpHintEnabled(bool enabled);

    void setClickEventListener(RewardTimerButtonClickCallback const& callback);

	void setTouchEnabled(bool enabled) override;
	void update(float delta) override;
	void onEnter() override;

private:
	void startHintAnimation();
	void stopHintAnimation();

    void buttonClickCallback(Ref* sender);

	void updateTitleWithTime(int32 milliseconds);
	void applyDefaultTitle();
	void updateTitleScale();
    void timeIsUp();

	void setShowLoadingIndicator(bool show);
    
	std::string m_defaultTitle;
    ui::Button* m_button;
	LoadingIndicator* m_loadingIndicator;
	uint32 m_timerEndTime;
	bool m_isTimerEnabled;
	bool m_isTimeUpHintEnabled;
	bool m_isTimeUp;

	RewardTimerButtonClickCallback m_clickCallback;
};


NS_END


#endif // __SHOP_REWARD_BUTTON_H__

