//
//  GiftBox.h
//  snowfight
//
//  Created by Luthier on 2021/10/22.
//

#ifndef __GIFT_BOX_H__
#define __GIFT_BOX_H__

#include "common/Common.h"
#include "InfoBubble.h"

USING_NS_CC;

NS_BEGIN

typedef std::function<void(Ref*)> GiftBoxClickCallback;

class GiftBox : public Node
{
public:
	enum GiftState
	{
		GIFT_STATE_NONE,
		GIFT_STATE_READY,
		GIFT_STATE_CLAIMED,
	};

    static GiftBox* create();
    
    bool init() override;
    
	GiftBox();
    ~GiftBox();

	void onEnter() override;

	void showTips(float delay = 0.f);
	void hideTips();

	void setGiftState(GiftState state);

	void setClickEventListener(GiftBoxClickCallback const& callback) { m_clickCallback = callback; }

	virtual bool onTouchBegan(Touch* touch, Event* event);
	virtual void onTouchMoved(Touch* touch, Event* event);
	virtual void onTouchEnded(Touch* touch, Event* event);
	virtual void onTouchCancelled(Touch* touch, Event* event);

private:
	void initAnimation();
	void playAnimation();
	void stopAnimation();

	void layoutElements();

	bool hitTest(Vec2 const& pt, Camera const* camera, Vec3* p) const;

	void changeToNormalState();
	void changeToPressedState();

	EventListenerTouchOneByOne* m_touchListener;
	bool m_hitted;
	Camera const* m_hittedByCamera;

	GiftState m_state;
	bool m_isTipsShown;
	Animation* m_animation;

	Sprite* m_mainSp;
	InfoBubble* m_infoBubble;

	GiftBoxClickCallback m_clickCallback;
};


NS_END


#endif // __GIFT_BOX_H__

