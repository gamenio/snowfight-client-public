//
//  PropertyBar.h
//  snowfight
//
//  Created by Luthier on 2020/06/20.
//

#ifndef __PROPERTY_BAR_H__
#define __PROPERTY_BAR_H__

#include "ui/UIButton.h"

#include "common/Common.h"
#include "common/utils/Timer.h"
#include "InfoBubble.h"

USING_NS_CC;


NS_BEGIN

typedef std::function<void(Ref*)> PropertyBarClickCallback;

class PropertyBar: public ui::Widget, public ActionTweenDelegate
{
public:
	enum TipsType
	{
		TIPS_NONE,
		TIPS_WATCH_AD_FIRST,
		TIPS_WATCH_AD_REMINDED,
	};

    static PropertyBar* create();
    
    bool init() override;
    
	PropertyBar();
    ~PropertyBar();

	void setWatchAdReminderEnabled(bool enabled);
	void setWatchAdReminderDelay(float delay);
	void showFirstWatchAdTips(float delay = 0);
	TipsType getShownTipsType() const { return m_shownTipsType; }
	void hideTips();

	// Update the amount of property
	// When animated is true, use delay to set the animation delay playing time
	void updateAmount(bool animated = false, float delay = 0);

	void setClickEventListener(PropertyBarClickCallback const& callback) { m_clickCallback = callback; }

	// ActionTweenDelegate
	void updateTweenAction(float value, const std::string& key) override;

	void onEnterTransitionDidFinish() override;
	void update(float delta) override;

	void onEnter() override;
	void setScale(float scale) override;

private:
	void onTouchWidget(Ref* sender, ui::Widget::TouchEventType event);

	void startCountingAnimation(int32 start, int32 end, float delay);
	void startPlusBtnAnimation(float delay);
	void setAmount(int32 amount, bool animated, float delay = 0.f);
	void updateAmountLabel(int32 amount);

	void updateWatchAdReminder(float delta);

	int32 m_amount;
	bool m_isWatchAdReminderEnabled;
	DelayTimer m_watchAdReminderDelayTimer;
	TipsType m_shownTipsType;
	PropertyBarClickCallback m_clickCallback;

    ui::Button* m_plusBtn;
	Label* m_amountLabel;
	InfoBubble* m_infoBubble;
};


NS_END


#endif // __PROPERTY_BAR_H__

