#include "GuidepostLayer.h"

#include "game/World.h"

NS_BEGIN

GuidepostLayer* GuidepostLayer::create()
{
	GuidepostLayer* ret = new (std::nothrow) GuidepostLayer();
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

GuidepostLayer::GuidepostLayer()
{
}

GuidepostLayer::~GuidepostLayer()
{
}

bool GuidepostLayer::init()
{
	if (!Layer::init())
		return false;

	World::getInstance()->setLocatorObjectLifecycleListener(this);

	return true;
}

void GuidepostLayer::update(float delta)
{
	for (auto p : m_guideposts)
	{
		Guidepost* guidepost = p.second;
		guidepost->update(delta);
	}
}

void GuidepostLayer::cleanAfterUpdate()
{
	for (auto p : m_guideposts)
	{
		Guidepost* guidepost = p.second;
		if (guidepost->isActive())
			guidepost->cleanUpdateMask();
	}
}

void GuidepostLayer::onLocatorObjectDestroyed(ObjectGuid const& guid)
{
	auto it = m_guideposts.find(guid);
	if (it != m_guideposts.end())
	{
		Guidepost* guidepost = (*it).second;
		this->removeChild(guidepost, true);
		m_guideposts.erase(it);
	}
}

void GuidepostLayer::onLocatorObjectActivated(DataLocatorObject* data)
{
	NS_ASSERT(data->getGuid() != ObjectGuid::EMPTY);

	auto it = m_guideposts.find(data->getGuid());
	if (it != m_guideposts.end())
		(*it).second->onActivated();
	else
	{
		Guidepost* guidepost = Guidepost::createWithData(data);
		m_guideposts.insert(data->getGuid(), guidepost);
		guidepost->onActivated();
		this->addChild(guidepost);
	}
}

void GuidepostLayer::onLocatorObjectInactivated(ObjectGuid const& guid)
{
	auto it = m_guideposts.find(guid);
	if (it != m_guideposts.end())
	{
		(*it).second->onInactivated();
	}
}


NS_END