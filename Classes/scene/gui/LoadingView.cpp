#include "LoadingView.h"

using namespace cocos2d::ui;

NS_BEGIN


#define FRAME_SIZE							Size(50, 50)
#define LOADING_INDICATOR_SIZE				Size(50, 8)

#define FRAMENAME_LOADING_VIEW_BG			"loading_view_bg.png"

LoadingView* LoadingView::create(Node* owner)
{
	auto ret = new (std::nothrow) LoadingView();
	if (ret && ret->init(owner))
	{
		ret->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(ret);
	}

	return ret;
}

bool LoadingView::init(Node* owner)
{
	if (!LayerColor::initWithColor(Color4B(0, 0, 0, 0)))
		return false;

	m_owner = owner;

	auto touchListener = EventListenerTouchOneByOne::create();
	touchListener->setSwallowTouches(true);

	touchListener->onTouchBegan = CC_CALLBACK_2(LoadingView::onTouchBegan, this);
	touchListener->onTouchMoved = CC_CALLBACK_2(LoadingView::onTouchMoved, this);
	touchListener->onTouchEnded = CC_CALLBACK_2(LoadingView::onTouchEnded, this);
	touchListener->onTouchCancelled = CC_CALLBACK_2(LoadingView::onTouchCancelled, this);
	this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(touchListener, this);

	auto keyboardListener = EventListenerKeyboard::create();
	keyboardListener->onKeyReleased = CC_CALLBACK_2(LoadingView::onKeyReleased, this);
	this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(keyboardListener, this);

	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();


	m_mainPanel = Node::create();
	m_mainPanel->setContentSize(FRAME_SIZE);
	m_mainPanel->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_mainPanel->setPosition(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2);
	this->addChild(m_mainPanel);

	m_background = Scale9Sprite::createWithSpriteFrameName(FRAMENAME_LOADING_VIEW_BG);
	m_background->setContentSize(m_mainPanel->getContentSize());
	m_background->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_mainPanel->addChild(m_background);

	m_loadingIndicator = LoadingIndicator::create(LOADING_INDICATOR_SIZE, Color3B(239, 239, 239));
	m_loadingIndicator->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_loadingIndicator->setPosition(m_mainPanel->getContentSize() / 2);
	m_mainPanel->addChild(m_loadingIndicator);

	return true;
}

LoadingView::LoadingView() :
	m_owner(nullptr),
	m_mainPanel(nullptr),
	m_background(nullptr),
	m_loadingIndicator(nullptr)
{
}

LoadingView::~LoadingView()
{
	m_owner = nullptr;
	m_mainPanel = nullptr;
	m_background = nullptr;
	m_loadingIndicator = nullptr;
}

bool LoadingView::onTouchBegan(Touch* touch, Event* event)
{
	return true;
}

void LoadingView::onTouchEnded(Touch* touch, Event* event)
{
}

void LoadingView::onTouchCancelled(Touch* touch, Event* event)
{
}

void LoadingView::onTouchMoved(Touch* touch, Event* event)
{
}

void LoadingView::onKeyReleased(EventKeyboard::KeyCode keyCode, Event *event)
{
	switch (keyCode)
	{
	case cocos2d::EventKeyboard::KeyCode::KEY_BACK:
		event->stopPropagation();
		break;
	default:
		break;
	}
}

void LoadingView::show()
{
	auto scene = Director::getInstance()->getRunningScene();
	NS_ASSERT(scene != nullptr);
	scene->addChild(this);
}

void LoadingView::dismiss()
{
    this->removeFromParent();
}


NS_END
