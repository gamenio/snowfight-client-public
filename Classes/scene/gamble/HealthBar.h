#ifndef __HEALTH_BAR_H__
#define __HEALTH_BAR_H__

#include "common/Common.h"
#include "scene/gui/silhouette/SILSprite.h"
#include "scene/gui/silhouette/SILLabel.h"
#include "scene/gui/silhouette/SILProgressTimer.h"

USING_NS_CC;

NS_BEGIN


class HealthBar : public Node
{
public:
	HealthBar();
	virtual ~HealthBar();

	static HealthBar* create();
	bool init() override;

	void setGlobalZOrder(float globalZOrder) override;

	void setHealthProgress(int32 value, int32 max);
private:
	SILSprite* m_bgSp;
	SILLabel* m_valueLabel;
	SILProgressTimer* m_healthProg;
};


NS_END


#endif // __HEALTH_BAR_H__