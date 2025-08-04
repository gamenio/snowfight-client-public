#include "Snowball.h"

#include "common/debugging/DebugDrawer.h"
#include "game/entities/DataPlayer.h"
#include "game/GameConfig.h"
#include "game/World.h"
#include "scene/ObjectPools.h"
#include "scene/SoundMgr.h"
#include "scene/gamble/Hero.h"
#include "GambleDefines.h"

// Debug drawer
#if NS_DEBUG
#define DEBUG_PIECE_JUMP_RANGE						0

#endif // NS_DEBUG

// Snowball rotation animation
#define TAG_ACTION_ROTATE								1000
#define NUMBER_OF_FRAMES_FOR_ROTATION					6
#define ROTATION_FRAME_FORMAT							"snowball%02d.png"
#define ROTATION_DEFAULT_FRAME							0
#define ROTATION_SPEED_SCALE_IN_LAUNCHED				1.5f // rotation speed scale after the snowball is launched

// Snowball sliding animation
#define SLIDING_DISTANCE								26
#define SLIDING_DURATION								0.35f

// Animation frame name
#define FRAMENAME_WATER_PIECE							"water_piece.png"

// Size of snowball, shadow, and splatter piece
#define COMPONENT_SIZE									Size(16, 16)

NS_BEGIN

Snowball* Snowball::createWithData(DataProjectile* data)
{
	Snowball* sprite = new (std::nothrow) Snowball();
	if (sprite && sprite->initWithData(data))
	{
		sprite->autorelease();
		return sprite;
	}
	CC_SAFE_DELETE(sprite);
	return nullptr;
}

Snowball::Snowball() :
	m_main(nullptr),
	m_shadow(nullptr),
	m_isSliding(false),
	m_snowSplatter(nullptr),
	m_waterSplatter(nullptr),
	m_rotationAnim(nullptr)
{

}


Snowball::~Snowball()
{
	this->unscheduleAllCallbacks();
	this->stopAllActions();

	m_main = nullptr;
	m_shadow = nullptr;
	m_snowSplatter = nullptr;
	m_waterSplatter = nullptr;
	CC_SAFE_RELEASE_NULL(m_rotationAnim);

}

bool Snowball::initWithData(DataProjectile* data)
{
	if (!Bullet::initWithData(data))
		return false;

	this->setAnchorPoint(Point::ZERO);

	this->initRotationAnimation();

	m_shadow = SILSprite::createWithSpriteFrameName("snowball_shadow.png");
	m_shadow->setContentSize(COMPONENT_SIZE);
	this->addChild(m_shadow);

	m_main = SILSprite::create();
	AnimationFrame* frame = m_rotationAnim->getFrames().at(ROTATION_DEFAULT_FRAME);
	m_main->setSpriteFrame(frame->getSpriteFrame());
	m_main->setContentSize(COMPONENT_SIZE);
	this->addChild(m_main);

	m_snowSplatter = Splatter::create(ROTATION_FRAME_FORMAT, NUMBER_OF_FRAMES_FOR_ROTATION, ROTATION_DEFAULT_FRAME);
	m_snowSplatter->setVisible(false);
	this->addChild(m_snowSplatter);

	m_waterSplatter = Splatter::create(FRAMENAME_WATER_PIECE);
	m_waterSplatter->setVisible(false);
	this->addChild(m_waterSplatter);

	return true;
}

void Snowball::update(float delta)
{
    Bullet::update(delta);

	if (!this->isActive())
		return;

	m_main->setContentSize(COMPONENT_SIZE);

    if(m_isSliding)
    {
		float offsetZ;
        float z = this->getData()->getMapData()->getTileZForPos(this->getPosition(), &offsetZ);
        z += (OVERLAY_SLIDING_SNOWBALL + offsetZ);
        m_main->setGlobalZOrder(z);
        m_shadow->setGlobalZOrder(z);
    }
}

void Snowball::reset()
{
	this->unscheduleAllCallbacks();
	this->stopAllActions();

	m_isSliding = false;

	m_main->stopAllActions();
	m_main->setVisible(true);
	m_main->setGlobalZOrder(0);
	m_main->setScale(1.0f);
	m_main->setSilhouetted(true);

	m_shadow->setVisible(true);
	m_shadow->setPosition(Point::ZERO);
	m_shadow->setGlobalZOrder(0);
	m_shadow->setScale(1.0f);
	m_shadow->setSilhouetted(true);

	m_snowSplatter->stop();
	m_snowSplatter->setVisible(false);
	m_snowSplatter->setScale(1.0f);
	m_waterSplatter->stop();
	m_waterSplatter->setVisible(false);
	m_waterSplatter->setScale(1.0f);

	Bullet::reset();

}

void Snowball::initRotationAnimation()
{
	m_rotationAnim = Animation::create();
	CC_SAFE_RETAIN(m_rotationAnim);
	for (int i = 0; i < NUMBER_OF_FRAMES_FOR_ROTATION; ++i)
	{
		std::string frameName = StringUtils::format(ROTATION_FRAME_FORMAT, i);
		m_rotationAnim->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName(frameName));
	}
	m_rotationAnim->setLoops(UINT_MAX);
	m_rotationAnim->setDelayPerUnit(ANIM_NORMAL_FRAME_DELAY);
}

void Snowball::startRotationAnimation(float speedScale)
{
	if (Action* action = m_main->getActionByTag(TAG_ACTION_ROTATE))
		m_main->stopAction(action);

	float delayPerUnit = ANIM_NORMAL_FRAME_DELAY + ANIM_NORMAL_FRAME_DELAY * (1.0 - speedScale);
	m_rotationAnim->setDelayPerUnit(delayPerUnit);
	Action* rotateAction = Animate::create(m_rotationAnim);
	rotateAction->setTag(TAG_ACTION_ROTATE);
	m_main->runAction(rotateAction);
}

void Snowball::stopRotationAnimation()
{
	if (Action* action = m_main->getActionByTag(TAG_ACTION_ROTATE))
		m_main->stopAction(action);
}

void Snowball::didLaunch()
{
	// Snowball rotation animation
	this->startRotationAnimation(ROTATION_SPEED_SCALE_IN_LAUNCHED);

	float scale = this->getData()->getScale();
	NS_ASSERT(scale >= 1.0f);
	m_main->setScale(scale);
	m_shadow->setScale(scale);
	m_snowSplatter->setScale(scale);
	m_waterSplatter->setScale(scale);
}

void Snowball::didShadowRelocation(Point const& position, float vertexZ)
{
	//CCLOG("didShadowRelocation: %f %f", position.x, position.y);
	m_main->setGlobalZOrder(vertexZ);
	m_shadow->setGlobalZOrder(vertexZ);
	m_shadow->setPosition(position);

}

void Snowball::didHit(ObjectGuid const& target)
{
    DataPlayer* myChar = World::getInstance()->getMyself();
	if (!myChar)
		return;

	if (target == myChar->getGuid())
		sSoundMgr->play(SOUND_SNOWBALL_HIT, false, SOUND_VOLUME_MYHERO, SOUNDID_MYHERO, 2);
	else
		sSoundMgr->play(SOUND_SNOWBALL_HIT, false, SOUND_VOLUME_ANTIHERO, SOUNDID_ANTIHERO, 2);
    
    
	this->stopRotationAnimation();
	m_main->setVisible(false);
	m_shadow->setVisible(false);

	// Snowflake splashing effect after a snowball hits a person or building.
	PieceConfig config;
	config.maxJumpRange = 10;
	config.minJumpRange = 5;
	config.maxJumpHeight = 15;
	config.minJumpHeight = 10;
	config.minNumber = 3;
	config.maxNumber = 5;
	config.minScale = 0.3f;
	config.maxScale = 0.5f;
	config.contentSize = COMPONENT_SIZE;
	this->runSplatter(m_snowSplatter, config);
}

void Snowball::didFallToGround()
{
	MapData const* mapData = this->getData()->getMapData();
	TileCoord currCoord(mapData->getMapSize(), this->getPosition());

	if (mapData->isWater(currCoord))
	{
		this->stopRotationAnimation();
		m_main->setVisible(false);
		m_shadow->setVisible(false);

		// Water splashing effect when a snowball hits the water surface.
		PieceConfig config;
		config.maxJumpRange = 8;
		config.minJumpRange = 0;
		config.maxJumpHeight = 20;
		config.minJumpHeight = 15;
		config.minNumber = 3;
		config.maxNumber = 5;
		config.minScale = 0.3f;
		config.maxScale = 0.6f;
		config.contentSize = COMPONENT_SIZE;
		this->runSplatter(m_waterSplatter,config);
	}
	// Sliding forward after falling to the ground
	else
	{
		Point launcherPos = this->getData()->getLauncherOrigin();
		float dx = this->getPosition().x - launcherPos.x;
		float dy = this->getPosition().y - launcherPos.y;
		float rad = atan2(dy, dx);
		float ox = std::cos(rad) * TILE_WIDTH_HALF;
		float oy = std::sin(rad) * TILE_WIDTH_HALF;
		TileCoord frontCoord(mapData->getMapSize(), Vec2(this->getPosition().x + ox, this->getPosition().y + oy));
		TileCoord backCoord(mapData->getMapSize(), Vec2(this->getPosition().x - ox, this->getPosition().y - oy));

		// If the terrain is a mixture of land and water, the snowball will not slide; otherwise, the snowball will slide on the water surface.
		if (mapData->isWater(frontCoord) || mapData->isWater(backCoord))
		{
			this->stopRotationAnimation();
			this->reset();
		}
		else
		{
			this->startRotationAnimation(1.0f);

			m_isSliding = true;
			this->runSlide([&]()
			{
				m_isSliding = false;
				this->stopRotationAnimation();
				this->reset();
			});
		}

	}
}

void Snowball::runSplatter(Splatter* splatter, PieceConfig const& config)
{

#if DEBUG_PIECE_JUMP_RANGE

	Brush* brush = sDebugDrawer->getDrawByTag("Snowball.PieceJumpRange");
	brush->clear();
	brush->drawCircle(this->getPosition(), config.maxJumpRange, Color4F::RED);
	if(config.minJumpRange > 0)
		brush->drawCircle(this->getPosition(), config.minJumpRange, Color4F::BLUE);

#endif

	splatter->setGlobalZOrder(m_main->getGlobalZOrder());
	splatter->setVisible(true);
	splatter->run(config, [&]() {
		this->reset();
	});

}

void Snowball::runSlide(std::function<void()> const& complete)
{
	Point launcherPos = this->getData()->getLauncherOrigin();
	float dx = this->getPosition().x - launcherPos.x;
	float dy = this->getPosition().y - launcherPos.y;
	float rad = atan2(dy, dx);
	float ox = std::cos(rad) * SLIDING_DISTANCE;
	float oy = std::sin(rad) * SLIDING_DISTANCE;
	Point dest(ox, oy);

	CallFunc* callfunc = CallFunc::create([&, complete] {
		complete();
	});

	//FadeOut* fadeOut = FadeOut::create(SLIDING_DURATION);
	//m_main->runAction(fadeOut);

	MoveBy* move = MoveBy::create(SLIDING_DURATION, dest);
    EaseCubicActionOut* ease = EaseCubicActionOut::create(move);
	Sequence* seq = Sequence::create(ease, callfunc, nullptr);
	this->runAction(seq);
}

NS_END
