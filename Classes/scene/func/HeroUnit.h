#ifndef __HERO_UNIT_H__
#define __HERO_UNIT_H__

#include "common/Common.h"
#include "game/ShopMgr.h"
#include "scene/gui/CatwalkUnit.h"

USING_NS_CC;

NS_BEGIN

class HeroUnit : public CatwalkUnit
{
public:
	HeroUnit();
	~HeroUnit();

	static HeroUnit* create(HeroTemplate const* tmpl);
	bool init(HeroTemplate const* tmpl);

	void activate();
	void inactivate();

	HeroTemplate const* getHeroTemplate() const { return m_heroTemplate; }
    void updateLockState();
    
private:
	bool hitTest(cocos2d::Point const& p);

	bool onTouchBegan(Touch* touch, Event* event);
	void onTouchEnded(Touch* touch, Event* event);
	void onTouchCancelled(Touch* touch, Event* event);
	void onTouchMoved(Touch* touch, Event* event);

	void setupAnimation();
	void startAnimation();
	void stopAnimation();
	void setDefaultFrame();

	bool m_activated;
	HeroTemplate const* m_heroTemplate;
	Sprite* m_lockSp;
	GLProgramState* m_lockDefaultGLProgramState;
	bool m_locked;
	Animation* m_animation;
};


NS_END

#endif // __HERO_UNIT_H__
