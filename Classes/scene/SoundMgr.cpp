#include "SoundMgr.h"

#include "common/utils/TimeUtil.h"

NS_BEGIN

const char* SOUND_BUTTON = "button.wav";

const char* SOUND_SWITCH = "switch.wav";
const char* SOUND_PLAY = "play.wav";
const char* SOUND_UNIT_TICK = "unit_tick.wav";
const char* SOUND_UNLOCK_HERO = "unlock_hero.wav";
const char* SOUND_STAT_COUNTING = "stat_counting.wav";
const char* SOUND_DELIVER_COINS = "deliver_coins.wav";
const char* SOUND_COINS_POURING = "coins_pouring.wav";

const char* SOUND_FUNC_MUSIC = "music.wav";

const char* SOUND_MAGICBEAN_RECEIVE = "magicbean_receive.wav";
const char* SOUND_FOOTSTEP = "footstep.wav";
const char* SOUND_PLAYER_DIED = "player_died.wav";
const char* SOUND_SNOWBALL_HIT = "snowball_hit.wav";
const char* SOUND_THROW = "throw.wav";
const char* SOUND_BUBBLE_POP = "bubble_pop.wav";
const char* SOUND_CHARACTER_POP = "character_pop.wav";
const char* SOUND_LEVELUP = "levelup.wav";
const char* SOUND_SHIELD = "shield.wav";
const char* SOUND_UNSHIELD = "unshield.wav";
const char* SOUND_GIVE_MONEY = "give_money.wav";
const char* SOUND_BATTLE_COUNTDOWN = "battle_countdown.wav";
const char* SOUND_BATTLE_START = "battle_start.wav";
const char* SOUND_BATTLE_VICTORY = "battle_victory.wav";
const char* SOUND_BATTLE_DEFEAT = "battle_defeat.wav";
const char* SOUND_CHEST_OPEN = "chest_open.wav";
const char* SOUND_ITEM_STORE = "item_store.wav";
const char* SOUND_ITEM_EQUIP = "item_equip.wav";
const char* SOUND_ITEM_DROP = "item_drop.wav";
const char* SOUND_COINS_DROP = "coins_drop.wav";
const char* SOUND_PLAYER_DAMAGED = "player_damaged.wav";

const char* SOUND_COW = "cow.wav";
const char* SOUND_SHEEP = "sheep.wav";
const char* SOUND_CHICKEN = "chicken.wav";
const char* SOUND_PIG = "pig.wav";
const char* SOUND_DOG = "dog.wav";
const char* SOUND_DUCK = "duck.wav";
const char* SOUND_BATTLE_AMBIENT = "ambient.wav";


#define FADE_VOLUME_OFFSET        0.05f

struct SoundInfo
{
	SoundInfo() :
		filePath(""),
		group(SOUNDGROUP_GENERAL),
		isBackground(false),
		isAutoUnload(true) { }

	SoundInfo(std::string const& resName, SoundGroup _group, bool _isBackground = false, bool _isAutoUnload = true) :
		filePath(""),
		group(_group),
		isBackground(_isBackground),
		isAutoUnload(_isAutoUnload) 
	{ 
		filePath = StringUtils::format("%s%s", SOUND_ROOT, resName.c_str());
	}

	std::string filePath;
	SoundGroup group;
	bool isBackground;
	bool isAutoUnload;
	std::unordered_map<int32/* SoundId */, std::unordered_set<int32/* AudioId */>> audioIdSetMap;
	std::unordered_map<int32/* SoundId */, int32> lastPlayTimeMap;
};

SoundMgr::SoundMgr():
	m_isBackgroundMusicEnabled(true),
	m_isSoundEffectEnabled(false)
{
	this->initSounds();
}

SoundMgr::~SoundMgr()
{
}

void SoundMgr::initSounds()
{
	// General sound effects
	m_sounds[SOUND_BUTTON] = { SOUND_BUTTON, SOUNDGROUP_GENERAL };

	// Function scene sound effects
    m_sounds[SOUND_UNLOCK_HERO] = { SOUND_UNLOCK_HERO, SOUNDGROUP_FUNCTIONAL};
    m_sounds[SOUND_PLAY] = { SOUND_PLAY, SOUNDGROUP_FUNCTIONAL };
    m_sounds[SOUND_UNIT_TICK] = { SOUND_UNIT_TICK, SOUNDGROUP_FUNCTIONAL };
    m_sounds[SOUND_SWITCH] = { SOUND_SWITCH, SOUNDGROUP_FUNCTIONAL };
	m_sounds[SOUND_STAT_COUNTING] = { SOUND_STAT_COUNTING, SOUNDGROUP_FUNCTIONAL };
	m_sounds[SOUND_DELIVER_COINS] = { SOUND_DELIVER_COINS, SOUNDGROUP_FUNCTIONAL };
	m_sounds[SOUND_COINS_POURING] = { SOUND_COINS_POURING, SOUNDGROUP_FUNCTIONAL };

    // Function scene background music
    m_sounds[SOUND_FUNC_MUSIC] = { SOUND_FUNC_MUSIC, SOUNDGROUP_FUNCTIONAL, true, false};
    
	// World sound effects
	m_sounds[SOUND_MAGICBEAN_RECEIVE] = { SOUND_MAGICBEAN_RECEIVE, SOUNDGROUP_WORLD };
	m_sounds[SOUND_FOOTSTEP] = { SOUND_FOOTSTEP, SOUNDGROUP_WORLD };
	m_sounds[SOUND_PLAYER_DIED] = { SOUND_PLAYER_DIED, SOUNDGROUP_WORLD, };
	m_sounds[SOUND_SNOWBALL_HIT] = { SOUND_SNOWBALL_HIT, SOUNDGROUP_WORLD };
    m_sounds[SOUND_THROW] = { SOUND_THROW, SOUNDGROUP_WORLD };
    m_sounds[SOUND_BUBBLE_POP] = { SOUND_BUBBLE_POP, SOUNDGROUP_WORLD };
    m_sounds[SOUND_CHARACTER_POP] = { SOUND_CHARACTER_POP, SOUNDGROUP_WORLD };
    m_sounds[SOUND_LEVELUP] = { SOUND_LEVELUP, SOUNDGROUP_WORLD };
    m_sounds[SOUND_SHIELD] = { SOUND_SHIELD, SOUNDGROUP_WORLD };
    m_sounds[SOUND_UNSHIELD] = { SOUND_UNSHIELD, SOUNDGROUP_WORLD };
	m_sounds[SOUND_GIVE_MONEY] = { SOUND_GIVE_MONEY, SOUNDGROUP_WORLD };
	m_sounds[SOUND_BATTLE_COUNTDOWN] = { SOUND_BATTLE_COUNTDOWN, SOUNDGROUP_WORLD };
	m_sounds[SOUND_BATTLE_START] = { SOUND_BATTLE_START, SOUNDGROUP_WORLD };
	m_sounds[SOUND_BATTLE_VICTORY] = { SOUND_BATTLE_VICTORY, SOUNDGROUP_WORLD };
	m_sounds[SOUND_BATTLE_DEFEAT] = { SOUND_BATTLE_DEFEAT, SOUNDGROUP_WORLD };
	m_sounds[SOUND_CHEST_OPEN] = { SOUND_CHEST_OPEN, SOUNDGROUP_WORLD };
	m_sounds[SOUND_ITEM_STORE] = { SOUND_ITEM_STORE, SOUNDGROUP_WORLD };
    m_sounds[SOUND_ITEM_EQUIP] = { SOUND_ITEM_EQUIP, SOUNDGROUP_WORLD };
	m_sounds[SOUND_ITEM_DROP] = { SOUND_ITEM_DROP, SOUNDGROUP_WORLD };
	m_sounds[SOUND_COINS_DROP] = { SOUND_COINS_DROP, SOUNDGROUP_WORLD };
    m_sounds[SOUND_PLAYER_DAMAGED] = { SOUND_PLAYER_DAMAGED, SOUNDGROUP_WORLD };

	// World background music
	m_sounds[SOUND_COW] = { SOUND_COW, SOUNDGROUP_WORLD, true };
    m_sounds[SOUND_SHEEP] = { SOUND_SHEEP, SOUNDGROUP_WORLD, true };
    m_sounds[SOUND_CHICKEN] = { SOUND_CHICKEN, SOUNDGROUP_WORLD, true };
    m_sounds[SOUND_PIG] = { SOUND_PIG, SOUNDGROUP_WORLD, true };
    m_sounds[SOUND_DOG] = { SOUND_DOG, SOUNDGROUP_WORLD, true };
    m_sounds[SOUND_DUCK] = { SOUND_DUCK, SOUNDGROUP_WORLD, true };
	m_sounds[SOUND_BATTLE_AMBIENT] = { SOUND_BATTLE_AMBIENT, SOUNDGROUP_WORLD, true };
}

void SoundMgr::preload(SoundGroup group, std::function<void()> complete)
{
	// Count the number of preloaded tasks in the same group.
	for (auto it = m_sounds.begin(); it != m_sounds.end(); ++it)
	{
		SoundInfo& info = (*it).second;
		if (info.group == group)
		{
			++m_preloadTasksOfGroups[(int32)info.group];
		}
	}

	for (auto it = m_sounds.begin(); it != m_sounds.end(); ++it)
	{
		SoundInfo& info = (*it).second;
		if (info.group == group)
		{
            if(!info.audioIdSetMap.empty())
                CCLOG("SoundMgr: '%s' may have been preloaded.", info.filePath.c_str());
            
            AudioEngine::preload(info.filePath, [&, complete](bool isSuccess)
            {
                //CCLOG("SoundMgr: PRELOAD filename: %s success: %d",info.filePath, isSuccess);
                --m_preloadTasksOfGroups[(int32)info.group];
                if(m_preloadTasksOfGroups[(int32)info.group] <= 0)
                {
					if (complete)
					{
						complete();
					}
                      
                }
            });
		}
	}
}


void SoundMgr::unload(SoundGroup group)
{
	for (auto it = m_sounds.begin(); it != m_sounds.end(); ++it)
	{
		SoundInfo& info = (*it).second;
		if (info.isAutoUnload && info.group == group)
        {
			for (auto idSetIt = info.audioIdSetMap.begin(); idSetIt != info.audioIdSetMap.end();)
			{
				std::unordered_set<int32>& audioIds = (*idSetIt).second;
				for (auto it = audioIds.begin(); it != audioIds.end();)
				{
					AudioEngine::stop(*it);
					it = audioIds.erase(it);
				}
				idSetIt = info.audioIdSetMap.erase(idSetIt);
			}

			AudioEngine::uncache(info.filePath);
		}
	}
}

SoundMgr* SoundMgr::instance()
{
	static SoundMgr instance;
	return &instance;
}

void SoundMgr::preloadGeneral(std::function<void()> complete)
{
	this->preload(SOUNDGROUP_GENERAL, complete);
}

void SoundMgr::preloadFunctional(std::function<void()> complete)
{
	this->preload(SOUNDGROUP_FUNCTIONAL, complete);
}

void SoundMgr::unloadFunctional()
{
	this->unload(SOUNDGROUP_FUNCTIONAL);
}


void SoundMgr::preloadWorld(std::function<void()> complete)
{
	this->preload(SOUNDGROUP_WORLD, complete);
}

void SoundMgr::unloadWorld()
{
	this->unload(SOUNDGROUP_WORLD);
}


void SoundMgr::unload(std::string const& resName)
{
    auto it = m_sounds.find(resName);
    if(it != m_sounds.end())
    {
        SoundInfo& info = (*it).second;
        for (auto idSetIt = info.audioIdSetMap.begin(); idSetIt != info.audioIdSetMap.end();)
        {
            std::unordered_set<int32>& audioIds = (*idSetIt).second;
            for (auto it = audioIds.begin(); it != audioIds.end();)
            {
                AudioEngine::stop(*it);
                it = audioIds.erase(it);
            }
            idSetIt = info.audioIdSetMap.erase(idSetIt);
        }
        
        AudioEngine::uncache(info.filePath);
    }
}

int32 SoundMgr::play(std::string const& resName, bool isLoop, float volume, int32 soundId, int32 maxAudios, float minDelay)
{
	int32 ret = AudioEngine::INVALID_AUDIO_ID;

	auto it = m_sounds.find(resName);
	if (it != m_sounds.end())
	{
		SoundInfo& info = (*it).second;
		bool isPlayable = false;
		if (info.isBackground)
			isPlayable = m_isBackgroundMusicEnabled;
		else
			isPlayable = m_isSoundEffectEnabled;

		if (isPlayable)
		{
			std::unordered_set<int32>& audioIds = info.audioIdSetMap[soundId];
			if (int32(audioIds.size()) < maxAudios)
			{
				NSTime currTime = time_util::getUptimeMillis();
				float elapsed = (currTime - info.lastPlayTimeMap[soundId]) / 1000.f;
				if (elapsed >= minDelay)
				{
					ret = AudioEngine::play2d(info.filePath, isLoop, volume);
					//CCLOG("SoundMgr: PLAY start. res_name: %s sound_id: %d audio_id: %d state: %d", resName.c_str(), soundId, ret, (int32)AudioEngine::getState(ret));
					if (ret != AudioEngine::INVALID_AUDIO_ID)
					{
						info.lastPlayTimeMap[soundId] = time_util::getUptimeMillis();
						audioIds.insert(ret);
						AudioEngine::setFinishCallback(ret, [&info, soundId](int audioId, std::string const& filename) {
							NS_ASSERT(info.filePath == filename);
							std::unordered_set<int32>& audioIds = info.audioIdSetMap[soundId];
							audioIds.erase(audioId);
							//CCLOG("SoundMgr: PLAY finish. sound_id: %d audio_id: %d filename: %s", soundId, audioId, filename.c_str());
						});
					}
				}
				//else
				//{
				//	CCLOG("SoundMgr: PLAY rejected (limited minimum delay). res_name: %s sound_id: %d minDelay: %f", resName.c_str(), soundId, minDelay);
				//}
			}
			//else
			//{
			//	CCLOG("SoundMgr: PLAY rejected (limited maximum audios). res_name: %s sound_id: %d maxAudios: %d", resName.c_str(), soundId, maxAudios);
			//}
		}
	}

	return ret;
}

void SoundMgr::stop(std::string const& resName, int32 audioId, int32 soundId)
{
	auto it = m_sounds.find(resName);
	if (it != m_sounds.end())
	{
		SoundInfo& info = (*it).second;
        
        auto idSetIt = info.audioIdSetMap.find(soundId);
        if(idSetIt != info.audioIdSetMap.end())
        {
            std::unordered_set<int32>& audioIds = (*idSetIt).second;
            if(audioId == AudioEngine::INVALID_AUDIO_ID)
            {
                for(auto it = audioIds.begin(); it != audioIds.end();)
                {
                    AudioEngine::stop(*it);
                    it = audioIds.erase(it);
                }
            }
            else
            {
                auto it = audioIds.find(audioId);
                if(it != audioIds.end())
                {
                    AudioEngine::stop(*it);
                    audioIds.erase(it);
                }
            }
            if(audioIds.empty())
                info.audioIdSetMap.erase(idSetIt);
        }
	}
}

bool SoundMgr::isPlaying(std::string const& resName, int32 audioId, int32 soundId)
{
	bool isPlaying = false;

	auto it = m_sounds.find(resName);
	if (it != m_sounds.end())
	{
		SoundInfo& info = (*it).second;

		auto idSetIt = info.audioIdSetMap.find(soundId);
		if (idSetIt != info.audioIdSetMap.end())
		{
			std::unordered_set<int32>& audioIds = (*idSetIt).second;
			if (audioId == AudioEngine::INVALID_AUDIO_ID)
			    isPlaying = !audioIds.empty();
			else
                isPlaying = audioIds.find(audioId) != audioIds.end();
		}
	}

	return isPlaying;
}

void SoundMgr::setVolume(std::string const& resName, float volume, int32 audioId, int32 soundId)
{
    auto it = m_sounds.find(resName);
    if (it != m_sounds.end())
    {
        SoundInfo& info = (*it).second;
        
        auto idSetIt = info.audioIdSetMap.find(soundId);
        if(idSetIt != info.audioIdSetMap.end())
        {
            std::unordered_set<int32>& audioIds = (*idSetIt).second;
            if(audioId == AudioEngine::INVALID_AUDIO_ID)
            {
                for(auto it = audioIds.begin(); it != audioIds.end(); ++it)
                    AudioEngine::setVolume(*it, volume);
            }
            else
            {
                auto it = audioIds.find(audioId);
                if(it != audioIds.end())
                    AudioEngine::setVolume(*it, volume);
            }
        }
    }
}

float SoundMgr::getVolume(std::string const& resName, int32 audioId, int32 soundId)
{
    float volume = 0.0f;
    
    auto it = m_sounds.find(resName);
    if (it != m_sounds.end())
    {
        SoundInfo& info = (*it).second;
        
        auto idSetIt = info.audioIdSetMap.find(soundId);
        if(idSetIt != info.audioIdSetMap.end())
        {
            std::unordered_set<int32>& audioIds = (*idSetIt).second;
            if(audioId == AudioEngine::INVALID_AUDIO_ID)
            {
				for (auto it = audioIds.begin(); it != audioIds.end(); ++it)
				{
#undef ERROR
					if(AudioEngine::getState(*it) != AudioEngine::AudioState::ERROR)
						volume = MAX(volume, AudioEngine::getVolume(*it));
				}
            }
            else
            {
                auto it = audioIds.find(audioId);
                if(it != audioIds.end())
                    volume = AudioEngine::getVolume(*it);
            }
        }
    }
    return volume;
}

void SoundMgr::fadeOutVolume(std::string const& resName, float duration, std::function<void(std::string const&)> const& complete, int32 audioId, int32 soundId)
{
    Scheduler* scheduler = Director::getInstance()->getScheduler();
    std::string key = StringUtils::format("%s%d%d", resName.c_str(), audioId, soundId);
    if(scheduler->isScheduled(key, this))
        scheduler->unschedule(key, this);
    
	float currVol = this->getVolume(resName, audioId, soundId);
	if (currVol <= 0.f)
	{
		if (complete)
			complete(resName);
		return;
	}

    float interval = duration / (currVol / FADE_VOLUME_OFFSET);
    scheduler->schedule([this, resName, complete, audioId, soundId, key, currVol](float dt) mutable {
        if(currVol <= 0.f)
        {
            Director::getInstance()->getScheduler()->unschedule(key, this);
            if(complete)
                complete(resName);
        }
        else
        {
            currVol -= FADE_VOLUME_OFFSET;
            currVol = MAX(currVol, 0.0f);
            this->setVolume(resName, currVol, audioId, soundId);
        }
    }, this, interval, false, key);
}

void SoundMgr::fadeInVolume(std::string const& resName, float duration, std::function<void(std::string const&)> const& complete, int32 audioId, int32 soundId)
{
    Scheduler* scheduler = Director::getInstance()->getScheduler();
    std::string key = StringUtils::format("%s%d%d", resName.c_str(), audioId, soundId);
    if(scheduler->isScheduled(key, this))
        scheduler->unschedule(key, this);
    
    float currVol = this->getVolume(resName, audioId, soundId);
    if(currVol >= 1.0f)
        return;
    
    float interval = duration / ((1.0f - currVol) / FADE_VOLUME_OFFSET);
    scheduler->schedule([this, resName, complete, audioId, soundId, key, currVol](float dt) mutable {
        if(currVol >= 1.f)
        {
            Director::getInstance()->getScheduler()->unschedule(key, this);
            if(complete)
                complete(resName);
        }
        else
        {
            currVol += FADE_VOLUME_OFFSET;
            currVol = MIN(currVol, 1.0f);
            this->setVolume(resName, currVol, audioId, soundId);
        }
    }, this, interval, false, key);
}

void SoundMgr::setEnableBackgroundMusic(bool enable)
{
	if (m_isBackgroundMusicEnabled != enable)
	{
		if (!enable)
		{
			for (auto it = m_sounds.begin(); it != m_sounds.end(); ++it)
			{
				SoundInfo& info = (*it).second;
				if (info.isBackground)
				{
					for (auto idSetIt = info.audioIdSetMap.begin(); idSetIt != info.audioIdSetMap.end();)
					{
						std::unordered_set<int32>& audioIds = (*idSetIt).second;
						for (auto it = audioIds.begin(); it != audioIds.end();)
						{
							AudioEngine::stop(*it);
							it = audioIds.erase(it);
						}
						idSetIt = info.audioIdSetMap.erase(idSetIt);
					}
				}
			}
		}


		m_isBackgroundMusicEnabled = enable;
	}
}

void SoundMgr::setEnableSoundEffect(bool enable)
{
	if (m_isSoundEffectEnabled != enable)
	{
		if (!enable)
		{
			for (auto it = m_sounds.begin(); it != m_sounds.end(); ++it)
			{
				SoundInfo& info = (*it).second;
				if (!info.isBackground)
				{
					for (auto idSetIt = info.audioIdSetMap.begin(); idSetIt != info.audioIdSetMap.end();)
					{
						std::unordered_set<int32>& audioIds = (*idSetIt).second;
						for (auto it = audioIds.begin(); it != audioIds.end();)
						{
							AudioEngine::stop(*it);
							it = audioIds.erase(it);
						}
						idSetIt = info.audioIdSetMap.erase(idSetIt);
					}
				}
			}
		}


		m_isSoundEffectEnabled = enable;
	}
}



NS_END

