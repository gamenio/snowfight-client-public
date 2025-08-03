#ifndef __DATA_UNIT_H__
#define __DATA_UNIT_H__


#include "game/entities/DataWorldObject.h"
#include "MovementInfo.h"
#include "StaminaInfo.h"

USING_NS_CC;

NS_BEGIN

enum MovementFlag
{
	MOVEMENT_FLAG_NONE				= 0,
	MOVEMENT_FLAG_WALKING			= 1 << 0,
	MOVEMENT_FLAG_HANDUP			= 1 << 1,
};

// 隐蔽状态
enum ConcealmentState
{
	CONCEALMENT_STATE_EXPOSED,
	CONCEALMENT_STATE_CONCEALING,
	CONCEALMENT_STATE_CONCEALED,
};

enum StaminaFlag
{
	STAMINA_FLAG_NONE				= 0,
	STAMINA_FLAG_ATTACK				= 1 << 0,
	STAMINA_FLAG_CHARGING			= 1 << 1,
};

enum UnitFlag : uint32
{
	UNIT_FLAG_NONE					= 0,
	UNIT_FLAG_DEATH_LOSE_MONEY		= 1 << 0,	// 在单位死亡（生命值为0）时是否失去钱币
	UNIT_FLAG_DAMAGED				= 1 << 1,	// 单位受到伤害
};

// 表情代码
enum SmileyCode
{
	SMILEY_NONE				= 0,
	SMILEY_LAUGH			= 1,
	SMILEY_NAUGHTY			= 2,
	SMILEY_DEVIL			= 3,
	SMILEY_SAD				= 4,
	SMILEY_CRY				= 5,
	SMILEY_ANGRY			= 6,
};

// 英雄ID
enum HeroID
{
	HERO_NONE				= 0,
	HERO_BOY				= 1,
	HERO_LILY				= 2,
	HERO_PENGUIN			= 3,
	HERO_RABBIT				= 4,
	HERO_TEENGIRL			= 5,
	HERO_ELK				= 6,
	HERO_SANTA				= 7,
	HERO_BEAR				= 8,
};
// 默认的英雄ID
#define DEFAULT_HERO_ID			HERO_BOY

// 属性的最小阶段
#define STAT_STAGE_MIN		0

// 属性类型
enum StatType
{
	STAT_NONE			= -1,
    STAT_MAX_HEALTH,
    STAT_HEALTH_REGEN_RATE,
    STAT_ATTACK_RANGE,
    STAT_MOVE_SPEED,
    STAT_MAX_STAMINA,
    STAT_STAMINA_REGEN_RATE,
    STAT_ATTACK_TAKES_STAMINA,
    STAT_DAMAGE,
	STAT_CHARGE_CONSUMES_STAMINA,
	STAT_DEFENSE,
    MAX_STAT_TYPES,
};

typedef std::array<uint8, MAX_STAT_TYPES> StatStageList;

class DataUnit: public DataWorldObject
{
public:
	enum Direction
	{
		UP,
		RIGHT_UP,
		RIGHT,
		RIGHT_DOWN,
		DOWN,
		LEFT_DOWN,
		LEFT,
		LEFT_UP,
		MAX_DIRECTIONS
	};

	DataUnit();
	virtual ~DataUnit();

	virtual void clearFields() override;
	virtual bool readFields(UpdateType updateType, uint32 updateFlags, FieldUpdateMask const& updateMask, DataInputStream* input) override;

	cocos2d::Point const& getPosition() const override { return m_movementInfo.position; }
	void setPosition(cocos2d::Point const& position);

	cocos2d::Point const& getLaunchCenter() const { return m_launchCenter; }
	void setLaunchCenter(cocos2d::Point const& center) { m_launchCenter = center; }
	float getLaunchRadiusInMap() const { return m_launchRadiusInMap; };
	void setLaunchRadiusInMap(float radius) { m_launchRadiusInMap = radius; }

	// 名称
	void setName(std::string const& name);
	std::string const& getName() const { return m_name; }

	// 显示ID
	uint32 getDisplayId() const { return m_displayId; }

	// 等级
	uint8 getLevel() const { return m_level; }
	// 经验值
	int32 getExperience() const { return m_experience; }
	int32 getNextLevelXP() const { return m_nextLevelXP; }

	float getOrientation() const { return m_movementInfo.orientation; }
	uint8 getDirection() const { return m_direction; }
	static uint8 getDirectionWithAngle(float rad);
	// 如果朝向发生改变则返回true
	bool expectFacingToAngle(float rad);

	// 攻击范围。单位：points
	float getAttackRange() const { return m_attackRange; }

	// 移动速度。单位：points/second
	int32 getMoveSpeed() const { return m_moveSpeed; }

	// 生命值
	int32 getHealth() const { return m_health; }
	// 最大生命值
	int32 getMaxHealth() const { return m_maxHealth; }
	bool isAlive() const { return m_isAlive; }
	void setAlive(bool isAlive);

	// 体力值
	void setStamina(int32 stamina);
	int32 getStamina() const { return m_staminaInfo.stamina; }
	void setStaminaInfo(StaminaInfo const& stamina);
	StaminaInfo const& getStaminaInfo() const { return m_staminaInfo; }
	// 体力计数器
	uint32 getStaminaCounter() const { return m_staminaInfo.counter; }
	void increaseStaminaCounter() { ++m_staminaInfo.counter; }
	// 攻击计数器
	void setAttackCounter(uint32 counter) { m_staminaInfo.attackCounter = counter; }
	uint32 getAttackCounter() const { return m_staminaInfo.attackCounter; }
	void resetAttackCounter() { m_staminaInfo.attackCounter = 0; }
	// 消耗体力总和
	void setConsumedStaminaTotal(uint32 total) { m_staminaInfo.consumedStaminaTotal = total; }
	uint32 getConsumedStaminaTotal() const { return m_staminaInfo.consumedStaminaTotal; }
	void increaseConsumedStaminaTotal(uint32 amount) { m_staminaInfo.consumedStaminaTotal += amount; }
	void resetConsumedStaminaTotal() { m_staminaInfo.consumedStaminaTotal = 0; }
	// 体力标记
	void setStaminaFlags(uint32 flags) { m_staminaInfo.flags = flags; }
	void addStaminaFlag(uint32 flag) { if (!this->hasStaminaFlag(flag)) m_staminaInfo.flags |= flag; }
	bool hasStaminaFlag(uint32 flag) const { return (m_staminaInfo.flags & flag) != 0; }
	void clearStaminaFlag(uint32 flag) { if (hasStaminaFlag(flag)) m_staminaInfo.flags &= ~flag; }
	uint32 getStaminaFlags() const { return m_staminaInfo.flags; }
	// 最大体力值
	virtual void setMaxStamina(int32 maxStamina) { m_staminaInfo.maxStamina = maxStamina; }
	int32 getMaxStamina() const { return m_staminaInfo.maxStamina; }
	// 蓄力开始的体力
	void setChargeStartStamina(int32 stamina) { m_staminaInfo.chargeStartStamina = stamina; }
	int32 getChargeStartStamina() const { return m_staminaInfo.chargeStartStamina; }
	// 已蓄力的体力
	void setChargedStamina(int32 stamina) { m_staminaInfo.chargedStamina = stamina; }
	int32 getChargedStamina() const { return m_staminaInfo.chargedStamina; }
	// 抛射体大小比例
	void setProjectileScale(float scale);
	float getProjectileScale() const { return m_projectileScale; }
	// 体力恢复比率。单位：体力恢复的比率/second
	virtual void setStaminaRegenRate(float rate) { m_staminaInfo.staminaRegenRate = rate; }
	float getStaminaRegenRate() const { return m_staminaInfo.staminaRegenRate; }
	// 每次攻击的需要的体力
	int32 getAttackTakesStamina() const { return m_attackTakesStamina; }
	// 蓄力攻击每秒消耗的体力
	int32 getChargeConsumesStamina() const { return m_staminaInfo.chargeConsumesStamina; }

	void setMovementInfo(MovementInfo const& movement);
	MovementInfo const& getMovementInfo() const { return m_movementInfo; }
	uint32 getMovementCounter() const { return m_movementInfo.counter; }
	// 动作标记
	void addMovementFlag(uint32 flag);
	bool hasMovementFlag(uint32 flag) const { return (m_movementInfo.flags & flag) != 0; }
	void clearMovementFlag(uint32 flag);
	uint32 getMovementFlags() const { return m_movementInfo.flags; }
	void setMovementFlags(uint32 flags);

	MovementInfo const& getMoveSegment() const { return m_moveSegment; }

	bool hasUnitFlag(uint32 flag) const { return (m_unitFlags & flag) != 0; }
	void clearUnitFlag(uint32 flag);

	// 表情
	uint16 getSmiley() const { return m_smiley; }

	// 隐蔽状态
	ConcealmentState getConcealmentState() const { return m_concealmentState; }

	// 捡拾持续时间。单位：毫秒
	int32 getPickupDuration() const { return m_pickupDuration; }

	// 魔豆数量
	int32 getMagicBeanCount() const { return m_magicBeanCount; }

	// 击败人数
	int32 getKillCount() const { return m_killCount; }

protected:
	MovementInfo m_movementInfo;
	MovementInfo m_moveSegment;
	uint8 m_direction;

	std::string m_name;
	uint32 m_displayId;

	int32 m_health;
	int32 m_maxHealth;
	bool m_isAlive;

	int32 m_moveSpeed;
	float m_attackRange;

	StaminaInfo m_staminaInfo;
	int32 m_attackTakesStamina;
	float m_projectileScale;

	uint8 m_level;
	int32 m_experience;
	int32 m_nextLevelXP;

	cocos2d::Point m_launchCenter;
	float m_launchRadiusInMap;

	uint32 m_unitFlags;
	uint16 m_smiley;
	ConcealmentState m_concealmentState;
	int32 m_pickupDuration;
	int32 m_magicBeanCount;
	int32 m_killCount;

};

NS_END

#endif //__DATA_UNIT_H__
