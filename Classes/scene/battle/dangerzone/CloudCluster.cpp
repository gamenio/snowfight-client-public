#include "CloudCluster.h"

NS_BEGIN

#if NS_DEBUG
#define DEBUG_NON_SPAWNABLE_CLOUD			0
#define DEBUG_RANDOM_POSITION				0

#endif // NS_DEBUG

#define CLOUD_ACTIVE_ZORDER					1
#define CLOUD_INACTIVE_ZORDER				0
#define CLOUD_SPAWN_RADIUS					20.f

#define CLOUDS_TIME_DIFF					0.2f
#define CLOUD_DELAY_DEVIATION				0.45f

const float CloudCluster::LARGE_CLOUD_DELAY			= 0.f;
const float CloudCluster::SMALL_CLOUD_DELAY			= 0.9f;
const float CloudCluster::ACTIVE_CLOUD_DURATION		= Cloud::ANIMATION_DURATION - CLOUDS_TIME_DIFF;

static const float CLOUD_INTERVAL = Cloud::ANIMATION_DURATION - CLOUDS_TIME_DIFF * 2;

CloudCluster* CloudCluster::create()
{
	CloudCluster* pRet = new CloudCluster();
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

bool CloudCluster::init()
{
	if (!Node::init())
		return false;

	this->setIgnoreAnchorPointForPosition(false);
	this->setAnchorPoint(Point::ANCHOR_MIDDLE);

#if NS_DEBUG
	m_debugDraw = DrawNode::create();
	this->addChild(m_debugDraw);
#endif // NS_DEBUG

	m_cloud1 = Cloud::create(CLOUD_INTERVAL, false);
	m_cloud1->setName("Cloud1");
	m_cloud1->setCloudStateListener(this);
	m_cloud1->setVisible(false);
	this->addChild(m_cloud1, CLOUD_INACTIVE_ZORDER);

	m_cloud2 = Cloud::create(CLOUD_INTERVAL, true);
	m_cloud2->setName("Cloud2");
	m_cloud2->setCloudStateListener(this);
	m_cloud2->setVisible(false);
	this->addChild(m_cloud2, CLOUD_INACTIVE_ZORDER);

	return true;
}

CloudCluster::CloudCluster() :
	m_debugDraw(nullptr),
	m_data(nullptr),
	m_isActivated(false),
	m_deviation(0),
	m_elapsed(0),
	m_duration(0),
	m_isFirstTick(true),
	m_performingAction(CloudData::ACTION_NONE),
	m_activeCloud(nullptr),
	m_cloud1(nullptr),
	m_cloud2(nullptr)
{
}

CloudCluster::~CloudCluster()
{
	m_debugDraw = nullptr;
	m_activeCloud = nullptr;
	m_cloud1 = nullptr;
	m_cloud2 = nullptr;

	CC_SAFE_RELEASE_NULL(m_data);
}

void CloudCluster::activate()
{
	if (m_isActivated)
		return;

	NS_ASSERT(m_data != nullptr);

	m_isActivated = true;
	m_activeCloud = m_cloud1;

	// Calculate the duration of loops
	if (m_data->loops > 0)
	{
		m_duration = m_data->loops * CloudCluster::ACTIVE_CLOUD_DURATION;
		m_elapsed = 0;
	}

	if (m_data->isSpawnable)
		m_deviation = random(0.f, CLOUD_DELAY_DEVIATION);

	int32 loops = m_data->loops;
	// Enable random position after reaching the duration of loops and continue looping
	if (loops > 0 && m_data->afterLoopsAction == CloudData::ACTION_ENABLE_RANDOM_POSITION)
		loops = 0;

	int32 cloud1Loops = (int32)std::ceil(loops / 2.f);
	m_cloud1->startAnimation(m_deviation, cloud1Loops);
	int32 cloud2Loops = loops - cloud1Loops;
	if (loops <= 0 || cloud2Loops > 0)
		m_cloud2->startAnimation(m_deviation + CLOUDS_TIME_DIFF, cloud2Loops);

	m_cloud1->setVisible(true);
	m_cloud2->setVisible(true);

	if (m_debugDraw)
		m_debugDraw->clear();

#if DEBUG_RANDOM_POSITION
	if (m_debugDraw)
	{
		m_debugDraw->drawCircle(Point::ZERO, CLOUD_SPAWN_RADIUS, 0, 360, false, loops > 0 ? Color4F::GREEN : Color4F::YELLOW);
	}

#endif
}

void CloudCluster::reset()
{
	this->inactivate();
	
	CC_SAFE_RELEASE_NULL(m_data);
	m_cloud1->resetAnimation();
	m_cloud2->resetAnimation();
}

void CloudCluster::setData(CloudData* data)
{
	CC_SAFE_RETAIN(data);
	CC_SAFE_RELEASE_NULL(m_data);
	m_data = data;

#if DEBUG_NON_SPAWNABLE_CLOUD
	if (!data->isSpawnable)
	{
		m_cloud1->setColor(Color3B::RED);
		m_cloud2->setColor(Color3B::RED);
	}
	else
	{
		m_cloud1->setColor(Color3B::WHITE);
		m_cloud2->setColor(Color3B::WHITE);
	}
#endif
}

void CloudCluster::onCloudWillAppear(Cloud* sender)
{
	if (!m_isActivated)
		return;

	//CCLOG("CloudCluster::onCloudWillAppear activeCloud: %s sender: %s", m_activeCloud->getName().c_str(), sender->getName().c_str());

	m_activeCloud = sender;
	m_activeCloud->setLocalZOrder(CLOUD_ACTIVE_ZORDER);
	Cloud* inactiveCloud = m_activeCloud == m_cloud2 ? m_cloud1 : m_cloud2;
	inactiveCloud->setLocalZOrder(CLOUD_INACTIVE_ZORDER);

	Point pos;
	if (m_performingAction == CloudData::ACTION_ENABLE_RANDOM_POSITION)
	{
		Point center;
		center.x = this->getContentSize().width / 2;
		center.y = this->getContentSize().height / 2;
		float radius = CLOUD_SPAWN_RADIUS;
		// https://programming.guide/random-point-within-circle.html
		float r = radius * sqrt(random(0.f, 1.0f));
		float a = random(0.f, 1.0f) * 2 * (float)M_PI;
		pos.x = center.x + r * std::cos(a);
		pos.y = center.y + r * std::sin(a);
	}
	else
	{
		pos.x = this->getContentSize().width / 2;
		pos.y = this->getContentSize().height / 2;
	}
	m_activeCloud->setPosition(pos);
}

void CloudCluster::onCloudDidDisappear(Cloud* sender)
{
	if (!m_isActivated)
		return;

	//CCLOG("CloudCluster::onCloudDidDisappear activeCloud: %s sender: %s", m_activeCloud->getName().c_str(), sender->getName().c_str());

	if (m_performingAction == CloudData::ACTION_STOP)
		this->inactivate();
}

void CloudCluster::update(float delta)
{
	this->updateLoopsAction(delta);

	m_cloud1->update(delta);
	m_cloud2->update(delta);
}

void CloudCluster::onEnter()
{
	Node::onEnter();
}

void CloudCluster::onExit()
{
	Node::onExit();
}

void CloudCluster::inactivate()
{
	if (!m_isActivated)
		return;

	m_elapsed = 0;
	m_duration = 0;
	m_isFirstTick = true;
	m_performingAction = CloudData::ACTION_NONE;

	m_activeCloud = nullptr;
	m_isActivated = false;
	m_deviation = 0;

	m_cloud1->setVisible(false);
	m_cloud2->setVisible(false);

	if(m_debugDraw)
		m_debugDraw->clear();
}

void CloudCluster::updateLoopsAction(float delta)
{
	if (m_data->loops <= 0 || !m_isActivated || m_performingAction != CloudData::ACTION_NONE)
		return;

	if (m_isFirstTick)
	{
		m_elapsed = 0;
		m_isFirstTick = false;
	}
	else
		m_elapsed += delta;

	if (m_elapsed >= m_duration)
	{
		m_performingAction = m_data->afterLoopsAction;
	}
}


NS_END