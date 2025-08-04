#include "Splatter.h"

#include "common/debugging/DebugDrawer.h"
#include "game/GameConfig.h"


#define NUMBER_OF_INIT					7		// Initial number of bieces.
#define MOVE_SPEED						40		// The speed at which pieces move. Unit: points/s

#define OPACITY_VISIBLE					255
#define OPACITY_INVISIBLE				0

NS_BEGIN

Splatter::Splatter():
	m_isRunning(false),
	m_remainingPieceCount(0),
	m_numberOfFrames(0),
	m_defaultFrameIndex(0),
	m_animation(nullptr)
{
}

Splatter::~Splatter()
{
	CC_SAFE_RELEASE_NULL(m_animation);
}

Splatter* Splatter::create(std::string frameNameFormat, int32 numberOfFrames, int32 defaultFrameIndex)
{
	Splatter* pRet = new (std::nothrow) Splatter();
	if (pRet && pRet->init(frameNameFormat, numberOfFrames, defaultFrameIndex))
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

bool Splatter::init(std::string frameNameFormat, int32 numberOfFrames, int32 defaultFrameIndex)
{
	bool ret = Node::init();
	if (ret)
	{
		m_frameNameFormat = frameNameFormat;
		m_numberOfFrames = numberOfFrames;
		m_defaultFrameIndex = defaultFrameIndex;

		this->setCascadeOpacityEnabled(true);

		this->initAnimation();
		this->initBatchOfBieces();

		this->scheduleUpdate();

		return true;
	}

	return false;
}

void Splatter::initAnimation()
{
	m_animation = Animation::create();
	CC_SAFE_RETAIN(m_animation);
	if (m_numberOfFrames > 1)
	{
		for (int i = 0; i < m_numberOfFrames; ++i)
		{
			std::string frameName = StringUtils::format(m_frameNameFormat.c_str(), i);
			m_animation->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName(frameName));
		}
		m_animation->setDelayPerUnit(ANIM_NORMAL_FRAME_DELAY);
		m_animation->setLoops(UINT_MAX);
	}
	else
		m_animation->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName(m_frameNameFormat));
}


void Splatter::initBatchOfBieces()
{
	for (int32 i = 0; i < NUMBER_OF_INIT; i++)
	{
		AnimationFrame* frame = m_animation->getFrames().at(m_defaultFrameIndex);
		SILSprite* piece = SILSprite::createWithSpriteFrame(frame->getSpriteFrame());
		//piece->setOpacity(OPACITY_INVISIBLE);
		piece->setVisible(false);
		this->addChild(piece);
	}

}


void Splatter::run(PieceConfig const& config, std::function<void()> const& complete)
{
	NS_ASSERT(!m_isRunning);

	int32 nPieces = config.minNumber + rand() % (config.maxNumber - config.minNumber + 1);
	Vector<Node*>& pieces = this->getChildren();
	NS_ASSERT(nPieces <= pieces.size());

	m_isRunning = true;
	m_remainingPieceCount = nPieces;
	m_pieceContentSize = config.contentSize;

	for (int32 i = 0; i < pieces.size() && i < nPieces; ++i)
	{
		Sprite* piece = dynamic_cast<Sprite*>(pieces.at(i));
		NS_ASSERT(piece != nullptr);

		//piece->setOpacity(OPACITY_VISIBLE);
        piece->setPosition(Point::ZERO);
		piece->setVisible(true);
		piece->setGlobalZOrder(this->getGlobalZOrder());
		piece->setContentSize(config.contentSize);

		// Random scale
		float scale = config.minScale + CCRANDOM_0_1() * (config.maxScale - config.minScale);
		piece->setScale(scale);

		// Random jump position
		float rad = rand() / (float)RAND_MAX * (float)(M_PI * 2);
		float range = config.minJumpRange + rand() % (config.maxJumpRange - config.minJumpRange + 1);
		float dx = std::cos(rad) * range;
		float dy = std::sin(rad) * range;
		Point dest((int32)dx, (int32)dy);
		// Random jump height
		float height = config.minJumpHeight + rand() % (config.maxJumpHeight - config.minJumpHeight + 1);
		// Calculate piece move time
		float time = (range + height) / MOVE_SPEED;

		//CCLOG("PIECE rad:%f range:%f dest:[%f, %f] height:%f time:%f number:%d", rad, range, dest.x, dest.y, height, time, nPieces);

		CallFunc* callfunc = CallFunc::create([&, complete] {
			m_remainingPieceCount--;
			if (m_remainingPieceCount <= 0)
			{
				m_isRunning = false;
				complete();
			}
		});

		JumpTo* jumTo = JumpTo::create(time, dest, height, 1);
		EaseCubicActionOut* ease = EaseCubicActionOut::create(jumTo);
		Hide* hide = Hide::create();
		Sequence* seq = Sequence::create(ease, hide, callfunc, nullptr);
		piece->runAction(seq);

		if (m_animation->getFrames().size() > 1)
		{
			float delay = MIN(ANIM_NORMAL_FRAME_DELAY, time / m_numberOfFrames);
			m_animation->setDelayPerUnit(delay);
			Animate* animate = Animate::create(m_animation);
			if (rand() % 2 == 0)
				piece->runAction(animate->reverse());
			else
				piece->runAction(animate);
		}
	}
}

void Splatter::stop()
{
	if (!m_isRunning)
		return;

	Vector<Node*>& pieces = this->getChildren();
	for (int32 i = 0; i < pieces.size(); ++i)
	{
		Sprite* piece = dynamic_cast<Sprite*>(pieces.at(i));
		NS_ASSERT(piece != nullptr);
		piece->stopAllActions();
	}

	m_remainingPieceCount = 0;
	m_pieceContentSize = Size::ZERO;
	m_isRunning = false;
}

void Splatter::update(float delta)
{
	if (!m_isRunning)
		return;

	Vector<Node*>& pieces = this->getChildren();
	for (int32 i = 0; i < pieces.size(); ++i)
	{
		Sprite* piece = dynamic_cast<Sprite*>(pieces.at(i));
		NS_ASSERT(piece != nullptr);
		piece->setContentSize(m_pieceContentSize);
	}
}

NS_END
