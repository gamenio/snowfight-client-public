#ifndef __STAMINA_BAR_H__
#define __STAMINA_BAR_H__

#include "common/Common.h"
#include "scene/actions/CyclicRolling.h"
#include "scene/gui/silhouette/SILProgressTimer.h"
#include "scene/gui/silhouette/SILSprite.h"

USING_NS_CC;

NS_BEGIN


class StaminaBar : public Node
{
public:
	StaminaBar();
	virtual ~StaminaBar();

	static StaminaBar* create();
	bool init() override;

	void setGlobalZOrder(float globalZOrder) override;

	void setStaminaProgress(int32 value, int32 max);
	void setCyclicRollingEnabled(bool isEnabled);

private:
	SILSprite* m_bgSp;
	SILProgressTimer* m_staminaProg;
	CyclicRolling* m_staminaCr;
};


NS_END


#endif // __STAMINA_BAR_H__