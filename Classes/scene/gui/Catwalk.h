#ifndef __CATWALK_H__
#define __CATWALK_H__

#include "common/Common.h"
#include "CatwalkUnit.h"


USING_NS_CC;

NS_BEGIN

typedef std::function<void(
	Ref*,               // Sender
	CatwalkUnit*,		// Selected unit
	int32,				// Prev selected unit index
	int32				// Selected unit index
	)> CatwalkSelectCallback;

typedef std::function<void(
	Ref*,               // Sender
	CatwalkUnit*,		// Prev selected unit
	CatwalkUnit*,		// Expected selected unit
	int32,				// Prev selected unit index
	int32				// Expected selected unit index
	)> CatwalkBeginScrollCallback;

class Catwalk : public LayerColor
{
public:
	Catwalk();
	~Catwalk();

	static Catwalk* create(cocos2d::Size const& contentSize, cocos2d::Size const& containerSize, float minScale, float maxScale, int32 minNumOfVisible, float lightness);
	bool init(cocos2d::Size const& contentSize, cocos2d::Size const& containerSize, float minScale, float maxScale, int32 minNumOfVisible, float lightness);

	virtual void update(float dt) override;

	void setUnitList(Vector<CatwalkUnit*> const& units);
	Vector<CatwalkUnit*> const& getUnitList() const { return m_unitList; }
	void setSelected(int32 index, bool animated = false);

	void setBeginScrollEventListener(CatwalkBeginScrollCallback const& callback);
	void setSelectEventListener(CatwalkSelectCallback const& callback);

private:
	virtual bool onTouchBegan(Touch* touch, Event* event) override;
	virtual void onTouchEnded(Touch* touch, Event* event) override;
	virtual void onTouchCancelled(Touch* touch, Event* event) override;
	virtual void onTouchMoved(Touch* touch, Event* event) override;

	void step(Node* target, float length);
	void updatePosition(Node* target, Vec2 newPos);

	int32 whichUnitContainsPoint(cocos2d::Point const& p);

	void startArrangement(float offset, int32 selectedIndex);
	void stopArrangement();
	void arrange();
	void setSelectedInternal(int32 index, bool animated = false);

	Node* m_unitContainer;
	Vector<CatwalkUnit*> m_unitList;
	DrawNode* m_debugDrawNode;

	PointArray* m_points;
	float m_tension;

	cocos2d::Point m_prevTouchPoint;
	bool m_touchMoved;
	float m_prevTouchOffset;

	float m_currPoint;
	float m_minExtrPoint;
	float m_maxExtrPoint;
	float m_unitSpan;
	float m_unitsLength;
	int32 m_selectedIndex;
	int32 m_expectedIndex;
    int32 m_tickedIndex;
	int32 m_minVisibleUnits;

	int32 m_minNumOfVisible;
	float m_minScale;
	float m_maxScale;
	float m_lightness;

	float m_duration;
	float m_elapsed;
	float m_moveOffset;
	bool m_arranging;
	float m_prevOffset;

	bool m_selectCallbackDelayed;
	CatwalkSelectCallback m_selectCallback;
	CatwalkBeginScrollCallback m_beginScrollCallback;
};


NS_END

#endif // __CATWALK_VIEW_H__
