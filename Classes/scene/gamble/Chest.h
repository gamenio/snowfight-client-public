#ifndef __CHEST_H__
#define __CHEST_H__

#include "common/Common.h"
#include "game/entities/DataItemBox.h"
#include "scene/gui/silhouette/SILSprite.h"
#include "GameObject.h"
#include "HealthBar.h"

USING_NS_CC;

NS_BEGIN


class Chest : public GameObject
{
public:
	Chest();
	virtual ~Chest();

	static Chest* createWithData(DataItemBox* data);
	virtual bool initWithData(DataItemBox* data);

	void onActivated() override;
	void onInactivated() override;

	void update(float delta) override;

	void setGlobalZOrder(float globalZOrder) override;
	DataItemBox* getData() const override { return m_data; }
    
private:
	void debugDraw();

	void initAnimations();
	Animation* createAnimation(uint8 dir, int32 numOfFrames, float delayPerUnit, bool repeated);
	void createOpenAnimation();

	void startOpenAnimation();
	void runAnimation(Animation* animation);
	void stopAnimation();

	void setAnimationFrame(int32 frameIndex);
	void animateLocked();
	void animateOpen(bool skipOpening);

    void updatePosition();
	void updateHealth(bool force);
	void updateActionAnimation();

	void updateTutorialEvents(bool force);

	DataItemBox* m_data;

	SILLabel* m_nameLabel;
	HealthBar* m_healthBar;
	SILSprite* m_mainSp;

	Map<uint8 /* Direction */, cocos2d::Animation*> m_animationSet;
};


NS_END


#endif // __CHEST_H__