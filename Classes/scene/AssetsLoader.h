#ifndef __ASSETS_LOADER_H__
#define __ASSETS_LOADER_H__

#include "common/Common.h"
#include "game/maps/MapData.h"

NS_BEGIN

////////////////////////////////////////////////////////////////////
// 通用资源
////////////////////////////////////////////////////////////////////

// Widget图集
#define WIDGET_PLIST						RES_IMAGE("widget.plist")
#define WIDGET_9PLIST						RES_IMAGE("widget.9.plist")

// 粒子图集
#define PARTICLE_PLIST						RES_IMAGE("particle.plist")
#define PARTICLE_ATLAS						RES_IMAGE("particle.pvr.ccz")

////////////////////////////////////////////////////////////////////
// 世界资源
////////////////////////////////////////////////////////////////////

// 战斗场景图集
#define BATTLE_PLIST						RES_IMAGE("battle.plist")
#define BATTLE_ATLAS						RES_IMAGE("battle.pvr.ccz")
#define BATTLE_9PLIST						RES_IMAGE("battle.9.plist")
#define BATTLE_9ATLAS						RES_IMAGE("battle.9.pvr.ccz")

// 角色图集
#define ROLE_PLIST							RES_IMAGE("role%d.plist")
#define ROLE_ATLAS							RES_IMAGE("role%d.pvr.ccz")
#define ROLE_COMPONENT_TRANSFORM			RES_IMAGE("role%d.json")

// 角色附件图集
#define ROLE_EXTRAS_PLIST					RES_IMAGE("role_extras.plist")
#define ROLE_EXTRAS_ATLAS					RES_IMAGE("role_extras.pvr.ccz")

// 角色特效图集
#define ROLE_EFFECT_PLIST					RES_IMAGE("role_effect.plist")
#define ROLE_EFFECT_ATLAS					RES_IMAGE("role_effect.pvr.ccz")

// 宝箱图集
#define CHEST_PLIST							RES_IMAGE("chest.plist")
#define CHEST_ATLAS							RES_IMAGE("chest.pvr.ccz")

// 道具图集
#define PROP_PLIST							RES_IMAGE("prop.plist")
#define PROP_ATLAS							RES_IMAGE("prop.pvr.ccz")

// 雪球图集
#define SNOWBALL_PLIST						RES_IMAGE("snowball.plist")
#define SNOWBALL_ATLAS						RES_IMAGE("snowball.pvr.ccz")

// 小地图
#define MINIMAP_IMAGE						MAP_ROOT "images/minimaps/%d.pvr.ccz"


////////////////////////////////////////////////////////////////////
// 功能场景资源
////////////////////////////////////////////////////////////////////

// 功能场景图集
#define FUNC_PLIST							RES_IMAGE("func.plist")
#define FUNC_ATLAS							RES_IMAGE("func.pvr.ccz")
#define FUNC_9PLIST							RES_IMAGE("func.9.plist")
#define FUNC_9ATLAS							RES_IMAGE("func.9.pvr.ccz")
#define FUNC_BG_ATLAS						RES_IMAGE("func_bg.pvr.ccz")
#define FUNC_BG_NARROW_ATLAS				RES_IMAGE("func_bg_narrow.pvr.ccz")

// 英雄Show图集
#define HERO_SHOW_PLIST						RES_IMAGE("show%d.plist")
#define HERO_SHOW_ATLAS						RES_IMAGE("show%d.pvr.ccz")

// 字体
#define BMFONT_PIXCELSTYLE					RES_FONT("pixcelstyle.fnt")

class AssetsLoader
{
	typedef std::vector<std::pair<std::string/* PListFilePath */, std::string /* AtlasFilePath */>> AtlasPathList;
	typedef std::unordered_map<std::string/* PListFilePath */, Texture2D*> TextureMap;
	enum LoadState
	{
		LOAD_STATE_NONE,
		LOAD_STATE_LOADING,
		LOAD_STATE_LOADED,
	};

public:
	static AssetsLoader* instance();

	// 通用资源，程序初始化时加载
	void loadGeneral(std::function<void()> const& loadedCallback);

	//  游戏世界资源
	void loadWorldBasic(std::function<void()> const& loadedCallback);
	void loadWorldGamble(std::function<void()> const& loadedCallback);
	void unloadWorld();
	void unloadWorldBasic();
	void unloadWorldGamble();
	void preloadMapTilesets(MapData* mapData, std::function<void()> const& loadedCallback);
	void unloadMapTilesets(MapData* mapData);

	// 功能页面资源
	void loadFunctional(std::function<void()> const& loadedCallback);
	void unloadFunctional();

private:
    AssetsLoader() : 
		m_worldBasicLoadState(LOAD_STATE_NONE),
		m_worldGambleLoadState(LOAD_STATE_NONE),
		m_functionalLoadState(LOAD_STATE_NONE) { }
	~AssetsLoader() { }

	void loadImage(std::string const& path);
	void loadImagesAsync(AtlasPathList const& paths, std::function<void(TextureMap const&)> const& completedCallback);
	void loadImagesAsync(AtlasPathList const& paths, std::function<void(TextureMap const&)> const& completedCallback, Texture2D::PixelFormat format, int32 currPathIndex, TextureMap& textures);

	void unloadSpriteFramesAndTexture(std::string const& texturePath);
	void loadSpriteFrames(std::string const& plist, Texture2D* texture);
	void loadSpriteFramesAndTexture(std::string const& plist);
    
	LoadState m_worldBasicLoadState;
	LoadState m_worldGambleLoadState;
	LoadState m_functionalLoadState;
};

#define sAssetsLoader AssetsLoader::instance()

NS_END

#endif // __ASSETS_LOADER_H__
