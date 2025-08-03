#ifndef __PARTICLE_EFFECTS_H__
#define __PARTICLE_EFFECTS_H__

#include "common/Common.h"

NS_BEGIN

USING_NS_CC;

class SnowEffect: public Node
{
public:
	enum
	{
		DURATION_INFINITY = -1
	};
	static SnowEffect* create();
	bool init() override;

	void start(float duration, int32 snowflakes, const Vec2& gravity, float speed, float life, float lifeVar, float startSize, float startSizeVar);
	void stop();
	bool isStopped() const;

	void update(float dt) override;

	void onEnter() override;
	
private:
	SnowEffect();
	~SnowEffect();

	ParticleSystemQuad* createParticleSnow(SpriteFrame* spriteFrame, int32 particles, const Vec2& gravity, float speed, float life, float lifeVar, float startSize, float startSizeVar);

	Node* m_batchNode;
	float m_duration;
	float m_abatingStartTime;
	float m_elapsed;
	float m_gravityUpdateElapsed;
};



NS_END

#endif // __PARTICLE_EFFECTS_H__
