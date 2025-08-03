#include "ItemCooldownProcesser.h"

NS_BEGIN

ItemCooldownProcesser::ItemCooldownProcesser(MyCharacter* owner) :
	m_owner(owner)
{
}

ItemCooldownProcesser::~ItemCooldownProcesser()
{
	m_owner = nullptr;
}

void ItemCooldownProcesser::startCooldown(uint32 itemId, NSTime duration, NSTime remainingTime)
{
	auto it = m_cooldowns.find(itemId);
	if(it != m_cooldowns.end())
		m_cooldowns.erase(it);

	ItemCooldown cooldown(duration, remainingTime);
	m_cooldowns.emplace(itemId, cooldown);
}

bool ItemCooldownProcesser::isReady(uint32 itemId) const
{
	auto it = m_cooldowns.find(itemId);
	return it == m_cooldowns.end();
}

float ItemCooldownProcesser::getElapsedCooldown(uint32 itemId) const
{
	auto it = m_cooldowns.find(itemId);
	if (it != m_cooldowns.end())
	{
		ItemCooldown const& cooldown = (*it).second;
		return cooldown.getElapsed();
	}

	return 0;
}

void ItemCooldownProcesser::update(float delta)
{
	for (auto it = m_cooldowns.begin(); it != m_cooldowns.end();)
	{
		ItemCooldown& cooldown = (*it).second;
		if (!cooldown.update(delta))
			it = m_cooldowns.erase(it);
		else
			++it;
	}
}

NS_END