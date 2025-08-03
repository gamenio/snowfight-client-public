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

	// Bug：使用ParticleBatchNode后，当粒子系统到达持续时间后粒子没有从屏幕上消失
	//m_batchNode = ParticleBatchNode::create(PARTICLE_ATLAS);
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

	// 发射粒子的持续时间
	particle->setDuration(ParticleSystem::DURATION_INFINITY);

	// 粒子总数
	particle->setTotalParticles(particles);

	// 粒子大小的变化率
	Size size = spriteFrame->getOriginalSize();
	particle->setStartSize(startSize);
	if (startSizeVar)
	{
		particle->setStartSizeVar(startSizeVar);
	}
	particle->setEndSize(ParticleSystem::START_SIZE_EQUAL_TO_END_SIZE);

	// 初始速度   
	particle->setSpeed(speed);

	// 下降速度
	particle->setGravity(gravity);

	// 角度的变化率
	particle->setAngle(-90);

	// 粒子自旋转速度的变化率  
	particle->setStartSpin(0);

	particle->setEndSpin(0);
	particle->setEndSpinVar(90);

	// 粒子生命的变化率
	particle->setLife(life);
	if (lifeVar != 0)
	{
		particle->setLifeVar(lifeVar);
	}

	// 粒子的放射速度
	particle->setEmissionRate(particle->getTotalParticles() / particle->getLife());

	// 位置的变化率
	Size winSize = Director::getInstance()->getWinSize();
	particle->setPosition(winSize.width / 2, winSize.height + 10);
	particle->setPosVar(Vec2(winSize.width / 2, 0));

	// 粒子颜色
	particle->setStartColor(Color4F(1.0f, 1.0f, 1.0f, 1.0f));
	particle->setEndColor(Color4F(1.0f, 1.0f, 1.0f, 1.0f));


	particle->setBlendAdditive(false);

	return particle;
}

NS_END