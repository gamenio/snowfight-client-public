#ifndef __HERO_H__
#define __HERO_H__

#include "common/Common.h"
#include "game/movement/MovementGenerator.h"
#include "game/entities/DataUnit.h"
#include "scene/gamble/Bullet.h"
#include "scene/gui/silhouette/SILSprite.h"
#include "Role.h"
#include "Nameplate.h"
#include "HeroEffects.h"
#include "ComponentTransformCache.h"
#include "MagicBean.h"

USING_NS_CC;


NS_BEGIN

// 英雄音效配置
#define SOUNDID_MYHERO                  1
#define SOUNDID_ANTIHERO                2
#define SOUND_VOLUME_MYHERO             1.0f
#define SOUND_VOLUME_ANTIHERO           0.2f

// 隐蔽和露出时的透明度
#define CONCEALING_OPACITY					153
#define EXPOSED_OPACITY						255

class BattleScene;

class Hero : public Role
{
	enum ComponentType
	{
		COMPONENT_BODY			= 0,
		COMPONENT_ARM			= 1,
		COMPONENT_SNOWBALL		= 2,
	};

	enum BasicMovement
	{
		BASIC_MOVEMENT_WALK		= 0,
		BASIC_MOVEMENT_STAND	= 1,
		BASIC_MOVEMENT_DEAD		= 2,
		BASIC_MOVEMENT_DAMAGED	= 3,
	};

	enum SnowballStyle
	{
		SNOWBALL_STYLE_NONE		= 0,
		SNOWBALL_STYLE_SQUARE	= 1,
	};

	static const int32 COMPONENT_MOVEMENT_NONE = 0;

	// Arm & Snowball component movement
	enum ArmMovement
	{
		ARM_MOVEMENT_BASIC			= 1,
		ARM_MOVEMENT_HANDUP			= 2,
		ARM_MOVEMENT_HURL			= 3,
	};

public:
	Hero();
	virtual ~Hero();

	static Hero* createWithData(DataUnit* data);
	virtual bool initWithData(DataUnit* data) override;

	virtual void onActivated() override;
	virtual void onInactivated() override;

	virtual void hurl() override { }
	virtual void update(float delta) override;
	virtual void onEnter() override;

	virtual void onItemApplicationUpdate(bool apply, uint32 itemId, int32 duration, int32 remainingTime);

private:
	void startWalkAnimation(ComponentType componentType, uint32 style, int32 componentMovement);
	void startDeadAnimation();
	void runAnimation(ComponentType componentType, Animation* animation);
	void stopAllAnimations();
	void stopAnimation(ComponentType componentType);

	Sprite* getComponent(ComponentType componentType);
	void updateComponentSizeAndPosition(ComponentType componentType, std::string const& frameName);
	uint64 generateAnimationId(ComponentType componentType, uint32 style, BasicMovement basicMovement, int32 componentMovement, uint8 dir);
	void setAnimationFrame(ComponentType componentType, uint32 style, BasicMovement basicMovement, int32 componentMovement, int32 frameIndex = 0);

	void initAnimations();
	Animation* createAnimation(ComponentType componentType, uint32 style, BasicMovement basicMovement, int32 componentMovement, uint8 dir, int32 numOfFrames, float delayPerUnit, bool repeated, bool listenFrameDisplayedEvent);
	void createWalkAnimation(ComponentType componentType, uint32 style, int32 componentMovement = COMPONENT_MOVEMENT_NONE, bool listenFrameDisplayedEvent = false);
	void createDeadAnimation();

	float calcWalkFrameDelay(int32 moveSpeed);
	void adjustWalkAnimFrameDelay();

	void updatePosition(bool force);
	void updateMovementAnimation();
	void updateSmileyVisibility(bool force);
	void updateMagicBeanVisibility();
	void updateMagicBeanCount(bool force);

	void conceal();
	void expose();

	void updateFootprints(bool force);

	void onAnimationFrameDisplayed(EventCustom* event);
	void registerAnimationFrameDisplayedListener();
	void unregisterAnimationFrameDisplayedListener();
	void updateComponentWhenSpriteFrameChanged(BasicMovement basicMovement, ComponentType componentType, int32 componentMovement, std::string const& frameName, int32 frameIndex);

	void debugDraw();

protected:
	void animateStand();
	void animateWalk();
	void animateDead(bool skipOpening = false);
	void animateAlive();
	void animateDamaged();
	void animateHurl();
	bool isHurlAnimated() const;

	void animateDelayedStand();
	void animateDelayedWalk();
	void stopAllDelayedMovements();

	virtual void updateGlobalZOrderOnAlive();
	virtual void updateGlobalZOrderOnDead();

	virtual void applyItem(ItemApplicationTemplate const* appTmpl, int32 duration, int32 remainingTime) {}
	virtual void unapplyItem(ItemApplicationTemplate const* appTmpl) {}

	virtual bool addFootprint(cocos2d::Point const& pos, float orient);

protected:
	SILSprite* m_mainSp;
	SILSprite* m_armSp;
	SILSprite* m_snowballSp;
	Nameplate* m_nameplate;
	HeroEffects* m_heroEffects;
	SILSprite* m_smiley;
	MagicBean* m_magicBean;

private:
	EventListenerCustom* m_animationFrameDisplayedListener;
	Map<uint64/* Animation ID */, cocos2d::Animation*> m_animationSet;
	BasicMovement m_currBasicMovement;
	int32 m_currArmMovement;
	bool m_isConcealing;
	bool m_isWalkAnimDirty;

	cocos2d::Point m_currStepPos;
	float m_currStepLength;

};

NS_END


#endif // __HERO_H__
