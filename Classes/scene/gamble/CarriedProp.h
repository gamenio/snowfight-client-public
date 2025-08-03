#ifndef __CARRIED_PROP_H__
#define __CARRIED_PROP_H__

#include "common/Common.h"
#include "game/entities/DataCarriedItem.h"
#include "game/WorldListeners.h"
#include "scene/gui/silhouette/SILLabel.h"
#include "scene/gui/silhouette/SILSprite.h"
#include "Gamble.h"


USING_NS_CC;

NS_BEGIN

#define CARRIED_PROP_FRAME_FORMAT				"carried_prop%d.png"

class CarriedProp : public Node, public Gamble
{
public:
	static CarriedProp* createWithData(DataCarriedItem* data);

	CarriedProp();
	~CarriedProp();

	bool initWithData(DataCarriedItem* data);
	
	void update(float delta) override;

	void onActivated() override;
	void onInactivated() override;
	bool isActive() const override { return m_isActive; }

	void cleanUpdateMask() override;
	DataCarriedItem* getData() const override { return m_data; }

	void changeToNormalState();
	void changeToPressedState();

	void startReminderAnimation();

private:
	void layoutElements();

	void stopScaleAnimation();

	DataCarriedItem* m_data;
	bool m_isActive;

	SILSprite* m_mainSp;
	SILLabel* m_nameLabel;
};


NS_END

#endif // __CARRIED_PROP_H__