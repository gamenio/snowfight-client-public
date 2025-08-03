#ifndef __ITEM_COOLDOWN_PROCESSER_H__
#define __ITEM_COOLDOWN_PROCESSER_H__

#include "common/Common.h"
#include "ItemCooldown.h"
#include "game/entities/DataCarriedItem.h"

NS_BEGIN

class MyCharacter;

class ItemCooldownProcesser
{
public:
	ItemCooldownProcesser(MyCharacter* owner);
	~ItemCooldownProcesser();

	void startCooldown(uint32 itemId, NSTime duration, NSTime remainingTime = 0);

	bool isReady(uint32 itemId) const;
	float getElapsedCooldown(uint32 itemId) const;

	void removeAll() { m_cooldowns.clear(); }

	void update(float delta);

private:
	MyCharacter* m_owner;
	std::unordered_map<uint32/* ItemID */, ItemCooldown> m_cooldowns;
};

NS_END

#endif // __ITEM_COOLDOWN_PROCESSER_H__
