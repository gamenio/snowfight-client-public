#include "Cloud.h"


NS_BEGIN

#define FRAGMENT_FRAMENAME								"cloud_fragment.png"
#define FRAGMENT_ACTION_START_SCALE						0.06f

const float Cloud::ANIMATION_DURATION = 2.f;

Cloud* Cloud::create(float interval, bool isAheadInterval)
{
	Cloud* pRet = new Cloud();
	if (pRet && pRet->init(interval, isAheadInterval))
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

bool Cloud::init(float interval, bool isAheadInterval)
{
	if (!Node::init())
		return false;

	m_interval = interval;
	m_isAheadInterval = isAheadInterval;

	this->setIgnoreAnchorPointForPosition(false);
	this->setAnchorPoint(Point::ANCHOR_MIDDLE);

	m_fragmentSp1 = Sprite::createWithSpriteFrameName(FRAGMENT_FRAMENAME);
	m_fragmentSp1->setName("Fragment1");
	m_fragmentSp1->setVisible(false);
	m_fragmentSp2 = Sprite::createWithSpriteFrameName(FRAGMENT_FRAMENAME);
	m_fragmentSp2->setName("Fragment2");
	m_fragmentSp2->setVisible(false);
	m_fragmentSp3 = Sprite::createWithSpriteFrameName(FRAGMENT_FRAMENAME);
	m_fragmentSp3->setName("Fragment3");
	m_fragmentSp3->setVisible(false);
	this->addChild(m_fragmentSp3);
	this->addChild(m_fragmentSp2);
	this->addChild(m_fragmentSp1);

	this->initRepeatedActions();

	return true;
}

Cloud::Cloud() :
	m_stateListener(nullptr),
	m_interval(0),
	m_isAheadInterval(false),
	m_delay(0),
	m_elapsed(0),
	m_isFirstTick(true),
	m_isInDelay(false),
	m_fragmentSp1(nullptr),
	m_fragmentSp2(nullptr),
	m_fragmentSp3(nullptr),
	m_currAction(nullptr),
	m_isStopActionsDelayed(false)
{
}

Cloud::~Cloud()
{
	m_currAction = nullptr;
	m_actions.clear();

	m_stateListener = nullptr;
	m_fragmentSp1 = nullptr;
	m_fragmentSp2 = nullptr;
	m_fragmentSp3 = nullptr;
}

void Cloud::setColor(Color3B const& color)
{
	if (_displayedColor != color)
	{
		m_fragmentSp1->setColor(color);
		m_fragmentSp2->setColor(color);
		m_fragmentSp3->setColor(color);
	}

	Node::setColor(color);
}

void Cloud::startAnimation(float delay, int32 loops)
{
	m_actions.clear();
	m_currAction = nullptr;
	m_isStopActionsDelayed = false;

	m_elapsed = 0;
	m_delay = delay;
	m_isFirstTick = true;
	m_isInDelay = m_delay > 0;

	m_fragmentSp1->setVisible(false);
	m_fragmentSp1->setPosition(Point::ZERO);
	m_fragmentSp1->setScale(FRAGMENT_ACTION_START_SCALE);
	{
		auto it = m_repeatedActions.find(m_fragmentSp1);
		NS_ASSERT(it != m_repeatedActions.end());
		this->startRepeatedAction(m_fragmentSp1, loops, (*it).second);
	}

	m_fragmentSp2->setVisible(false);
	m_fragmentSp2->setPosition(Point::ZERO);
	m_fragmentSp2->setScale(FRAGMENT_ACTION_START_SCALE);
	{
		auto it = m_repeatedActions.find(m_fragmentSp2);
		NS_ASSERT(it != m_repeatedActions.end());
		this->startRepeatedAction(m_fragmentSp2, loops, (*it).second);
	}

	m_fragmentSp3->setVisible(false);
	m_fragmentSp3->setPosition(Point::ZERO);
	m_fragmentSp3->setScale(FRAGMENT_ACTION_START_SCALE);
	{
		auto it = m_repeatedActions.find(m_fragmentSp3);
		NS_ASSERT(it != m_repeatedActions.end());
		this->startRepeatedAction(m_fragmentSp3, loops, (*it).second);
	}
}

void Cloud::stopAnimation()
{
	if (m_actions.empty())
		return;

	m_isInDelay = false;
	if (!m_currAction)
	{
		m_isStopActionsDelayed = false;
		m_actions.clear();
	}
	else
		m_isStopActionsDelayed = true;
}

void Cloud::resetAnimation()
{
	this->stopAnimation();

	m_elapsed = 0;
	m_isFirstTick = true;

	m_fragmentSp1->setVisible(false);
	m_fragmentSp1->setPosition(Point::ZERO);
	m_fragmentSp1->setScale(FRAGMENT_ACTION_START_SCALE);
	m_fragmentSp2->setVisible(false);
	m_fragmentSp2->setPosition(Point::ZERO);
	m_fragmentSp2->setScale(FRAGMENT_ACTION_START_SCALE);
	m_fragmentSp3->setVisible(false);
	m_fragmentSp3->setPosition(Point::ZERO);
	m_fragmentSp3->setScale(FRAGMENT_ACTION_START_SCALE);
}

void Cloud::initRepeatedActions()
{
	{
		RepeatedActionConfig config;
		config.move1Duration = 0.6f;
		config.move1Offset = Vec2(0, -5);
		config.scale1Duration = 0.6f;
		config.scale1 = 0.6f;
		config.move2Duration = 0.5f;
		config.move2Offset = Vec2(0, -6);
		config.scale2Duration = 0.5f;
		config.scale2 = 0.7f;
		config.scale3Duration = 0.9f;
		config.scale3 = FRAGMENT_ACTION_START_SCALE;
		config.willAppearCallback = CC_CALLBACK_1(Cloud::onCloudWillAppear, this);
		config.didAppearCallback = CC_CALLBACK_1(Cloud::onCloudDidAppear, this);
		config.didDisappearCallback = CC_CALLBACK_1(Cloud::onCloudDidDisappear, this);
		ActionInterval* action = this->createRepeatedAction(config);
		m_repeatedActions.insert(m_fragmentSp1, action);
	}
	{
		RepeatedActionConfig config;
		config.move1Duration = 0.6f;
		config.move1Offset = Vec2(10, 2);
		config.scale1Duration = 0.6f;
		config.scale1 = 0.6f;
		config.move2Duration = 0.65f;
		config.move2Offset = Vec2(10, 4);
		config.scale2Duration = 0.65f;
		config.scale2 = 0.5f;
		config.scale3Duration = 0.75f;
		config.scale3 = FRAGMENT_ACTION_START_SCALE;
		ActionInterval* action = this->createRepeatedAction(config);
		m_repeatedActions.insert(m_fragmentSp2, action);
	}
	{
		RepeatedActionConfig config;
		config.move1Duration = 0.6f;
		config.move1Offset = Vec2(-6, 2);
		config.scale1Duration = 0.6f;
		config.scale1 = 0.6f;
		config.move2Duration = 0.8f;
		config.move2Offset = Vec2(-6, 2);
		config.scale2Duration = 0.8f;
		config.scale2 = 0.7f;
		config.scale3Duration = 0.6f;
		config.scale3 = FRAGMENT_ACTION_START_SCALE;
		ActionInterval* action = this->createRepeatedAction(config);
		m_repeatedActions.insert(m_fragmentSp3, action);
	}
}

ActionInterval* Cloud::createRepeatedAction(RepeatedActionConfig const& config)
{
	Vector<FiniteTimeAction*> actions;
	if(m_isAheadInterval)
		actions.pushBack(DelayTime::create(m_interval));

	if (config.willAppearCallback)
	{
		auto callback = config.willAppearCallback;
		CallFuncN* callFunc = CallFuncN::create([callback](Node* node) {
			callback(node);
		});
		actions.pushBack(callFunc);
	}
	if (config.didAppearCallback)
	{
		auto callback = config.didAppearCallback;
		CallFuncN* callFunc = CallFuncN::create([callback](Node* node) {
			callback(node);
		});
		actions.pushBack(callFunc);
	}
	actions.pushBack(Spawn::create(MoveBy::create(config.move1Duration, config.move1Offset), ScaleTo::create(config.scale1Duration, config.scale1), nullptr));
	actions.pushBack(EaseQuadraticActionOut::create(Spawn::create(MoveBy::create(config.move2Duration, config.move2Offset), ScaleTo::create(config.scale2Duration, config.scale2), nullptr)));
	actions.pushBack(ScaleTo::create(config.scale3Duration, config.scale3));
	actions.pushBack(Place::create(Point::ZERO));
	if (config.didDisappearCallback)
	{
		auto callback = config.didDisappearCallback;
		CallFuncN* callFunc = CallFuncN::create([callback](Node* node) {
			callback(node);
		});
		actions.pushBack(callFunc);
	}

	if(!m_isAheadInterval)
		actions.pushBack(DelayTime::create(m_interval));

	auto action = Sequence::create(actions);
	NS_ASSERT(std::abs(action->getDuration() - (ANIMATION_DURATION + m_interval)) < 0.0001f);

	return action;
}

void Cloud::startRepeatedAction(Node* target, int32 loops, ActionInterval* repeatedAction)
{
	Action* action;
	if (loops > 0)
		action = Repeat::create(repeatedAction, (uint32)loops);
	else
		action = RepeatForever::create(repeatedAction);

	action->startWithTarget(target);
	m_actions.pushBack(action);
}

void Cloud::onCloudWillAppear(Node* node)
{
	//CCLOG("Cloud::onCloudWillAppear() %s node: %s", this->getName().c_str(), node->getName().c_str());

	if(m_stateListener)
		m_stateListener->onCloudWillAppear(this);
}

void Cloud::onCloudDidAppear(Node* node)
{
	//CCLOG("Cloud::onCloudDidAppear() %s node: %s", this->getName().c_str(), node->getName().c_str());

	m_fragmentSp1->setVisible(true);
	m_fragmentSp2->setVisible(true);
	m_fragmentSp3->setVisible(true);
}

void Cloud::onCloudDidDisappear(Node* node)
{
	//CCLOG("Cloud::onCloudDidDisappear() %s node: %s", this->getName().c_str(), node->getName().c_str());

	m_fragmentSp1->setVisible(false);
	m_fragmentSp2->setVisible(false);
	m_fragmentSp3->setVisible(false);

	if (m_stateListener)
		m_stateListener->onCloudDidDisappear(this);
}

void Cloud::step(float dt)
{
	for (auto it = m_actions.begin(); it != m_actions.end(); )
	{
		Action* action = *it;

		m_currAction = action;
		m_currAction->step(dt);
		m_currAction = nullptr;

		if (action->isDone())
			it = m_actions.erase(it);
		else
			++it;
	}
}

void Cloud::update(float delta)
{
	if (m_isStopActionsDelayed)
	{
		m_currAction = nullptr;
		m_actions.clear();
		m_isStopActionsDelayed = false;
	}
	
	if (m_actions.empty())
		return;

	if (m_isInDelay)
	{
		if (m_isFirstTick)
		{
			m_isFirstTick = false;
			m_elapsed = 0;
		}
		else
		{
			m_elapsed += delta;
		}

		if (m_elapsed >= m_delay)
		{
			m_elapsed -= m_delay;

			this->step(0.f);
			if (m_elapsed > 0)
				this->step(m_elapsed);

			m_elapsed = 0;
			m_isInDelay = false;
		}
	}
	else
		this->step(delta);
}

NS_END
