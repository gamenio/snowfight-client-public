#ifndef __NAMEPLATE_H__
#define __NAMEPLATE_H__

#include "common/Common.h"
#include "LevelPlate.h"
#include "HealthBar.h"
#include "StaminaBar.h"
#include "scene/gui/silhouette/SILSprite.h"
#include "scene/gui/silhouette/SILLabel.h"

USING_NS_CC;

NS_BEGIN

class DataUnit;

// A character's nameplate, which is used to display information such as health value, player name, and so on.
class Nameplate : public Node
{
public:
	Nameplate();
	virtual ~Nameplate();

	static Nameplate* create(DataUnit* data);
	bool init(DataUnit* data);

	void updateElements(bool force);
	void setGlobalZOrder(float globalZOrder) override;

	void setStaminaCyclicRollingEnabled(bool isEnabled);

private:
	void initElements();
	void layoutElements();

	void updateHealth(bool force);
	void updateName(bool force);
	void updateStamina(bool force);
	void updateLevel(bool force);
	void updateAIAction(bool force);

	DataUnit* m_data;

	bool m_isLayoutDirty;
	SILLabel* m_nameLabel;
	HealthBar* m_healthBar;
	StaminaBar* m_staminaBar;
	LevelPlate* m_levelPlate;
	SILLabel* m_aiActionLabel;

	DrawNode* m_debugDraw;
};

NS_END

#endif // __NAMEPLATE_H__
