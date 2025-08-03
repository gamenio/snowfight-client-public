#ifndef __SOUND_MGR_H__
#define __SOUND_MGR_H__

#include "audio/include/AudioEngine.h"
#include "common/Common.h"

USING_NS_CC;
using namespace experimental;

NS_BEGIN

////////////////////////////////////////////////////////////////////
// 通用资源
////////////////////////////////////////////////////////////////////

// 音效
extern const char* SOUND_BUTTON;

////////////////////////////////////////////////////////////////////
// 功能场景资源
////////////////////////////////////////////////////////////////////

// 音效
extern const char* SOUND_SWITCH;
extern const char* SOUND_PLAY;
extern const char* SOUND_UNIT_TICK;
extern const char* SOUND_UNLOCK_HERO;
extern const char* SOUND_STAT_COUNTING;
extern const char* SOUND_DELIVER_COINS;
extern const char* SOUND_COINS_POURING;

// 背景音乐
extern const char* SOUND_FUNC_MUSIC;

////////////////////////////////////////////////////////////////////
// 世界资源
////////////////////////////////////////////////////////////////////

// 音效
extern const char* SOUND_MAGICBEAN_RECEIVE;
extern const char* SOUND_FOOTSTEP;
extern const char* SOUND_PLAYER_DIED;
extern const char* SOUND_SNOWBALL_HIT;
extern const char* SOUND_THROW;
extern const char* SOUND_BUBBLE_POP;
extern const char* SOUND_CHARACTER_POP;
extern const char* SOUND_LEVELUP;
extern const char* SOUND_SHIELD;
extern const char* SOUND_UNSHIELD;
extern const char* SOUND_GIVE_MONEY;
extern const char* SOUND_BATTLE_COUNTDOWN;
extern const char* SOUND_BATTLE_START;
extern const char* SOUND_BATTLE_VICTORY;
extern const char* SOUND_BATTLE_DEFEAT;
extern const char* SOUND_CHEST_OPEN;
extern const char* SOUND_ITEM_STORE;
extern const char* SOUND_ITEM_EQUIP;
extern const char* SOUND_ITEM_DROP;
extern const char* SOUND_COINS_DROP;
extern const char* SOUND_PLAYER_DAMAGED;

// 背景音乐
extern const char* SOUND_COW;
extern const char* SOUND_SHEEP;
extern const char* SOUND_CHICKEN;
extern const char* SOUND_PIG;
extern const char* SOUND_DOG;
extern const char* SOUND_DUCK;
extern const char* SOUND_BATTLE_AMBIENT;

#define SOUNDID_DEFAULT             0

enum SoundGroup
{
	SOUNDGROUP_GENERAL,
	SOUNDGROUP_FUNCTIONAL,
	SOUNDGROUP_WORLD,
    MAX_SOUND_GROUPS
};

struct SoundInfo;

class SoundMgr
{
public:
	static SoundMgr* instance();

	void preloadGeneral(std::function<void()> complete);

	void preloadFunctional(std::function<void()> complete);
	void unloadFunctional();

	void preloadWorld(std::function<void()> complete);
	void unloadWorld();
    
    void unload(std::string const& resName);

    // 播放声音
	// maxAudios: 相同soundId的声音同时播放的最大数量
	// minDelay: 相同soundId的声音之间的最小延迟
    int32 play(std::string const& resName, bool isLoop = false, float volume = 1.0f, int32 soundId = SOUNDID_DEFAULT, int32 maxAudios = 1, float minDelay = 0.f);
	void stop(std::string const& resName, int32 audioId = AudioEngine::INVALID_AUDIO_ID, int32 soundId = SOUNDID_DEFAULT);
	bool isPlaying(std::string const& resName, int32 audioId = AudioEngine::INVALID_AUDIO_ID, int32 soundId = SOUNDID_DEFAULT);
    
    void setVolume(std::string const& resName, float volume, int32 audioId = AudioEngine::INVALID_AUDIO_ID, int32 soundId = SOUNDID_DEFAULT);
    float getVolume(std::string const& resName, int32 audioId = AudioEngine::INVALID_AUDIO_ID, int32 soundId = SOUNDID_DEFAULT);
    void fadeOutVolume(std::string const& resName, float duration, std::function<void(std::string const&)> const& complete = nullptr, int32 audioId = AudioEngine::INVALID_AUDIO_ID, int32 soundId = SOUNDID_DEFAULT);
    void fadeInVolume(std::string const& resName, float duration, std::function<void(std::string const&)> const& complete = nullptr, int32 audioId = AudioEngine::INVALID_AUDIO_ID, int32 soundId = SOUNDID_DEFAULT);
    
	void setEnableBackgroundMusic(bool enable);
	bool isBackgroundMusicEnabled() const { return m_isBackgroundMusicEnabled; }

	void setEnableSoundEffect(bool enable);
	bool isSoundEffectEnabled() const { return m_isSoundEffectEnabled; }

private:
	SoundMgr();
	~SoundMgr();

	void initSounds();
	void preload(SoundGroup group, std::function<void()> complete = nullptr);
	void unload(SoundGroup group);
	
	std::unordered_map<std::string/* ResName */, SoundInfo> m_sounds;
    std::array<int32/* Tasks */, MAX_SOUND_GROUPS> m_preloadTasksOfGroups;
	bool m_isBackgroundMusicEnabled;
	bool m_isSoundEffectEnabled;
};

#define sSoundMgr SoundMgr::instance()

NS_END

#endif // __SOUND_MGR_H__
