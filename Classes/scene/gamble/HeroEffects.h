#ifndef __HERO_EFFECTS_H__
#define __HERO_EFFECTS_H__

#include "common/Common.h"

USING_NS_CC;

NS_BEGIN

enum EffectType
{
	EFFECT_SHIELDED				= 1,
	EFFECT_LEVELUP				= 2,
	EFFECT_DROP_COINS			= 3,
	EFFECT_HEALING				= 4,
};

enum EffectPlacement
{
	EFFECT_FRONT		= 0,
	EFFECT_BACK			= 1,
};

struct EffectConfig
{
	EffectType type;
	bool hasBackEffect;
	int32 numberOfFrames;	// Number of animation frames
	// Whether or not the animation will remain visible after it completes playing. This value is ignored if the animation is played in reverse
	bool keepVisable;
};

class HeroEffects : public Ref
{
	typedef Map<int32 /* EffectType */, Animation*> AnimationSet;

public:
	HeroEffects();
	~HeroEffects();

	static HeroEffects* create();
	bool init();

	Node* getFrontEffectNode() { return m_frontRendererContainer; }
	Node* getBackEffectNode() { return m_backRendererContainer; }

	void updatePosition(cocos2d::Point const& position, cocos2d::Size const& effectedContentSize);
	void setGlobalZOrder(float globalZOrder);
	void setOpacity(GLubyte opacity);

	void play(EffectType type, bool skipOpening = false, bool reversed = false, float delay = 0.f);
	void stop(EffectType type);
	void stopAll();

private:
	void initEffectRenderers();
	void initEffectAnimations();
	Animation* createAnimation(EffectType type, EffectPlacement placement, int32 frames);

	void runAnimation(Animation* animation, EffectType type, EffectPlacement placement, bool skipOpening, bool reversed, float delay);
	void stopAnimation(EffectType type, EffectPlacement placement);
	Animation* getAnimation(AnimationSet const& animations, EffectType type);

	Node* getRendererContainer(EffectPlacement placement);

	Node* m_frontRendererContainer;
	Node* m_backRendererContainer;

	AnimationSet m_frontAnimations;
	AnimationSet m_backAnimations;
};

NS_END


#endif // __HERO_EFFECTS_H__
