#include "AntiHero.h"

#include "common/utils/TimeUtil.h"
#include "common/utils/MathTools.h"
#include "scene/ObjectPools.h"
#include "scene/gamble/Snowball.h"
#include "scene/SoundMgr.h"
#include "scene/TutorialService.h"
#include "game/entities/DataPlayer.h"
#include "game/ObjectMgr.h"
#include "game/World.h"

NS_BEGIN

#define SOUND_MINDELAY_DAMAGED			2.0f

AntiHero::AntiHero() :
	m_damagedLastPlayTime(0)
{
}


AntiHero::~AntiHero()
{
}


AntiHero* AntiHero::createWithData(DataUnit* data)
{
	AntiHero *pRet = new AntiHero();
	if (pRet && pRet->initWithData(data))
	{
		pRet->autorelease();
		return pRet;
	}
	else
	{
		CC_SAFE_DELETE(pRet);
		return nullptr;
	}
}

bool AntiHero::initWithData(DataUnit* data)
{
	if (!Hero::initWithData(data))
		return false;

	return true;
}

void AntiHero::onActivated()
{
	if (this->isActive())
		return;

	Hero::onActivated();

	TutorialService* service = TutorialService::getInstance();
	if (service->isEnabled() && sGameCenter->getLocalPlayer()->isTrainee())
		service->triggerEvent(TUTORIAL_EVENT_ENEMY_FOUND);
}

void AntiHero::onInactivated()
{
	if (!this->isActive())
		return;

	Hero::onInactivated();
}

void AntiHero::hurl()
{
	Hero::hurl();
    
    if(!this->isActive() || !this->getData()->isAlive())
        return;

	if (this->isHurlAnimated())
		return;
    
	this->animateHurl();
	sSoundMgr->play(SOUND_THROW, false, SOUND_VOLUME_ANTIHERO, SOUNDID_ANTIHERO, 2);
}

void AntiHero::update(float delta)
{
	Hero::update(delta);

	if (!this->isActive())
		return;

	this->updateAlive();
	this->updateLevel();
	this->updateReceiveDamage();
}

void AntiHero::applyItem(ItemApplicationTemplate const* appTmpl, int32 duration, int32 remainingTime)
{
	Hero::applyItem(appTmpl, duration, remainingTime);

	switch (appTmpl->visualId)
	{
	case ITEM_VISUAL_SHIELD:
		if (remainingTime >= duration)
		{
			m_heroEffects->play(EFFECT_SHIELDED);
			sSoundMgr->play(SOUND_SHIELD, false, SOUND_VOLUME_ANTIHERO, SOUNDID_ANTIHERO);
		}
		else
			m_heroEffects->play(EFFECT_SHIELDED, true);
		break;
	case ITEM_VISUAL_HEALING:
		m_heroEffects->play(EFFECT_HEALING);
	default:
		break;
	}
}

void AntiHero::unapplyItem(ItemApplicationTemplate const* appTmpl)
{
	switch (appTmpl->visualId)
	{
	case ITEM_VISUAL_SHIELD:
		m_heroEffects->play(EFFECT_SHIELDED, false, true);
		sSoundMgr->play(SOUND_UNSHIELD, false, SOUND_VOLUME_ANTIHERO, SOUNDID_ANTIHERO);
		break;
	default:
		break;
	}

	Hero::unapplyItem(appTmpl);
}

void AntiHero::updateAlive()
{
	if (m_data->hasUpdatedField(CUNIT_FIELD_IS_ALIVE))
	{
		if (!m_data->isAlive())
		{
			this->animateDead();
			sSoundMgr->play(SOUND_PLAYER_DIED, false, SOUND_VOLUME_ANTIHERO, SOUNDID_ANTIHERO);

			if (m_data->hasUnitFlag(UNIT_FLAG_DEATH_LOSE_MONEY))
				m_heroEffects->play(EFFECT_DROP_COINS, false, false, 0.33f);
		}
	}
}

void AntiHero::updateLevel()
{
	if (m_data->hasUpdatedField(CUNIT_FIELD_LEVEL))
	{
		m_heroEffects->play(EFFECT_LEVELUP);
		sSoundMgr->play(SOUND_LEVELUP, false, SOUND_VOLUME_ANTIHERO, SOUNDID_ANTIHERO);
	}
}

void AntiHero::updateReceiveDamage()
{
	if (!m_data->isAlive())
		return;

	if (m_data->hasUpdatedField(CUNIT_FIELD_HEALTH) && m_data->hasUnitFlag(UNIT_FLAG_DAMAGED))
	{
		NSTime currTime = time_util::getUptimeMillis();
		float elapsed = (currTime - m_damagedLastPlayTime) / 1000.f;
		if (elapsed >= SOUND_MINDELAY_DAMAGED)
		{
			int32 ret = sSoundMgr->play(SOUND_PLAYER_DAMAGED, false, SOUND_VOLUME_ANTIHERO, SOUNDID_ANTIHERO, 2);
			if(ret != AudioEngine::INVALID_AUDIO_ID)
				m_damagedLastPlayTime = time_util::getUptimeMillis();
		}
	}
}

bool AntiHero::addFootprint(Point const& pos, float orient)
{
    sSoundMgr->play(SOUND_FOOTSTEP, false, SOUND_VOLUME_ANTIHERO, SOUNDID_ANTIHERO);
    return Hero::addFootprint(pos, orient);
}

NS_END
