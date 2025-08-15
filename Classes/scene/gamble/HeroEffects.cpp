#include "HeroEffects.h"

#include "game/GameConfig.h"
#include "scene/gui/silhouette/SILSprite.h"

NS_BEGIN

// Frame naming format: role_effect<EffectType><EffectPlacement><FrameIndex>.png
#define EFFECT_FRAME_FROMAT		"role_effect%d%d%02d.png"

#define ACTCION_TAG_EFFECT			1

static std::vector<EffectConfig> sEffectConfigs = {
	{ EFFECT_SHIELDED,		true,		5,		true	},
	{ EFFECT_LEVELUP,		true,		9,		false	},
	{ EFFECT_DROP_COINS,	false,		6,		false	},
	{ EFFECT_HEALING,		false,		23,		false	},
};

bool findEffectConfig(EffectType type, EffectConfig& result)
{
	auto it = std::find_if(sEffectConfigs.begin(), sEffectConfigs.end(), [type](EffectConfig const& config) {
		return config.type == type;
	});
	if (it != std::end(sEffectConfigs))
	{
		result = *it;
		return true;
	}

	return false;
}

HeroEffects::HeroEffects() :
	m_frontRendererContainer(nullptr),
	m_backRendererContainer(nullptr)
{
}


HeroEffects::~HeroEffects()
{
	CC_SAFE_RELEASE_NULL(m_frontRendererContainer);
	CC_SAFE_RELEASE_NULL(m_backRendererContainer);
}

HeroEffects* HeroEffects::create()
{
	HeroEffects *pRet = new HeroEffects();
	if (pRet && pRet->init())
	{
		pRet->autorelease();
		return pRet;
	}
	else
	{
		CC_SAFE_DELETE(pRet);
		return nullptr;
	}
}

bool HeroEffects::init()
{
	m_frontRendererContainer = Node::create();
	m_frontRendererContainer->setCascadeOpacityEnabled(true);
	CC_SAFE_RETAIN(m_frontRendererContainer);

	m_backRendererContainer = Node::create();
	m_backRendererContainer->setCascadeOpacityEnabled(true);
	CC_SAFE_RETAIN(m_backRendererContainer);

	this->initEffectRenderers();
	this->initEffectAnimations();

	return true;
}

void HeroEffects::initEffectRenderers()
{
	for (auto it = sEffectConfigs.begin(); it != sEffectConfigs.end(); ++it)
	{
		EffectConfig const& config = *it;

		SILSprite* renderer = SILSprite::create();
		renderer->setTag(config.type);
		renderer->setVisible(false);
		m_frontRendererContainer->addChild(renderer);

		if (config.hasBackEffect)
		{
			renderer = SILSprite::create();
			renderer->setTag(config.type);
			renderer->setVisible(false);
			m_backRendererContainer->addChild(renderer);
		}
	}
}

void HeroEffects::updatePosition(Point const& position, Size const& effectedContentSize)
{
	for (auto it = sEffectConfigs.begin(); it != sEffectConfigs.end(); ++it)
	{
		EffectConfig const& config = *it;

		Sprite* frontRenderer = dynamic_cast<Sprite*>(m_frontRendererContainer->getChildByTag(config.type));
		switch (config.type)
		{
		case EFFECT_SHIELDED:
			frontRenderer->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
			frontRenderer->setPosition(position.x + effectedContentSize.width / 2, position.y);
			break;
		case EFFECT_LEVELUP:
			frontRenderer->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
			frontRenderer->setPosition(position.x + effectedContentSize.width / 2, position.y - 2);
			break;
		case EFFECT_DROP_COINS:
			frontRenderer->setAnchorPoint(Point::ANCHOR_MIDDLE);
			frontRenderer->setPosition(position.x + effectedContentSize.width / 2, position.y + effectedContentSize.height / 2);
			break;
		case EFFECT_HEALING:
			frontRenderer->setAnchorPoint(Point::ANCHOR_MIDDLE_BOTTOM);
			frontRenderer->setPosition(position.x + effectedContentSize.width / 2 + 10, position.y + effectedContentSize.height - 29);
			break;
		default:
			break;
		}

		if (config.hasBackEffect)
		{
			Sprite* backRenderer = dynamic_cast<Sprite*>(m_backRendererContainer->getChildByTag(config.type));
			if (backRenderer)
			{
				backRenderer->setAnchorPoint(frontRenderer->getAnchorPoint());
				backRenderer->setPosition(frontRenderer->getPosition());
			}
		}
	}
}

void HeroEffects::setGlobalZOrder(float globalZOrder)
{
	auto const& frontRenderers =  m_frontRendererContainer->getChildren();
	for (auto it = frontRenderers.begin(); it != frontRenderers.end(); ++it)
	{
		(*it)->setGlobalZOrder(globalZOrder);
	}

	auto const& backRenderers = m_backRendererContainer->getChildren();
	for (auto it = backRenderers.begin(); it != backRenderers.end(); ++it)
	{
		(*it)->setGlobalZOrder(globalZOrder);
	}
}

void HeroEffects::setOpacity(GLubyte opacity)
{
	m_frontRendererContainer->setOpacity(opacity);
	m_backRendererContainer->setOpacity(opacity);
}

void HeroEffects::play(EffectType type, bool skipOpening, bool reversed, float delay)
{
	if (Animation* anim = this->getAnimation(m_frontAnimations, type))
		this->runAnimation(anim, type, EFFECT_FRONT, skipOpening, reversed, delay);

	if (Animation* anim = this->getAnimation(m_backAnimations, type))
		this->runAnimation(anim, type, EFFECT_BACK, skipOpening, reversed, delay);
}

void HeroEffects::stop(EffectType type)
{
	this->stopAnimation(type, EFFECT_FRONT);
	this->stopAnimation(type, EFFECT_BACK);
}

void HeroEffects::stopAll()
{
	for (auto it = sEffectConfigs.begin(); it != sEffectConfigs.end(); ++it)
	{
		EffectConfig const& config = *it;
		this->stop(config.type);
	}
}

void HeroEffects::initEffectAnimations()
{
	for (auto it = sEffectConfigs.begin(); it != sEffectConfigs.end(); ++it)
	{
		EffectConfig const& config = *it;
		Animation* animation = this->createAnimation(config.type, EFFECT_FRONT, config.numberOfFrames);
		m_frontAnimations.insert(config.type, animation);
		if (config.hasBackEffect)
		{
			animation = this->createAnimation(config.type, EFFECT_BACK, config.numberOfFrames);
			m_backAnimations.insert(config.type, animation);
		}
	}
}

Animation* HeroEffects::createAnimation(EffectType type, EffectPlacement placement, int32 frames)
{
	Animation* animation = Animation::create();
	for (int32 i = 0; i < frames; ++i)
	{
		std::string frameName = StringUtils::format(EFFECT_FRAME_FROMAT, (int32)type, (int32)placement, i);
		SpriteFrame* frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(frameName);
		NS_ASSERT(frame != nullptr);
		animation->addSpriteFrame(frame);
	}
	animation->setDelayPerUnit(ANIM_NORMAL_FRAME_DELAY);

	return animation;
}

void HeroEffects::stopAnimation(EffectType type, EffectPlacement placement)
{
	Node* rendererContainer = this->getRendererContainer(placement);
	Node* target = rendererContainer->getChildByTag(type);
	if (!target)
		return;

	if (Action* action = target->getActionByTag(ACTCION_TAG_EFFECT))
		target->stopAction(action);

	target->setVisible(false);
}

Animation* HeroEffects::getAnimation(AnimationSet const& animations, EffectType type)
{
	auto it = animations.find(type);
	if (it != animations.end())
		return (*it).second;

	return nullptr;
}

Node* HeroEffects::getRendererContainer(EffectPlacement placement)
{
	Node* rendererContainer = nullptr;
	switch (placement)
	{
	case EffectPlacement::EFFECT_FRONT:
		rendererContainer = m_frontRendererContainer;
		break;
	default:
		rendererContainer = m_backRendererContainer;
		break;
	}

	return rendererContainer;
}

void HeroEffects::runAnimation(Animation* animation, EffectType type, EffectPlacement placement, bool skipOpening, bool reversed, float delay)
{
	Node* rendererContainer = this->getRendererContainer(placement);
	Sprite* target =  dynamic_cast<Sprite*>(rendererContainer->getChildByTag(type));
	if (!target)
		return;

	Action* action = target->getActionByTag(ACTCION_TAG_EFFECT);
	if (action)
	{
		target->stopAction(action);
		action = nullptr;
	}

	Vector<AnimationFrame*> const& frames = animation->getFrames();
	if (frames.empty())
		return;

	EffectConfig config;
	if (!findEffectConfig(type, config))
		return;

	if (skipOpening)
	{
		target->setSpriteFrame(frames.at(frames.size() - 1)->getSpriteFrame());
		target->setVisible(true);
	}
	else
	{
		Animate* animate = Animate::create(animation);
		bool keepVisable = false;
		if (reversed)
		{
			target->setSpriteFrame(frames.at(frames.size() - 1)->getSpriteFrame());
			animate = animate->reverse();
		}
		else
		{
			target->setSpriteFrame(frames.at(0)->getSpriteFrame());
			keepVisable = config.keepVisable;
		}

		Vector<FiniteTimeAction*> arrayOfActions;
		if (delay > 0.f)
		{
			arrayOfActions.pushBack(DelayTime::create(delay));
			arrayOfActions.pushBack(Show::create());
		}
		else
			target->setVisible(true);

		arrayOfActions.pushBack(animate);

		if (!keepVisable)
			arrayOfActions.pushBack(Hide::create());

		action = Sequence::create(arrayOfActions);
		action->setTag(ACTCION_TAG_EFFECT);
		target->runAction(action);
	}

}

NS_END