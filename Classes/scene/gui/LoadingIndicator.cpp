#include "LoadingIndicator.h"


NS_BEGIN

#define IMG_LOADING_CIRCLE                RES_IMAGE("loading_circle.png")

#define NUMBER_OF_CIRCLES				3
#define CIRCLE_PADDING					3.0f

#define ACTION_TAG_ANIMATION					1

LoadingIndicator* LoadingIndicator::create(Size const& size, Color3B const& tintColor)
{
	LoadingIndicator* ret = new (std::nothrow) LoadingIndicator();
	if (ret && ret->init(size, tintColor))
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

bool LoadingIndicator::init(Size const& size, Color3B const& tintColor)
{
	if (!Node::init())
		return false;

	this->setContentSize(size);
	this->setAnchorPoint(Point::ANCHOR_MIDDLE);

	float paddings = (NUMBER_OF_CIRCLES - 1) * CIRCLE_PADDING;
	float circleSize = MIN((size.width - paddings) / NUMBER_OF_CIRCLES, size.height);
	circleSize = MIN(circleSize, size.height);
	float x = (size.width - (circleSize * NUMBER_OF_CIRCLES + paddings)) / 2;
	float y = size.height / 2;

	for (int32 i = 0; i < NUMBER_OF_CIRCLES; ++i)
	{
        Sprite* circle = Sprite::create(IMG_LOADING_CIRCLE);
		circle->setColor(tintColor);
		circle->setOpacity(0);
		circle->setScale(0);
		circle->setPosition(x + i * circleSize + circleSize / 2 + i * CIRCLE_PADDING, y);
		circle->setContentSize(Size(circleSize, circleSize));
		this->addChild(circle);
	}

	this->runAnimation();

	return true;
}

LoadingIndicator::LoadingIndicator() :
	m_isAnimating(false)
{

}

LoadingIndicator::~LoadingIndicator()
{

}

void LoadingIndicator::setVisible(bool visible)
{
	if (_visible != visible)
	{
		if (visible)
			this->runAnimation();
		else
			this->stopAnimation();
	}

	Node::setVisible(visible);
}

void LoadingIndicator::setTintColor(Color3B const& tintColor)
{
	auto const& children = this->getChildren();
	for (int32 i = 0; i < children.size(); ++i)
	{
		Node* node = children.at(i);
		node->setColor(tintColor);
	}
}

void LoadingIndicator::stopAnimation()
{
	auto const& children = this->getChildren();
	for (int32 i = 0; i < children.size(); ++i)
	{
		Node* node = children.at(i);
		node->stopActionByTag(ACTION_TAG_ANIMATION);
		node->setScale(0.0f);
		node->setOpacity(0);
	}

	m_isAnimating = false;
}

void LoadingIndicator::runAnimation()
{
	if (m_isAnimating)
		this->stopAnimation();

	float duration = 0.75f;
	std::vector<float> timeBegins = { 0.0f, 0.12f, 0.24f };

	auto const& children = this->getChildren();
	for (int32 i = 0; i < children.size(); ++i)
	{
		Node* node = children.at(i);
		ScaleTo* scaleOut = ScaleTo::create(duration * 0.5f, 0.3f);
		ScaleTo* scaleIn = ScaleTo::create(duration * 0.5f, 1.0f);
		FadeTo* fadeIn = FadeTo::create(duration * 0.5f, 255);
		FadeTo* fadeOut = FadeTo::create(duration * 0.5f, 153);
		Sequence* zoomFadeSeq = Sequence::create(
			EaseSineIn::create(Spawn::createWithTwoActions(scaleOut, fadeOut)),
			EaseSineOut::create(Spawn::createWithTwoActions(scaleIn, fadeIn)),
			nullptr);
		Sequence* seq = Sequence::create(
			DelayTime::create(timeBegins[i]),
			Repeat::create(zoomFadeSeq, pow(2, 30)),
			nullptr);
		seq->setTag(ACTION_TAG_ANIMATION);
		node->runAction(seq);
	}

	m_isAnimating = true;
}



NS_END
