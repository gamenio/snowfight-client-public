#include "AssetsLoader.h"

#include "2d/CCFontAtlasCache.h"

#include "common/utils/TimeUtil.h"
#include "common/Machine.h"
#include "gamble/Snowball.h"
#include "gamble/Footprint.h"
#include "game/LocaleMgr.h"
#include "game/ShopMgr.h"
#include "scene/GLProgramManager.h"
#include "SoundMgr.h"
#include "gamble/ComponentTransformCache.h"


NS_BEGIN

AssetsLoader* AssetsLoader::instance()
{
	static AssetsLoader instance;
	return &instance;
}

void AssetsLoader::loadGeneral(std::function<void()> const& loadedCallback)
{
    // 加载本地化资源
	sLocaleMgr->loadGeneral();

	// 加载GlPrograms
    GLProgramManager::getInstance()->loadGLPrograms();

	// 加载Widget图集
	this->loadSpriteFramesAndTexture(WIDGET_PLIST);
	this->loadSpriteFramesAndTexture(WIDGET_9PLIST);

	// 粒子图集
	this->loadSpriteFramesAndTexture(PARTICLE_PLIST);

	sSoundMgr->preloadGeneral([loadedCallback]() {
		if (loadedCallback)
			loadedCallback();
	});
}

void AssetsLoader::loadWorldBasic(std::function<void()> const& loadedCallback)
{
	if (m_worldBasicLoadState == LOAD_STATE_LOADED)
	{
		if (loadedCallback)
			loadedCallback();
	}
	else if (m_worldBasicLoadState == LOAD_STATE_NONE)
	{
		m_worldBasicLoadState = LOAD_STATE_LOADING;

		sLocaleMgr->loadWorld();

		AtlasPathList atlasPaths;
		// 战斗场景图集
		atlasPaths.emplace_back(BATTLE_PLIST, BATTLE_ATLAS);
		atlasPaths.emplace_back(BATTLE_9PLIST, BATTLE_9ATLAS);
		this->loadImagesAsync(atlasPaths, [loadedCallback, this](TextureMap const& textures) {
			for (auto it = textures.begin(); it != textures.end(); ++it)
			{
				this->loadSpriteFrames((*it).first, (*it).second);
			}

			sSoundMgr->preloadWorld([this, loadedCallback]() {
				if (loadedCallback)
					loadedCallback();

				m_worldBasicLoadState = LOAD_STATE_LOADED;
			});
		});
	}
}

void AssetsLoader::loadWorldGamble(std::function<void()> const& loadedCallback)
{
	if (m_worldGambleLoadState == LOAD_STATE_LOADED)
	{
		if (loadedCallback)
			loadedCallback();
	}
	else if(m_worldGambleLoadState == LOAD_STATE_NONE)
	{
		m_worldGambleLoadState = LOAD_STATE_LOADING;

		AtlasPathList atlasPaths;
		// 角色附件图集
		atlasPaths.emplace_back(ROLE_EXTRAS_PLIST, ROLE_EXTRAS_ATLAS);
		// 角色特效图集
		atlasPaths.emplace_back(ROLE_EFFECT_PLIST, ROLE_EFFECT_ATLAS);
		// 雪球图集
		atlasPaths.emplace_back(SNOWBALL_PLIST, SNOWBALL_ATLAS);
		// 宝箱图集
		atlasPaths.emplace_back(CHEST_PLIST, CHEST_ATLAS);
		// 道具图集
		atlasPaths.emplace_back(PROP_PLIST, PROP_ATLAS);
		// 角色图集
		auto heroTmplList = sShopMgr->getHeroTemplateList();
		int32 nHeroTmpl = (int32)heroTmplList->size();
		for (int32 i = 0; i < nHeroTmpl; i++)
		{
			HeroTemplate const& tmpl = (*heroTmplList)[i];
			atlasPaths.emplace_back(StringUtils::format(ROLE_PLIST, tmpl.id), StringUtils::format(ROLE_ATLAS, tmpl.id));
			// 角色部件变换信息
			ComponentTransformCache::getInstance()->addComponentTransfromsWithFile(StringUtils::format(ROLE_COMPONENT_TRANSFORM, tmpl.id));
		}
		this->loadImagesAsync(atlasPaths, [this, loadedCallback, heroTmplList](TextureMap const& textures) {
			for (auto it = textures.begin(); it != textures.end(); ++it)
			{
				this->loadSpriteFrames((*it).first, (*it).second);
			}

			if (loadedCallback)
				loadedCallback();

			m_worldGambleLoadState = LOAD_STATE_LOADED;
		});
	}
}

void AssetsLoader::preloadMapTilesets(MapData* mapData, std::function<void()> const& loadedCallback)
{
	auto& tilesets = mapData->getMapInfo()->getTilesets();

	AtlasPathList atlasPaths;
	for (auto iter = tilesets.crbegin(), iterCrend = tilesets.crend(); iter != iterCrend; ++iter)
	{
		TMXTilesetInfo* tilesetInfo = *iter;
		if (tilesetInfo)
			atlasPaths.emplace_back("", tilesetInfo->_sourceImage);
	}

	atlasPaths.emplace_back("", StringUtils::format(MINIMAP_IMAGE, mapData->getMapId()));

	this->loadImagesAsync(atlasPaths,  [loadedCallback](TextureMap const& textures) {
		if (loadedCallback)
			loadedCallback();
	});
}

void AssetsLoader::unloadMapTilesets(MapData* mapData)
{
	auto& tilesets = mapData->getMapInfo()->getTilesets();
	for (auto iter = tilesets.crbegin(), iterCrend = tilesets.crend(); iter != iterCrend; ++iter)
	{
		TMXTilesetInfo* tilesetInfo = *iter;
		if (tilesetInfo)
			Director::getInstance()->getTextureCache()->removeTextureForKey(tilesetInfo->_sourceImage);
	}

	Director::getInstance()->getTextureCache()->removeTextureForKey(StringUtils::format(MINIMAP_IMAGE, mapData->getMapId()));
}

void AssetsLoader::unloadWorld()
{
	this->unloadWorldBasic();
	this->unloadWorldGamble();
}

void AssetsLoader::unloadWorldBasic()
{
	NS_ASSERT_LOG(m_worldBasicLoadState != LOAD_STATE_LOADING, "Unable to unload world basic assets that is loading.");
	if (m_worldBasicLoadState != LOAD_STATE_LOADED)
		return;

	sLocaleMgr->unloadWorld();
	sSoundMgr->unloadWorld();

	// 战斗场景图集
	this->unloadSpriteFramesAndTexture(BATTLE_ATLAS);
	this->unloadSpriteFramesAndTexture(BATTLE_9ATLAS);

	m_worldBasicLoadState = LOAD_STATE_NONE;
}

void AssetsLoader::unloadWorldGamble()
{
	NS_ASSERT_LOG(m_worldGambleLoadState != LOAD_STATE_LOADING, "Unable to unload world gamble assets that is loading.");
	if (m_worldGambleLoadState != LOAD_STATE_LOADED)
		return;

	// 清理角色图集
	auto heroTmplList = sShopMgr->getHeroTemplateList();
	int32 nHeroTmpl = (int32)heroTmplList->size();
	for (int32 i = 0; i < nHeroTmpl; i++)
	{
		HeroTemplate const& tmpl = (*heroTmplList)[i];
		std::string atlas = StringUtils::format(ROLE_ATLAS, tmpl.id);
		this->unloadSpriteFramesAndTexture(atlas);
		// 角色部件变换信息
		ComponentTransformCache::getInstance()->removeComponentTransfromsFromFile(StringUtils::format(ROLE_COMPONENT_TRANSFORM, tmpl.id));
	}

	// 清理角色附件图集
	this->unloadSpriteFramesAndTexture(ROLE_EXTRAS_ATLAS);

	// 清理角色特效图集
	this->unloadSpriteFramesAndTexture(ROLE_EFFECT_ATLAS);

	// 清理雪球图集
	this->unloadSpriteFramesAndTexture(SNOWBALL_ATLAS);

	// 清理宝箱图集
	this->unloadSpriteFramesAndTexture(CHEST_ATLAS);

	// 清理道具图集
	this->unloadSpriteFramesAndTexture(PROP_ATLAS);

	m_worldGambleLoadState = LOAD_STATE_NONE;
}

void AssetsLoader::loadFunctional(std::function<void()> const& loadedCallback)
{
	if (m_functionalLoadState == LOAD_STATE_LOADED)
	{
		if (loadedCallback)
			loadedCallback();
	}
	else if(m_functionalLoadState == LOAD_STATE_NONE)
	{
		m_functionalLoadState = LOAD_STATE_LOADING;

		// 加载背景
		if (Machine::instance()->isNarrowScreen())
			this->loadImage(FUNC_BG_NARROW_ATLAS);
		else
			this->loadImage(FUNC_BG_ATLAS);

		// 加载场景图集
		this->loadSpriteFramesAndTexture(FUNC_PLIST);
		this->loadSpriteFramesAndTexture(FUNC_9PLIST);

		// 加载英雄Show图集
		auto heroTmplList = sShopMgr->getHeroTemplateList();
		int32 nHeroTmpl = (int32)heroTmplList->size();
		for (int32 i = 0; i < nHeroTmpl; i++)
		{
			HeroTemplate const& tmpl = (*heroTmplList)[i];
			std::string plist = StringUtils::format(HERO_SHOW_PLIST, tmpl.id);
			this->loadSpriteFramesAndTexture(plist);
		}

		// 加载字体
		FontAtlasCache::getFontAtlasFNT(BMFONT_PIXCELSTYLE);

		sSoundMgr->preloadFunctional([&, loadedCallback]() {
			if (loadedCallback)
				loadedCallback();
			m_functionalLoadState = LOAD_STATE_LOADED;
		});
	}
}

void AssetsLoader::unloadFunctional()
{
	NS_ASSERT_LOG(m_functionalLoadState != LOAD_STATE_LOADING, "Unable to unload functional assets that is loading.");
	if (m_functionalLoadState != LOAD_STATE_LOADED)
		return;

	sSoundMgr->unloadFunctional();

	// 清理场景图集
	this->unloadSpriteFramesAndTexture(FUNC_ATLAS);
	this->unloadSpriteFramesAndTexture(FUNC_9ATLAS);
	Director::getInstance()->getTextureCache()->removeTextureForKey(FUNC_BG_ATLAS);


	// 清理英雄Show图集
	auto heroTmplList = sShopMgr->getHeroTemplateList();
	int32 nHeroTmpl = (int32)heroTmplList->size();
	for (int32 i = 0; i < nHeroTmpl; i++)
	{
		HeroTemplate const& tmpl = (*heroTmplList)[i];
		std::string atlas = StringUtils::format(HERO_SHOW_ATLAS, tmpl.id);
		this->unloadSpriteFramesAndTexture(atlas);
	}

	// 清理字体
	FontAtlasCache::unloadFontAtlasTTF(BMFONT_PIXCELSTYLE);

	m_functionalLoadState = LOAD_STATE_NONE;

}

void AssetsLoader::loadImage(std::string const& path)
{
	const Texture2D::PixelFormat currentPixelFormat = Texture2D::getDefaultAlphaPixelFormat();
	Texture2D::setDefaultAlphaPixelFormat(Texture2D::PixelFormat::AUTO);
	Director::getInstance()->getTextureCache()->addImage(path);;
	Texture2D::setDefaultAlphaPixelFormat(currentPixelFormat);
}


void AssetsLoader::loadImagesAsync(AtlasPathList const& paths, std::function<void(TextureMap const&)> const& completedCallback)
{
	TextureMap textures;
	this->loadImagesAsync(paths, completedCallback, Texture2D::getDefaultAlphaPixelFormat(), 0, textures);
}

void AssetsLoader::loadImagesAsync(AtlasPathList const& paths, std::function<void(TextureMap const&)> const& completedCallback, Texture2D::PixelFormat format, int32 currPathIndex, TextureMap& textures)
{
	if (currPathIndex == paths.size())
	{
		Texture2D::setDefaultAlphaPixelFormat(format);
		if (completedCallback)
			completedCallback(textures);
		return;
	}

	if (currPathIndex == 0)
		Texture2D::setDefaultAlphaPixelFormat(Texture2D::PixelFormat::AUTO);

	auto const& path = paths[currPathIndex];
	Director::getInstance()->getTextureCache()->addImageAsync(path.second, [this, textures, paths, currPathIndex, format, completedCallback](Texture2D* tex) mutable {
		if(tex)
			textures[paths[currPathIndex].first] = tex;
		++currPathIndex;
		this->loadImagesAsync(paths, completedCallback, format, currPathIndex, textures);
	});
}


void AssetsLoader::unloadSpriteFramesAndTexture(std::string const& texturePath)
{
	auto spriteFrameCache = SpriteFrameCache::getInstance();
	auto textureCache = Director::getInstance()->getTextureCache();
	spriteFrameCache->removeSpriteFramesFromTexture(textureCache->getTextureForKey(texturePath));
	textureCache->removeTextureForKey(texturePath);
}

void AssetsLoader::loadSpriteFrames(std::string const& plist, Texture2D* texture)
{
	auto spriteFrameCache = SpriteFrameCache::getInstance();
	if (spriteFrameCache->isSpriteFramesWithFileLoaded(plist))
	{
		std::string plistContent; 
		FileUtils::Status status = FileUtils::getInstance()->getContents(plist, &plistContent);
		NS_UNUSED_VARIABLE(status);
		NS_ASSERT(status == FileUtils::Status::OK);
		spriteFrameCache->addSpriteFramesWithFileContent(plistContent, texture);
	}
	else
		spriteFrameCache->addSpriteFramesWithFile(plist, texture);
}

void snowfight::AssetsLoader::loadSpriteFramesAndTexture(std::string const& plist)
{
	auto spriteFrameCache = SpriteFrameCache::getInstance();
	if (spriteFrameCache->isSpriteFramesWithFileLoaded(plist))
	{
		const Texture2D::PixelFormat currentPixelFormat = Texture2D::getDefaultAlphaPixelFormat();
		Texture2D::setDefaultAlphaPixelFormat(Texture2D::PixelFormat::AUTO);
		bool ret = spriteFrameCache->reloadTexture(plist);
		Texture2D::setDefaultAlphaPixelFormat(currentPixelFormat);

		NS_UNUSED_VARIABLE(ret);
		NS_ASSERT(ret);
	}
	else
		spriteFrameCache->addSpriteFramesWithFile(plist);
}

NS_END