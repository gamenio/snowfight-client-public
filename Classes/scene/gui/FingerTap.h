//
//  FingerTap.h
//  snowfight
//
//  Created by Luthier on 2021/10/19.
//

#ifndef __FINGER_TAP_H__
#define __FINGER_TAP_H__

#include "common/Common.h"

USING_NS_CC;

NS_BEGIN

class FingerTap : public Node
{
public:
    static FingerTap* create();
    
    bool init() override;
    
	FingerTap();
    ~FingerTap();

	void onEnter() override;

	void show(cocos2d::Point const& position, float delay = 0.f);
	void hide();
	bool isShown() const { return m_isShown; }

private:
	void layoutElements();

	void showInternal();

	void moveTo(cocos2d::Point const& position);
	void stopMovingAnimation();

	void startTappingAnimation();
	void stopTappingAnimation();

	bool m_isShown;
	Sprite* m_mainSp;
};


NS_END


#endif // __BUY_BUTTON_H__

