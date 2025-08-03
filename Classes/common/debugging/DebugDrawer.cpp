#include "DebugDrawer.h"

#include <cstdio>

NS_BEGIN

static Brush* NullBrush = new Brush();

Brush * Brush::create()
{
    Brush * ret = new (std::nothrow) Brush();
    if (ret && ret->init())
    {
        ret->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(ret);
    }
    return ret;
}

bool Brush::init()
{
    if(Node::init())
    {
        m_draw = DrawNode::create(1.0f);
        this->addChild(m_draw);
    }
    return true;
}


Brush::Brush():
    m_draw(nullptr),
	m_clearing(false)
{
    
}

Brush::~Brush()
{
	m_draw = nullptr;
}

void Brush::clear()
{
    if(m_draw)
        m_draw->clear();
}

void Brush::setLineWidth(float width)
{
	if (m_draw)
		m_draw->setLineWidth(width);
}

void Brush::drawRect(Vec2 const& origin, Size const& size, Color4F const& color)
{
	if (m_draw)
		m_draw->drawRect(origin, origin + size, color);
}

void Brush::drawRect(Vec2 const& origin, Vec2 const& destination, Color4F const& color)
{
    if(m_draw)
        m_draw->drawRect(origin, destination, color);
}

void Brush::drawRhombus(Vec2 const& origin, Size const& size, Color4F const& color)
{
	if (m_draw)
	{
		float widthHalf = size.width / 2;
		float heightHalf = size.height / 2;
		Point left(origin.x, origin.y + heightHalf);
		Point top(left.x + widthHalf, left.y + heightHalf);
		Point right(top.x + widthHalf, left.y);
		Point bottom(top.x, origin.y);
		m_draw->drawLine(left, top, color);
		m_draw->drawLine(top, right, color);
		m_draw->drawLine(right, bottom, color);
		m_draw->drawLine(bottom, left, color);
	}
}

void Brush::drawPoint(Vec2 const& point, const float pointSize, Color4F const& color)
{
    if(m_draw)
        m_draw->drawPoint(point, pointSize, color);
}

void Brush::drawLine(Vec2 const& origin, Vec2 const& destination, Color4F const& color)
{
    if(m_draw)
        m_draw->drawLine(origin, destination, color);
}

void Brush::drawPoly(Vec2 const* poli, uint32 numberOfPoints, bool closePolygon, Color4F const& color)
{
	if (m_draw)
		m_draw->drawPoly(poli, numberOfPoints, closePolygon, color);
}

void Brush::drawSolidPoly(Vec2 const* poli, uint32 numberOfPoints, Color4F const& color)
{
	if (m_draw)
		m_draw->drawSolidPoly(poli, numberOfPoints, color);
}

void Brush::drawCubicBezier(Vec2 const& origin, Vec2 const& control1, Vec2 const& control2, Vec2 const& destination, Color4F const& color)
{
    if(m_draw)
        m_draw->drawCubicBezier(origin, control1, control2, destination, 360, color);
}

void Brush::drawCircle(Vec2 const& center, float radius, Color4F const& color)
{
    if(m_draw)
        m_draw->drawCircle(center, radius, 0, 360, false, color);
}

void Brush::drawCircle(Vec2 const& center, float radius, float scaleX, float scaleY, Color4F const& color)
{
	if (m_draw)
		m_draw->drawCircle(center, radius, 0, 360, false, scaleX, scaleY, color);
}

void Brush::drawSolidCircle(Vec2 const& center, float radius, Color4F const& color)
{
	if (m_draw)
		m_draw->drawSolidCircle(center, radius, 0, 360, color);
}

void Brush::drawSolidCircle(Vec2 const& center, float radius, float scaleX, float scaleY, Color4F const& color)
{
	if (m_draw)
		m_draw->drawSolidCircle(center, radius, 0, 360,  scaleX, scaleY, color);
}

Brush* Brush::setClearing(bool clearing)
{
	m_clearing = clearing;
	return this;
}


DebugDrawer::DebugDrawer():
	m_scheduler(nullptr),
	m_drawLayer(nullptr)
{
	m_drawLayer = Layer::create();
	CC_SAFE_RETAIN(m_drawLayer);

	m_scheduler = Director::getInstance()->getScheduler();
	CC_SAFE_RETAIN(m_scheduler);
	m_scheduler->scheduleUpdate(this, UPDATE_PRIORITY_DEBUG, false);

}


DebugDrawer::~DebugDrawer()
{
	if (m_scheduler)
	{
		m_scheduler->unscheduleUpdate(this);
		CC_SAFE_RELEASE_NULL(m_scheduler);
	}

	CC_SAFE_RELEASE_NULL(m_drawLayer);
}



DebugDrawer* DebugDrawer::instance()
{
	static DebugDrawer instance;
	return &instance;
}

void DebugDrawer::attach(Node* node, int32 zOrder)
{
	node->addChild(m_drawLayer, zOrder);
}

void DebugDrawer::detach()
{
    m_drawLayer->removeAllChildren();
    m_drawNodeSet.clear();
	m_drawLayer->removeFromParent();
}

Brush* DebugDrawer::getDrawByTag(std::string const& tag)
{
	if (!m_filterTags.empty() && m_filterTags.find(tag) == m_filterTags.end())
		return NullBrush;

	Brush* node = m_drawNodeSet.at(tag);
	if (!node)
	{
        node = Brush::create();
		m_drawNodeSet.insert(tag, node);
		m_drawLayer->addChild(node);
	}
	return node;
}


void DebugDrawer::clear(bool all)
{
	for (auto p : m_drawNodeSet)
	{
		if (all || p.second->isClearing())
			this->getDrawByTag(p.first)->clear();
	}
}

void DebugDrawer::update(float delta)
{
	this->clear(false);
}

NS_END
