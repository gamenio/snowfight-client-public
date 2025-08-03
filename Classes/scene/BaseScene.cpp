#include "BaseScene.h"

namespace std
{
	size_t hash<CachedTexture>::operator()(CachedTexture const& key) const
	{
		return hash<std::string>()(key.getPath());
	}
}

NS_BEGIN

CachedTexture::CachedTexture(std::string const& resName, Texture2D::PixelFormat format, bool isLocale)
{
	if (isLocale)
		m_path = sLocaleMgr->getImagePath(resName);
	else
		m_path = StringUtils::format("%s%s", IMAGE_ROOT, resName.c_str());

	const Texture2D::PixelFormat currentPixelFormat = Texture2D::getDefaultAlphaPixelFormat();
	Texture2D::setDefaultAlphaPixelFormat(format);
	Director::getInstance()->getTextureCache()->addImage(m_path);
	Texture2D::setDefaultAlphaPixelFormat(currentPixelFormat);
	NS_ASSERT(Director::getInstance()->getTextureCache()->getTextureForKey(m_path) != nullptr);
}

CachedTexture::~CachedTexture()
{
	if (!m_path.empty())
		Director::getInstance()->getTextureCache()->removeTextureForKey(m_path);
}


BaseScene::BaseScene() :
	m_resultCode(RESULT_CODE_OK)
{
}

BaseScene::~BaseScene()
{

}

bool BaseScene::init()
{
	return Layer::init();
}


void BaseScene::startSceneForResult(Scene* scene, ResultCallback const& callback)
{
	BaseScene* layer = nullptr;
	auto const& children = scene->getChildren();
	for (auto it = children.begin(); it != children.end(); ++it)
	{
		layer = dynamic_cast<BaseScene*>(*it);
		if (layer)
		{
			layer->m_resultCallback = callback;
			break;
		}
			
	}

	NS_ASSERT_LOG(layer != nullptr, "No BaseScene layer was found in the starting scene.");

	Director::getInstance()->pushScene(scene);
}

void BaseScene::setResult(ResultCode resultCode, ValueMapIntKey const& data)
{
	m_resultCode = resultCode;
	m_data = data;
}

void BaseScene::finish()
{
	if (this->m_resultCallback)
		this->m_resultCallback(m_resultCode, m_data);

	Director::getInstance()->popScene();
}

std::string BaseScene::autoUncacheImage(std::string const& resName, Texture2D::PixelFormat format, bool isLocale)
{
	auto ret = m_cachedTextures.emplace(resName, format, isLocale);
	NS_ASSERT(ret.second);
	return (*ret.first).getPath();
}



NS_END
