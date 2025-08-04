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

// Concealment state
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
	UNIT_FLAG_DEATH_LOSE_MONEY		= 1 << 0,	// Is money lost when a unit dies (health dropped to 0)
	UNIT_FLAG_DAMAGED				= 1 << 1,	// The unit was damaged
};

// Smiley code
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

// Hero ID
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
// Default hero ID
#define DEFAULT_HERO_ID			HERO_BOY

// The minimum stage of statistic
#define STAT_STAGE_MIN		0

// Statistic type
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

	// Name
	void setName(std::string const& name);
	std::string const& getName() const { return m_name; }

	// Display Id
	uint32 getDisplayId() const { return m_displayId; }

	// Level
	uint8 getLevel() const { return m_level; }
	// Experience
	int32 getExperience() const { return m_experience; }
	int32 getNextLevelXP() const { return m_nextLevelXP; }

	float getOrientation() const { return m_movementInfo.orientation; }
	uint8 getDirection() const { return m_direction; }
	static uint8 getDirectionWithAngle(float rad);
	// Return true if the facing has changed.
	bool expectFacingToAngle(float rad);

	// Attack range. Unit: points
	float getAttackRange() const { return m_attackRange; }

	// Moveing speed. Unit: points/second
	int32 getMoveSpeed() const { return m_moveSpeed; }

	// Health
	int32 getHealth() const { return m_health; }
	// Maximum health
	int32 getMaxHealth() const { return m_maxHealth; }
	bool isAlive() const { return m_isAlive; }
	void setAlive(bool isAlive);

	// Stamina
	void setStamina(int32 stamina);
	int32 getStamina() const { return m_staminaInfo.stamina; }
	void setStaminaInfo(StaminaInfo const& stamina);
	StaminaInfo const& getStaminaInfo() const { return m_staminaInfo; }
	// Stamina counter
	uint32 getStaminaCounter() const { return m_staminaInfo.counter; }
	void increaseStaminaCounter() { ++m_staminaInfo.counter; }
	// Attack counter
	void setAttackCounter(uint32 counter) { m_staminaInfo.attackCounter = counter; }
	uint32 getAttackCounter() const { return m_staminaInfo.attackCounter; }
	void resetAttackCounter() { m_staminaInfo.attackCounter = 0; }
	// The total of stamina consumed
	void setConsumedStaminaTotal(uint32 total) { m_staminaInfo.consumedStaminaTotal = total; }
	uint32 getConsumedStaminaTotal() const { return m_staminaInfo.consumedStaminaTotal; }
	void increaseConsumedStaminaTotal(uint32 amount) { m_staminaInfo.consumedStaminaTotal += amount; }
	void resetConsumedStaminaTotal() { m_staminaInfo.consumedStaminaTotal = 0; }
	// Stamina flag
	void setStaminaFlags(uint32 flags) { m_staminaInfo.flags = flags; }
	void addStaminaFlag(uint32 flag) { if (!this->hasStaminaFlag(flag)) m_staminaInfo.flags |= flag; }
	bool hasStaminaFlag(uint32 flag) const { return (m_staminaInfo.flags & flag) != 0; }
	void clearStaminaFlag(uint32 flag) { if (hasStaminaFlag(flag)) m_staminaInfo.flags &= ~flag; }
	uint32 getStaminaFlags() const { return m_staminaInfo.flags; }
	// Maximum stamina
	virtual void setMaxStamina(int32 maxStamina) { m_staminaInfo.maxStamina = maxStamina; }
	int32 getMaxStamina() const { return m_staminaInfo.maxStamina; }
	// Stamina at the start of charging
	void setChargeStartStamina(int32 stamina) { m_staminaInfo.chargeStartStamina = stamina; }
	int32 getChargeStartStamina() const { return m_staminaInfo.chargeStartStamina; }
	// Charged stamina
	void setChargedStamina(int32 stamina) { m_staminaInfo.chargedStamina = stamina; }
	int32 getChargedStamina() const { return m_staminaInfo.chargedStamina; }
	// Projectile size scale
	void setProjectileScale(float scale);
	float getProjectileScale() const { return m_projectileScale; }
	// Stamina regeneration rate. Unit: Stamina regeneration rate / second
	virtual void setStaminaRegenRate(float rate) { m_staminaInfo.staminaRegenRate = rate; }
	float getStaminaRegenRate() const { return m_staminaInfo.staminaRegenRate; }
	// The stamina required for each attack
	int32 getAttackTakesStamina() const { return m_attackTakesStamina; }
	// Stamina consumed per second for charged attack
	int32 getChargeConsumesStamina() const { return m_staminaInfo.chargeConsumesStamina; }

	void setMovementInfo(MovementInfo const& movement);
	MovementInfo const& getMovementInfo() const { return m_movementInfo; }
	uint32 getMovementCounter() const { return m_movementInfo.counter; }
	// Movement flag
	void addMovementFlag(uint32 flag);
	bool hasMovementFlag(uint32 flag) const { return (m_movementInfo.flags & flag) != 0; }
	void clearMovementFlag(uint32 flag);
	uint32 getMovementFlags() const { return m_movementInfo.flags; }
	void setMovementFlags(uint32 flags);

	MovementInfo const& getMoveSegment() const { return m_moveSegment; }

	bool hasUnitFlag(uint32 flag) const { return (m_unitFlags & flag) != 0; }
	void clearUnitFlag(uint32 flag);

	// Smiley
	uint16 getSmiley() const { return m_smiley; }

	// Concealment state
	ConcealmentState getConcealmentState() const { return m_concealmentState; }

	// Pickup duration. Unit: milliseconds
	int32 getPickupDuration() const { return m_pickupDuration; }

	// Number of magic beans
	int32 getMagicBeanCount() const { return m_magicBeanCount; }

	// Number of kills
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
