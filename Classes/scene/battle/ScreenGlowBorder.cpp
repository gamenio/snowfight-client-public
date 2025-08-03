#include "ScreenGlowBorder.h"

#include "scene/Utils.h"
#include "common/Machine.h"

NS_BEGIN

#define FRAMENAME_GLOWBORDER_CORNER_ROUNDED			"glowborder_corner_rounded.png"
#define FRAMENAME_GLOWBORDER_CORNER_STRAIGHT		"glowborder_corner_straight.png"
#define FRAMENAME_GLOWBORDER_EDGE					"glowborder_edge.png"

using namespace cocos2d::ui;

ScreenGlowBorder::ScreenGlowBorder()
{
}

ScreenGlowBorder::~ScreenGlowBorder()
{
}

ScreenGlowBorder* ScreenGlowBorder::create()
{
	ScreenGlowBorder* pRet = new ScreenGlowBorder();
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

bool ScreenGlowBorder::init()
{
	if (!Node::init())
		return false;

	auto visibleSize = Director::getInstance()->getVisibleSize();
	this->setContentSize(visibleSize);

	this->initElements();

	return true;
}

void ScreenGlowBorder::initElements()
{
	std::string cornerFrameName;
	Rect safeArea = Machine::instance()->getSafeAreaRect();
	if (safeArea.origin.x > 0 || safeArea.origin.y > 0)
		cornerFrameName = FRAMENAME_GLOWBORDER_CORNER_ROUNDED;
	else
		cornerFrameName = FRAMENAME_GLOWBORDER_CORNER_STRAIGHT;
	Sprite* topLeftCorner = Sprite::createWithSpriteFrameName(cornerFrameName);
	topLeftCorner->setPosition(topLeftCorner->getContentSize().width / 2, this->getContentSize().height - topLeftCorner->getContentSize().height / 2);
	this->addChild(topLeftCorner);

	Sprite* topRightCorner = Sprite::createWithSpriteFrameName(cornerFrameName);
	topRightCorner->setRotation(90);
	topRightCorner->setPosition(this->getContentSize().width - topRightCorner->getContentSize().width / 2, this->getContentSize().height - topRightCorner->getContentSize().height / 2);
	this->addChild(topRightCorner);

	Sprite* bottomLeftCorner = Sprite::createWithSpriteFrameName(cornerFrameName);
	bottomLeftCorner->setRotation(-90);
	bottomLeftCorner->setPosition(bottomLeftCorner->getContentSize().width / 2, bottomLeftCorner->getContentSize().height / 2);
	this->addChild(bottomLeftCorner);

	Sprite* bottomRightCorner = Sprite::createWithSpriteFrameName(cornerFrameName);
	bottomRightCorner->setRotation(180);
	bottomRightCorner->setPosition(this->getContentSize().width - bottomRightCorner->getContentSize().width / 2, bottomRightCorner->getContentSize().height / 2);
	this->addChild(bottomRightCorner);

	Scale9Sprite* topEdge = Scale9Sprite::createWithSpriteFrameName(FRAMENAME_GLOWBORDER_EDGE);
	topEdge->setRotation(90);
	topEdge->setPosition(this->getContentSize().width / 2, this->getContentSize().height - topEdge->getContentSize().height / 2);
	topEdge->setContentSize(Size(topEdge->getContentSize().width, this->getContentSize().width - topLeftCorner->getContentSize().width - topRightCorner->getContentSize().width));
	this->addChild(topEdge);

	Scale9Sprite* bottomEdge = Scale9Sprite::createWithSpriteFrameName(FRAMENAME_GLOWBORDER_EDGE);
	bottomEdge->setRotation(-90);
	bottomEdge->setPosition(this->getContentSize().width / 2, bottomEdge->getContentSize().height / 2);
	bottomEdge->setContentSize(Size(bottomEdge->getContentSize().width, this->getContentSize().width - bottomLeftCorner->getContentSize().width - bottomRightCorner->getContentSize().width));
	this->addChild(bottomEdge);

	Scale9Sprite* leftEdge = Scale9Sprite::createWithSpriteFrameName(FRAMENAME_GLOWBORDER_EDGE);
	leftEdge->setPosition(leftEdge->getContentSize().width / 2, this->getContentSize().height / 2);
	leftEdge->setContentSize(Size(leftEdge->getContentSize().width, this->getContentSize().height - topLeftCorner->getContentSize().height - bottomLeftCorner->getContentSize().height));
	this->addChild(leftEdge);

	Scale9Sprite* rightEdge = Scale9Sprite::createWithSpriteFrameName(FRAMENAME_GLOWBORDER_EDGE);
	rightEdge->setRotation(180);
	rightEdge->setPosition(this->getContentSize().width - rightEdge->getContentSize().width / 2, this->getContentSize().height / 2);
	rightEdge->setContentSize(Size(rightEdge->getContentSize().width, this->getContentSize().height - topRightCorner->getContentSize().height - bottomRightCorner->getContentSize().height));
	this->addChild(rightEdge);
}

NS_END
