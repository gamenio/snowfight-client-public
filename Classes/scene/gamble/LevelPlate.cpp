#include "LevelPlate.h"

#include "game/LocaleMgr.h"
#include "scene/Utils.h"

NS_BEGIN

#define FRAMENAME_LEVEL_BG				"level_bg.png"

LevelPlate* LevelPlate::create()
{
	LevelPlate* pRet = new LevelPlate();
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

bool LevelPlate::init()
{
	if (!Node::init())
		return false;

	SpriteFrame* bgFrame = SpriteFrameCache::getInstance()->getSpriteFrameByName(FRAMENAME_LEVEL_BG);
	this->setContentSize(bgFrame->getOriginalSize());

	m_bgSp = SILSprite::createWithSpriteFrame(bgFrame);
	m_bgSp->setPosition(this->getContentSize().width / 2, this->getContentSize().height / 2);
	this->addChild(m_bgSp);

	m_levelLabel = SILLabel::createWithSystemFont("0", DEFAULT_SYSTEM_FONT, 9);
	m_levelLabel->setPosition(this->getContentSize().width / 2, this->getContentSize().height /  2 + 1);
	m_levelLabel->setTextColor(Color4B(238, 237, 244, 255));
	Utils::enableBoldForLabel(m_levelLabel);
	this->addChild(m_levelLabel);

	return true;
}

LevelPlate::LevelPlate() :
	m_bgSp(nullptr),
	m_levelLabel(nullptr)
{
}

LevelPlate::~LevelPlate()
{
	m_bgSp = nullptr;
	m_levelLabel = nullptr;
}

void LevelPlate::setLevel(uint8 level)
{
	m_levelLabel->setString(std::to_string((int32)level));
}

void LevelPlate::setGlobalZOrder(float globalZOrder)
{
	m_bgSp->setGlobalZOrder(globalZOrder);
	m_levelLabel->setGlobalZOrder(globalZOrder);

	Node::setGlobalZOrder(globalZOrder);
}


NS_END
