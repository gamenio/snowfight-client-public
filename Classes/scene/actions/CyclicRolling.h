#ifndef __CYCLIC_ROLLING_H__
#define __CYCLIC_ROLLING_H__

#include "common/Common.h"
#include "scene/gui/silhouette/SILClippingNode.h"
#include "scene/gui/silhouette/SILSprite.h"


USING_NS_CC;

NS_BEGIN

class CyclicRolling : public Node
{
public:
	static CyclicRolling* create(std::string const& spriteFrameName, std::string const& stencilFrameName, float rollingSpeed);
	bool init(std::string const& spriteFrameName, std::string const& stencilFrameName, float rollingSpeed);

	void update(float dt) override;
	void onEnter() override;

	void setGlobalZOrder(float globalZOrder) override;
	void setVisible(bool visible) override;

	void setPercentage(float percentage);
	float getPercentage() const { return m_percentage; }

private:
	CyclicRolling();
	~CyclicRolling();

	SILClippingNode* m_clippingNode;
	Sprite* m_stencil;
	SILSprite* m_part1;
	SILSprite* m_part2;
	cocos2d::Size m_stencilOriginalSize;
	float m_rollingSpeed;
	float m_percentage;
};


NS_END

#endif // __CYCLIC_ROLLING_H__
