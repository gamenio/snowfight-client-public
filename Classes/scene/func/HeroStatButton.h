//
//  HeroStatButton.h
//  snowfight
//
//  Created by Luthier on 2019/11/19.
//

#ifndef __HERO_STAT_BUTTON_H__
#define __HERO_STAT_BUTTON_H__

#include "ui/UIButton.h"
#include "ui/UIScale9Sprite.h"

#include "common/Common.h"
#include "InfoBubble.h"

USING_NS_CC;


NS_BEGIN

typedef std::function<void(Ref*)> HeroStatButtonClickCallback;

class HeroStatButton: public Node
{
public:
	enum TipsType
	{
		TIPS_NONE				= 0,
		TIPS_VIEW_STATS,
		TIPS_UPGRADE_STATS,
	};


    static HeroStatButton* create();
    
    bool init() override;
    
    HeroStatButton();
    ~HeroStatButton();

	void showTips(TipsType type, float delay = 0.f);
	void hideTips();
	TipsType getShownTipsType() const { return m_shownTipsType; }

    void setClickEventListener(HeroStatButtonClickCallback const& callback);

	void setScale(float scale) override;
	void onEnter() override;

private:
    void buttonClickCallback(Ref* sender);

	void startJumpAnimation(float interval);
	void startFinishJumpAnimation();
	void stopJumpAnimation();
    
	TipsType m_shownTipsType;
    ui::Button* m_iconBtn;
	Sprite* m_shadowSp;
	InfoBubble* m_infoBubble;

    HeroStatButtonClickCallback m_clickCallback;
};


NS_END


#endif // __HERO_STAT_BUTTON_H__

