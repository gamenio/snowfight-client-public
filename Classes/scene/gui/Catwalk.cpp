#include "Catwalk.h"

#include "scene/SoundMgr.h"
#include "scene/Utils.h"

NS_BEGIN

#if NS_DEBUG

#define DEBUG_SPLINE_PATH				0
#define DEBUG_TRIMMED_BOUNDING_BOX		0

#endif // NS_DEBUG

#define MIN_UNIT_SPAN					0.06f
#define MAX_LENGTH						1.0f
#define MAX_LENGTH_HALF					0.5f	


#define DRAG_MAX_INC						0.05f	
#define DRAG_DAMPING_COEFF					0.004f //0.002f
#define ARRANGING_DURATION					0.2f

#define UNSELECTED_INDEX				-1

// CatmullRom Spline formula:
Vec2 cardinalSplineAt(const Vec2 &p0, const Vec2 &p1, const Vec2 &p2, const Vec2 &p3, float tension, float t)
{
	float t2 = t * t;
	float t3 = t2 * t;

	/*
	* Formula: s(-ttt + 2tt - t)P1 + s(-ttt + tt)P2 + (2ttt - 3tt + 1)P2 + s(ttt - 2tt + t)P3 + (-2ttt + 3tt)P3 + s(ttt - tt)P4
	*/
	float s = (1 - tension) / 2;

	float b1 = s * ((-t3 + (2 * t2)) - t);                      // s(-t3 + 2 t2 - t)P1
	float b2 = s * (-t3 + t2) + (2 * t3 - 3 * t2 + 1);          // s(-t3 + t2)P2 + (2 t3 - 3 t2 + 1)P2
	float b3 = s * (t3 - 2 * t2 + t) + (-2 * t3 + 3 * t2);      // s(t3 - 2 t2 + t)P3 + (-2 t3 + 3 t2)P3
	float b4 = s * (t3 - t2);                                   // s(t3 - t2)P4

	float x = (p0.x*b1 + p1.x*b2 + p2.x*b3 + p3.x*b4);
	float y = (p0.y*b1 + p1.y*b2 + p2.y*b3 + p3.y*b4);

	return Vec2(x, y);
}

Catwalk* Catwalk::create(Size const& contentSize, Size const& containerSize, float minScale, float maxScale, int32 minNumOfVisible, float lightness)
{
	Catwalk* ret = new (std::nothrow) Catwalk();
	if (ret && ret->init(contentSize, containerSize, minScale, maxScale, minNumOfVisible, lightness))
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

bool Catwalk::init(Size const& contentSize, Size const& containerSize, float minScale, float maxScale, int32 minNumOfVisible, float lightness)
{
	if (!LayerColor::initWithColor(Color4B(0, 0, 0, 0)))
		return false;

	auto touchListener = EventListenerTouchOneByOne::create();
	touchListener->setSwallowTouches(true);

	touchListener->onTouchBegan = CC_CALLBACK_2(Catwalk::onTouchBegan, this);
	touchListener->onTouchMoved = CC_CALLBACK_2(Catwalk::onTouchMoved, this);
	touchListener->onTouchEnded = CC_CALLBACK_2(Catwalk::onTouchEnded, this);
	touchListener->onTouchCancelled = CC_CALLBACK_2(Catwalk::onTouchCancelled, this);
	this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(touchListener, this);

	this->setIgnoreAnchorPointForPosition(false);

	m_minScale = minScale;
	m_maxScale = maxScale;
	m_minNumOfVisible = minNumOfVisible;
	m_lightness = lightness;

	m_points = PointArray::create(10);
	CC_SAFE_RETAIN(m_points);

	m_unitContainer = LayerColor::create(Color4B(255, 255, 255, 0));
	m_unitContainer->setIgnoreAnchorPointForPosition(false);
	m_unitContainer->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_unitContainer->setPosition(contentSize.width / 2, contentSize.height / 2);
	m_unitContainer->setContentSize(containerSize);
	this->addChild(m_unitContainer);

	m_debugDrawNode = DrawNode::create(1.f);
	m_debugDrawNode->setAnchorPoint(m_unitContainer->getAnchorPoint());
	m_debugDrawNode->setPosition(m_unitContainer->getPosition());
	m_debugDrawNode->setContentSize(m_unitContainer->getContentSize());
	this->addChild(m_debugDrawNode);

	this->setContentSize(contentSize);
	this->scheduleUpdate();

	return true;
}

void Catwalk::step(Node* target, float length)
{
	float _deltaT = (float)1 / (m_points->count() - 1);

	ssize_t p;
	float lt;

	// eg.
	// p..p..p..p..p..p..p
	// 1..2..3..4..5..6..7
	// want p to be 1, 2, 3, 4, 5, 6
	if (length == 1)
	{
		p = m_points->count() - 1;
		lt = 1;
	}
	else
	{
		p = length / _deltaT;
		lt = (length - _deltaT * (float)p) / _deltaT;
	}

	// Interpolate
	Vec2 pp0 = m_points->getControlPointAtIndex(p - 1);
	Vec2 pp1 = m_points->getControlPointAtIndex(p + 0);
	Vec2 pp2 = m_points->getControlPointAtIndex(p + 1);
	Vec2 pp3 = m_points->getControlPointAtIndex(p + 2);

	Vec2 newPos = cardinalSplineAt(pp0, pp1, pp2, pp3, m_tension, lt);

	this->updatePosition(target, newPos);
}

void Catwalk::update(float dt)
{
	if (!m_arranging)
		return;

	m_elapsed += dt;
	float t = m_elapsed / m_duration;
	t = MIN(1.0f, t);

	float newOffset = m_moveOffset * t;
	float diff = newOffset - m_prevOffset;
	//CCLOG("currPoint: %.15f diff: %.15f t: %.15f newoff:%.15f prevoff:%.15f", m_currPoint, diff, t, newOffset,  m_prevOffset);
	m_prevOffset = newOffset;
	m_currPoint += diff;

	this->arrange();

	if (m_elapsed >= m_duration)
	{
		int32 prevSelIndex = m_selectedIndex;
		m_selectedIndex = m_expectedIndex;
		//CCLOG("sel sprite index: %d  currpoint: %f", m_selectedIndex, m_currPoint);

		if(std::fabs(m_moveOffset) >= m_unitSpan / 2.5f || m_tickedIndex != m_selectedIndex)
		{
			sSoundMgr->play(SOUND_UNIT_TICK);
		}
		m_tickedIndex = m_selectedIndex;

		this->stopArrangement();

		if (m_selectCallbackDelayed && m_selectCallback)
		{
			CatwalkUnit* unit = m_unitList.at(m_selectedIndex);
			m_selectCallback(this, unit, prevSelIndex, m_selectedIndex);
		}
	}
    else
    {
        // Play Tick sound
        float a = MAX_LENGTH_HALF - m_currPoint;
        float n = a / m_unitSpan;
        int32 index = (int32)n;
        float remainder = a - index * m_unitSpan;
        if (remainder > m_unitSpan / 2)
            index += 1;
        int32 maxIndex = MAX(m_selectedIndex, m_expectedIndex);
        int32 minIndex = MIN(m_selectedIndex, m_expectedIndex);
        if(index > minIndex && index < maxIndex)
        {
			if (index != m_tickedIndex)
			{
				sSoundMgr->play(SOUND_UNIT_TICK);
			}
                

            m_tickedIndex = index;
        }
        //CCLOG("UPDATE index: %d/%f expectedIndex:%d offset:%f", index, a / m_unitSpan, m_expectedIndex, diff);
    }
}

void Catwalk::setUnitList(Vector<CatwalkUnit*> const& units)
{
    if(units.empty())
        return;
    
	Size containerSize = m_unitContainer->getContentSize();
	m_unitList.clear();
	for (auto it = units.begin(); it != units.end(); ++it)
	{
		CatwalkUnit* unit = *it;
		unit->setAnchorPoint(Point::ANCHOR_MIDDLE);
		m_unitList.pushBack(unit);
	}

	Rect splineBounds(0, 0, containerSize.width, containerSize.height);
	std::vector<Vec2> points;

	// Curve 1
	//points.push_back(Vec2(0, splineBounds.size.height));
	//points.push_back(Vec2(splineBounds.size.width / 2, 0));
	//points.push_back(Vec2(splineBounds.size.width, splineBounds.size.height));
	// Curve 2
	//points.push_back(Vec2(0, 0));
	//points.push_back(Vec2(splineBounds.size.width / 2, splineBounds.size.height));
	//points.push_back(Vec2(splineBounds.size.width, 0));

	// Straight line
	points.push_back(Vec2(0, splineBounds.size.height / 2));
	points.push_back(Vec2(splineBounds.size.width, splineBounds.size.height / 2));

	m_points->setControlPoints(points);

	m_unitContainer->removeAllChildren();
	for (auto it = m_unitList.begin(); it != m_unitList.end(); ++it)
	{
		m_unitContainer->addChild(*it);
	}

	int32 numUnits = (int32)m_unitList.size();
	m_minVisibleUnits = MIN(numUnits, m_minNumOfVisible);
	m_unitSpan = 0;
	if (numUnits > 1)
	{
		if (m_minVisibleUnits > 1)
		{
			m_unitSpan = MAX_LENGTH_HALF / (m_minNumOfVisible - 1 + 0.5f);
		}
		else
			m_unitSpan = MAX_LENGTH;
	}

	m_unitsLength = m_unitSpan * (numUnits - 1);
    m_expectedIndex = m_selectedIndex = 0;
	m_minExtrPoint = -m_unitsLength + MIN_UNIT_SPAN * (m_minVisibleUnits - 1);
	m_maxExtrPoint = MAX_LENGTH - MIN_UNIT_SPAN * (m_minVisibleUnits - 1);
    m_currPoint = MAX_LENGTH_HALF;
    
    this->arrange();
}

void Catwalk::setBeginScrollEventListener(CatwalkBeginScrollCallback const& callback)
{
	m_beginScrollCallback = callback;
}

void Catwalk::setSelectEventListener(CatwalkSelectCallback const& callback)
{
	m_selectCallback = callback;
}

void Catwalk::updatePosition(Node* target, Vec2 newPos)
{
	target->setPosition(newPos);
}

int32 Catwalk::whichUnitContainsPoint(Point const& p)
{
	auto&unitContainer = m_unitContainer->getChildren();
	Point localPoint = m_unitContainer->convertToNodeSpace(p);
	//CCLOG("touch point:[%f, %f], [%f, %f]", m_currTouchPoint.x, m_currTouchPoint.y, p.x, p.y);
    int32 nUnits = (int32)unitContainer.size();
	for (int32 i = nUnits - 1; i >= 0; --i)
	{
		CatwalkUnit* unit = dynamic_cast<CatwalkUnit*>(unitContainer.at(i));
		if (!unit || !unit->isVisible())
			continue;

		if (unit->getTrimmedBoundingBox().containsPoint(localPoint))
		{
			int32 sel = (int32)m_unitList.getIndex(unit);
			NS_ASSERT(sel != UINT_MAX);
			CCLOG("HIT TEST index: %d zorder index:%d ", sel, i);
			return sel;
		}
	}

	return UINT_MAX;
}

bool Catwalk::onTouchBegan(Touch* touch, Event* event)
{
	if (!m_unitList.empty())
	{
		m_prevTouchPoint = touch->getLocation();
		Point localPoint = this->convertToNodeSpace(m_prevTouchPoint);
		if(this->getBoundingBox().containsPoint(localPoint))
        {
            m_tickedIndex = m_selectedIndex;
			return true;
        }
	}

	return false;
}

void Catwalk::onTouchEnded(Touch* touch, Event* event)
{
	if (!m_touchMoved)
	{
		int32 hitIndex = this->whichUnitContainsPoint(touch->getLocation());
		if(hitIndex != UINT_MAX)
			this->setSelectedInternal(hitIndex, true);
	}
	else
	{
		int32 count = (int32)m_unitList.size();
		float offset = 0;
		int32 selectedIndex = -1;

		if (m_currPoint >= MAX_LENGTH_HALF - m_unitsLength && m_currPoint <= MAX_LENGTH_HALF)
		{
			float a = MAX_LENGTH_HALF - m_currPoint;
			int32 index = (int32)(a / m_unitSpan);
			float remainder = a - index * m_unitSpan;
			//CCLOG("STEP remainder:%f", remainder);
			//CCLOG("TOUCH ENDED currpoint:%f stretchlen:%f index:%d remainder:%f %f", m_currPoint, m_stretchLen, index, rem, remainder);
			if (remainder > m_unitSpan / 2)
			{
				offset = -(m_unitSpan - remainder);
				selectedIndex = index + 1;
			}
			else
			{
				offset = remainder;
				selectedIndex = index;
			}

		}
		else
		{
			if (m_currPoint > MAX_LENGTH_HALF)
			{
				offset = MAX_LENGTH_HALF - m_currPoint;
				selectedIndex = 0;
			}
			else
			{
				offset = MAX_LENGTH_HALF - (m_currPoint + m_unitsLength);
				selectedIndex = count - 1;
			}
		}

		if (selectedIndex >= 0)
		{
			m_selectCallbackDelayed = true;
			this->startArrangement(offset, selectedIndex);
		}
	}

	m_touchMoved = false;
	m_prevTouchPoint = Point::ZERO;
}

void Catwalk::onTouchCancelled(Touch* touch, Event* event)
{
	m_touchMoved = false;
	m_prevTouchPoint = Point::ZERO;
}

void Catwalk::onTouchMoved(Touch* touch, Event* event)
{
	Point touchPoint = touch->getLocation();
	float offset = m_prevTouchPoint.x - touchPoint.x;
	if (offset == 0)
		return;

	if (!m_touchMoved && std::fabs(Utils::convertDistanceFromPointToInch(offset)) < GESTURE_THRESHOLD_INCH)
		return;
		
	if(m_arranging)
		this->stopArrangement();

	float inc = MIN(DRAG_MAX_INC, std::abs(offset) * DRAG_DAMPING_COEFF);
	m_currPoint += offset < 0 ? inc : -inc;
	m_currPoint = MAX(m_minExtrPoint, MIN(m_maxExtrPoint, m_currPoint));
	//CCLOG("TOUCH MOVED offset: %f point:[%f,%f] inc: %.15f", offset, touchPoint.x, touchPoint.y, inc);
	this->arrange();
    
    // Play Tick sound
    int32 count = (int32)m_unitList.size();
    float a = MAX_LENGTH_HALF - m_currPoint;
    float n = a / m_unitSpan;
    int32 index = (int32)n;
    if(offset < 0)
    {
        if(n < 0)
            index = 0;
        else if(n > count - 1)
            index = UNSELECTED_INDEX;
        else if(n > 0)
            index += 1;
    }
    else
    {
        if(n < 0)
            index = UNSELECTED_INDEX;
        else if(n > count - 1)
            index = count - 1;
    }
    
    bool reversed = m_prevTouchOffset * offset < 0;
    if(index != m_tickedIndex && !reversed)
    {
        sSoundMgr->play(SOUND_UNIT_TICK);
    }
    //CCLOG("TOUCH MOVED index: %d/%f tickedIndex:%d offset:%f reversed:%d", index, a / m_unitSpan, m_tickedIndex, offset, reversed);
    m_tickedIndex = index;

	if (!m_touchMoved)
	{
		if (m_beginScrollCallback)
		{
			CatwalkUnit* unit = m_unitList.at(m_selectedIndex);
			m_beginScrollCallback(this, unit, nullptr, m_selectedIndex, UNSELECTED_INDEX);
		}

	}

	m_prevTouchOffset = offset;
	m_prevTouchPoint = touchPoint;
	m_touchMoved = true;
}

Catwalk::Catwalk() :
	m_unitContainer(nullptr),
	m_debugDrawNode(nullptr),
	m_points(nullptr),
	m_tension(0),
	m_currPoint(0),
	m_duration(ARRANGING_DURATION),
	m_elapsed(0),
	m_prevTouchPoint(Point::ZERO),
	m_selectedIndex(UNSELECTED_INDEX),
	m_expectedIndex(UNSELECTED_INDEX),
    m_tickedIndex(UNSELECTED_INDEX),
	m_minVisibleUnits(0),
	m_minNumOfVisible(0),
	m_minScale(0),
	m_maxScale(0),
	m_lightness(0),
	m_unitSpan(0),
	m_touchMoved(false),
	m_moveOffset(0),
	m_arranging(false),
	m_prevOffset(0),
	m_selectCallbackDelayed(false)
{

}

Catwalk::~Catwalk()
{
	CC_SAFE_RELEASE_NULL(m_points);
	m_debugDrawNode = nullptr;
}

void Catwalk::startArrangement(float offset, int32 selectedIndex)
{
	if (m_arranging)
		this->stopArrangement();

	m_moveOffset = offset;
	m_expectedIndex = selectedIndex;
	m_elapsed = 0;
	m_arranging = true;
	//CCLOG("ARRANGE START currpoint:%.15f totallen:%f offset: %.15f index: %d", m_currPoint, m_unitsLength, offset, selectedIndex);
}

void Catwalk::stopArrangement()
{
	if (!m_arranging)
		return;

	m_elapsed = 0;
	m_prevOffset = 0;
	m_moveOffset = 0;
	m_arranging = false;
	// CCLOG("ARRANGE STOP");
}

void Catwalk::arrange()
{

#if DEBUG_SPLINE_PATH || DEBUG_TRIMMED_BOUNDING_BOX
	m_debugDrawNode->clear();
#endif

#if DEBUG_SPLINE_PATH
	m_debugDrawNode->drawCardinalSpline(m_points, m_tension, 100, Color4F::MAGENTA);
#endif


	int32 numUnits = (int32)m_unitList.size();

	int32 firstIndex = 0;
	int32 lastIndex = 0;
	float nextP = 0;
	float delta = 0;

	if (numUnits > 1)
	{
		if (m_currPoint <= 0)
		{
			float remainLen = m_unitsLength - std::abs(m_currPoint);
			firstIndex = MIN(numUnits - m_minVisibleUnits, (int32)std::ceil(std::abs(m_currPoint) / m_unitSpan));
			lastIndex = MIN(m_unitsLength, (std::abs(m_currPoint) + MAX_LENGTH)) / m_unitSpan;
			nextP = MAX(0, m_currPoint + firstIndex * m_unitSpan);
			delta = MIN(m_unitSpan, remainLen / (m_minVisibleUnits - 1));

			//CCLOG("currpoint:%.15f unitspan: %.15f remainLen: %.15f step range:[%d,%d]", m_currPoint, m_unitSpan, remainLen, firstIndex, lastIndex);
		}
		else
		{
			float remainLen = MIN(m_unitsLength, MAX_LENGTH - m_currPoint);
			firstIndex = 0;
			lastIndex = MAX(m_minVisibleUnits - 1, (int32)(remainLen / m_unitSpan));
			nextP = m_currPoint;
			delta = MIN(m_unitSpan, remainLen / (m_minVisibleUnits - 1));
		}
			
	}
	else
	{
		nextP = MAX(0, m_currPoint);
	}

	//CCLOG("ARRANGE currpoint:%.15f unitspan: %f step range:[%d,%d] point range:[%f,%f] delta: %f ",
	//	m_currPoint, 
	//	m_unitSpan, 
	//	firstIndex, lastIndex, 
	//	nextP, 
	//	nextP + (lastIndex - firstIndex) * delta,
	//	delta);

	for (int32 i = 0; i < numUnits; ++i)
	{
		CatwalkUnit* unit = m_unitList.at(i);

		if (i < firstIndex || i > lastIndex)
		{
			unit->setVisible(false);
			unit->setScale(1.0f);
			unit->setLocalZOrder(0);
			unit->setAnchorPoint(Point::ANCHOR_MIDDLE);
			//CCLOG("OUT VIEWPORT spriteindex:%d point:%f", i, nextP);
		}
		else
		{
			float a = (nextP > MAX_LENGTH_HALF ? MAX_LENGTH - nextP : nextP);
			float p = a / MAX_LENGTH_HALF;

			unit->setVisible(true);

			// Scale
			float scale = (m_maxScale - m_minScale) * p + m_minScale;
			unit->setScale(scale);

			// Zorder
			int32 zorder = (int32)(a * 100);
			unit->setLocalZOrder(zorder);

			// AnchorPoint
			Point anchorPoint(unit->getAnchorPoint());
			if (nextP > MAX_LENGTH_HALF)
				anchorPoint.x = scale * 0.5f;
			else
				anchorPoint.x = 1.0 - scale * 0.5f;
			//anchorPoint.y = scale * 0.5f;
			unit->setAnchorPoint(anchorPoint);

			// Lightness
			float lightness = MIN(0.9f, m_lightness * (1.0f - p));
			unit->setLightness(lightness);

			//CCLOG("IN VIEWPORT spriteindex:%d point:%f zorder:%d scale:%.15f anchorpoint:[%f,%f] lightness:%f A:%f P:%f", 
			//	i, nextP, zorder, scale, 
			//	anchorPoint.x, anchorPoint.y, 
			//	lightness, 
			//	a, p);

			this->step(unit, nextP);
			nextP += delta;

#if DEBUG_TRIMMED_BOUNDING_BOX
			Rect r;
			r = unit->getBoundingBox();
			m_debugDrawNode->drawRect(r.origin, r.origin + r.size, Color4F::GREEN);
			r = unit->getTrimmedBoundingBox();
			m_debugDrawNode->drawRect(r.origin, r.origin + r.size, Color4F::RED);
#endif
		}
	}
}

void Catwalk::setSelectedInternal(int32 index, bool animated)
{
	if (m_selectedIndex == index)
		return;

	float newPoint = MAX_LENGTH_HALF - index * m_unitSpan;
	if (animated)
	{
		float offset = newPoint - m_currPoint;
		this->startArrangement(offset, index);

		m_selectCallbackDelayed = true;
		if (m_beginScrollCallback)
		{
			CatwalkUnit* prevUnit = m_unitList.at(m_selectedIndex);
			CatwalkUnit* expectedUnit = m_unitList.at(index);
			m_beginScrollCallback(this, prevUnit, expectedUnit, m_selectedIndex, index);
		}

	}
	else
	{
		int32 prevSelIndex = m_selectedIndex;
		m_expectedIndex = m_selectedIndex = index;
		m_currPoint = newPoint;
		this->arrange();

		m_selectCallbackDelayed = false;
		if (m_selectCallback)
		{
			CatwalkUnit* unit = m_unitList.at(m_selectedIndex);
			m_selectCallback(this, unit, prevSelIndex, m_selectedIndex);
		}
	}
}

void Catwalk::setSelected(int32 index, bool animated /*= false*/)
{
	if (m_selectedIndex == index)
		return;

	float newPoint = MAX_LENGTH_HALF - index * m_unitSpan;
	m_selectCallbackDelayed = false;
	if (animated)
	{
		float offset = newPoint - m_currPoint;
		this->startArrangement(offset, index);
	}
	else
	{
		m_expectedIndex = m_selectedIndex = index;
		m_currPoint = newPoint;
		this->arrange();
	}
}

NS_END
