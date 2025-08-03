#ifndef __DEBUG_DRAWER_H__
#define __DEBUG_DRAWER_H__

#include "common/Common.h"

USING_NS_CC;

NS_BEGIN

class Brush: public Node
{
public:
    static Brush* create();
    bool init();
    
    Brush();
    ~Brush();
    
    void clear();
	void setLineWidth(float width);
	void drawRect(Vec2 const& origin, cocos2d::Size const& size, Color4F const& color);
    void drawRect(Vec2 const& origin,  Vec2 const& destination, Color4F const& color);
	void drawRhombus(Vec2 const& origin, cocos2d::Size const& size, Color4F const& color);
    void drawPoint(Vec2 const& point, const float pointSize, Color4F const& color);
    void drawLine(Vec2 const& origin, Vec2 const& destination, Color4F const& color);
	void drawPoly(Vec2 const* poli, uint32 numberOfPoints, bool closePolygon, Color4F const& color);
	void drawSolidPoly(Vec2 const* poli, uint32 numberOfPoints, Color4F const& color);
    void drawCubicBezier(Vec2 const& origin, Vec2 const& control1, Vec2 const& control2, Vec2 const& destination, Color4F const& color);
	// 绘制圆形需要在每个更新周期内至少调用一次clear()，否则会出现图形绘制不出来的问题
    void drawCircle(Vec2 const& center, float radius, Color4F const& color);
	void drawCircle(Vec2 const& center, float radius, float scaleX, float scaleY, Color4F const& color);
	void drawSolidCircle(Vec2 const& center, float radius, Color4F const& color);
	void drawSolidCircle(Vec2 const& center, float radius, float scaleX, float scaleY, Color4F const& color);

	bool isClearing() const { return m_clearing; }
	// 标记为清除，在调用DebugDrawer::clear函数后画布将被清除
	Brush* setClearing(bool clearing);

	DrawNode* getDrawNode() const { return m_draw; }

private:
    DrawNode* m_draw;
	bool m_clearing;
};

class DebugDrawer
{
public:
	static DebugDrawer* instance();

	void attach(Node* node, int32 zOrder);
	void detach();
	Brush* getDrawByTag(std::string const& tag);

	void setFilterTags(std::set<std::string> const& tags) { m_filterTags = tags; }

	void clear(bool all = true);
	void update(float delta);
    
private:
	DebugDrawer();
	~DebugDrawer();

	Scheduler* m_scheduler;
	std::set<std::string> m_filterTags;
	Map<std::string, Brush*> m_drawNodeSet;
	Layer* m_drawLayer;
};


#define sDebugDrawer		DebugDrawer::instance()


NS_END

#endif //__DEBUG_DRAWER_H__

