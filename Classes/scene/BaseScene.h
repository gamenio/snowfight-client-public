#ifndef __BASE_SCENE_H__
#define __BASE_SCENE_H__

#include "common/Common.h"
#include "game/LocaleMgr.h"

USING_NS_CC;

NS_BEGIN
class CachedTexture;
NS_END

namespace std
{
	using NS_PREFIX::CachedTexture;

	template<>
	struct hash<CachedTexture>
	{
	public:
		size_t operator()(CachedTexture const& key) const;
	};
}

NS_BEGIN

class CachedTexture
{
public:
	CachedTexture(std::string const& resName, Texture2D::PixelFormat format = Texture2D::PixelFormat::DEFAULT, bool isLocale = false);
	~CachedTexture();

	bool operator== (CachedTexture const& right) const { return m_path == right.m_path; }
	bool operator!= (CachedTexture const& right) const { return m_path != right.m_path; }

	std::string const& getPath() const { return m_path; }

private:
	std::string m_path;
};

class BaseScene : public Layer
{
public:
	enum ResultCode
	{
		RESULT_CODE_OK,
		RESULT_CODE_CANCEL
	};
	typedef std::function<void(ResultCode resultCode, ValueMapIntKey const& data)> ResultCallback;

	BaseScene();
	virtual ~BaseScene() = 0;

	bool init() override;

	void startSceneForResult(Scene* scene, ResultCallback const& resultCallback);
	void setResult(ResultCode resultCode, ValueMapIntKey const& data);
	void finish();

	// Caches the image named resName in the TextureCache and returns the image path.
	// If the parameter isLocale is true, the path will point to the localized directory (e.g., en-US).
	// Note: Cached images will be removed from the TextureCache when the scene is released.
	std::string autoUncacheImage(std::string const& resName, Texture2D::PixelFormat format = Texture2D::PixelFormat::DEFAULT, bool isLocale = false);

private:
	std::unordered_set<CachedTexture> m_cachedTextures;
	ValueMapIntKey m_data;

	ResultCallback m_resultCallback;
	ResultCode m_resultCode;
};

NS_END


#endif // __BASE_SCENE_H__