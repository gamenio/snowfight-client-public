//
// StatDetailPanel.h
// snowfight
//
// Created by Luthier on 2019/5/9.
//

#ifndef __STAT_DETAIL_PANEL_H__
#define __STAT_DETAIL_PANEL_H__

#include "ui/UIScale9Sprite.h"

#include "common/Common.h"
#include "game/ShopMgr.h"
#include "StatValueLabel.h"

USING_NS_CC;


NS_BEGIN

struct StatValues
{
	StatValues() :
		health(0),
		maxHealth(0),
		healthRegenRate(0),
		stamina(0),
		maxStamina(0),
		staminaRegenRate(0),
		damage(0),
		maxDamage(0),
		moveSpeed(0),
		maxMoveSpeed(0),
		attackRange(0),
		maxAttackRange(0)
	{
	}

	int32 health;
	int32 maxHealth;
	float healthRegenRate;
	float maxHealthRegenRate;
	int32 stamina;
	int32 maxStamina;
	float staminaRegenRate;
	float maxStaminaRegenRate;
	int32 damage;
	int32 maxDamage;
	int32 moveSpeed;
	int32 maxMoveSpeed;
	int32 attackRange;
	int32 maxAttackRange;

};

class StatDetailPanel: public LayerColor
{
public:
	StatDetailPanel();
	~StatDetailPanel();

    static StatDetailPanel* create();
    bool init() override;

	void setHeroTemplate(HeroTemplate const* tmpl);
	void updateStats();

	void stopAnimation();
    
private:
	void initHealthGroup();
	void initStaminaGroup();

	void initMiscGroup();
	void initDamageItem();
	void initMoveSpeedItem();
	void initAttackRangeItem();

	void updateStatValues();

	void initProgressAnimations();
	void startProgressAnimations();
	void stopProgressAnimations();
	float startProgressActions(float value, float maximum, StatValueLabel* valueLabel, ProgressTimer* progressTimer);
	void stopProgressActions(Label* valueLabel, ProgressTimer* progressTimer);
	float startProgressAction(float value, float maximum, StatValueLabel* valueLabel);
	void stopProgressAction(Label* valueLabel);

	void playCountingTick(float duration);
	void stopCountingTick();

	void setHeroName(std::string const& name);

	HeroTemplate const* m_template;
	StatValues m_statValues;

	Label* m_nameLabel;

	Node* m_healthGroup;
	StatValueLabel* m_healthValueLabel;
	ProgressTimer* m_healthProg;
	StatValueLabel* m_healthRegenRateLabel;

	Node* m_staminaGroup;
	StatValueLabel* m_staminaValueLabel;
	ProgressTimer* m_staminaProg;
	StatValueLabel* m_staminaRegenRateLabel;

	Node* m_miscGroup;
	StatValueLabel* m_damageValueLabel;
	ProgressTimer* m_damageProg;
	StatValueLabel* m_moveSpeedValueLabel;
	ProgressTimer* m_moveSpeedProg;
	StatValueLabel* m_attackRangeValueLabel;
	ProgressTimer* m_attackRangeProg;
};


NS_END


#endif // __STAT_DETAIL_PANEL_H__

