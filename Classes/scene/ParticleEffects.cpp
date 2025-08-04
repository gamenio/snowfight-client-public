#include "ParticleEffects.h"

#include "AssetsLoader.h"

NS_BEGIN

#define ABATING_DURATION					10.0f

SnowEffect* SnowEffect::create()
{
	SnowEffect* ret = new (std::nothrow) SnowEffect();
	if (ret && ret->init())
	{
		ret->autorelease();
		return ret;
	}
	else
	{
		CC_SAFE_DELETE(ret);
		return nullptr;
	}
}

bool SnowEffect::init()
{
	if (!Node::init())
		return false;

	m_batchNode = Node::create();
	this->addChild(m_batchNode);

	return true;

}

void SnowEffect::start(float duration, int32 snowflakes, const Vec2& gravity, float speed, float life, float lifeVar, float startSize, float startSizeVar)
{
	if(!this->isStopped())
		this->stop();

	SpriteFrame* spriteFrame = SpriteFrameCache::getInstance()->getSpriteFrameByName("snowflake.png");
	ParticleSystemQuad* particle = createParticleSnow(spriteFrame, snowflakes, gravity, speed, life, lifeVar, startSize, startSizeVar);
	m_batchNode->addChild(particle);

	m_duration = duration;
	m_elapsed = 0;
	if (m_duration != DURATION_INFINITY)
		m_abatingStartTime = MAX(0.0f, m_duration - ABATING_DURATION);

	this->scheduleUpdate();
}

void SnowEffect::stop()
{
	if (this->isStopped())
		return;

	this->unscheduleUpdate();

	m_batchNode->removeAllChildren();
}

bool SnowEffect::isStopped() const
{
	return m_batchNode->getChildrenCount() == 0;
}

void SnowEffect::update(float dt)
{
	m_elapsed += dt;

	bool emissionRateUpdate = false;
	float timeRatio = 0.f;
	if (m_duration != DURATION_INFINITY && m_elapsed >= m_abatingStartTime)
	{
		timeRatio = MAX(0.0f, (m_duration - m_elapsed) / (m_duration - m_abatingStartTime));
		emissionRateUpdate = true;
	}

	auto& children = m_batchNode->getChildren();
	for (Node* node : children)
	{
		ParticleSystemQuad* particle = dynamic_cast<ParticleSystemQuad*>(node);
		if (!particle || !particle->isActive())
			continue;


		if (emissionRateUpdate)
		{
			float rate = particle->getTotalParticles() / particle->getLife() * timeRatio;
			//CCLOG("EmissionRate:%f", rate);
			if (rate > 0)
			{
				particle->setEmissionRate(rate);
			}
			else
			{
				particle->stopSystem();
				//CCLOG("stopSystem");
			}
		}
	}

	if (children.empty())
	{
		this->unscheduleUpdate();
	}
}

void SnowEffect::onEnter()
{
	Node::onEnter();


}

SnowEffect::SnowEffect() :
	m_batchNode(nullptr),
	m_duration(DURATION_INFINITY),
	m_elapsed(0),
	m_abatingStartTime(0),
	m_gravityUpdateElapsed(0)
{

}

SnowEffect::~SnowEffect()
{
}


cocos2d::ParticleSystemQuad* SnowEffect::createParticleSnow(SpriteFrame* spriteFrame, int32 particles, const Vec2& gravity, float speed, float life, float lifeVar, float startSize, float startSizeVar)
{
	ParticleSystemQuad* particle = ParticleSystemQuad::create();

	particle->setDisplayFrame(spriteFrame);

	particle->setEmitterMode(ParticleSystem::Mode::GRAVITY);
	particle->setAutoRemoveOnFinish(true);

	// Particle emission duration
	particle->setDuration(ParticleSystem::DURATION_INFINITY);

	// Total particles
	particle->setTotalParticles(particles);

	// Particle size variation
	Size size = spriteFrame->getOriginalSize();
	particle->setStartSize(startSize);
	if (startSizeVar)
	{
		particle->setStartSizeVar(startSizeVar);
	}
	particle->setEndSize(ParticleSystem::START_SIZE_EQUAL_TO_END_SIZE);

	// Initial speed
	particle->setSpeed(speed);

	// Particle gravity
	particle->setGravity(gravity);

	// Particle angle
	particle->setAngle(-90);

	// Particle spin variation
	particle->setStartSpin(0);

	particle->setEndSpin(0);
	particle->setEndSpinVar(90);

	// Particle life variation
	particle->setLife(life);
	if (lifeVar != 0)
	{
		particle->setLifeVar(lifeVar);
	}

	// Emission rate
	particle->setEmissionRate(particle->getTotalParticles() / particle->getLife());

	// Position variation
	Size winSize = Director::getInstance()->getWinSize();
	particle->setPosition(winSize.width / 2, winSize.height + 10);
	particle->setPosVar(Vec2(winSize.width / 2, 0));

	// Particle color variation
	particle->setStartColor(Color4F(1.0f, 1.0f, 1.0f, 1.0f));
	particle->setEndColor(Color4F(1.0f, 1.0f, 1.0f, 1.0f));


	particle->setBlendAdditive(false);

	return particle;
}

NS_END