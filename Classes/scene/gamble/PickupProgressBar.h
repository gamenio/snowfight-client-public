#ifndef __PICKUP_PROGRESS_BAR_H__
#define __PICKUP_PROGRESS_BAR_H__

#include "common/Common.h"
#include "scene/gui/silhouette/SILSprite.h"
#include "scene/gui/silhouette/SILProgressTimer.h"

USING_NS_CC;

NS_BEGIN

class PickupProgressBar : public Node
{
public:
	PickupProgressBar();
	~PickupProgressBar();

	static PickupProgressBar* create();
	bool init() override;

	void onEnter() override;

	void start(NSTime elapsedTime, NSTime duration);
	void stop();

	void setGlobalZOrder(float globalZOrder) override;

private:
	void startProgressAction(float duration, float fromPercentage, float toPercentage);
	void stopProgressAction();

	bool m_isStopped;

	SILSprite* m_progBg;
	SILProgressTimer* m_progTimer;
};


NS_END

#endif // __PICKUP_PROGRESS_BAR_H__
