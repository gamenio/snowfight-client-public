#ifndef __CATWALK_UNIT_H__
#define __CATWALK_UNIT_H__

#include "common/Common.h"


USING_NS_CC;


NS_BEGIN

class CatwalkUnit : public Sprite
{
public:
	CatwalkUnit();
	~CatwalkUnit();

	static CatwalkUnit* create(std::string const& filename);
	static CatwalkUnit* create(std::string const& filename, cocos2d::Rect const& rect);

	static CatwalkUnit* createWithTexture(Texture2D *texture);
	static CatwalkUnit* createWithTexture(Texture2D *texture, cocos2d::Rect const& rect, bool rotated = false);

	static CatwalkUnit* createWithSpriteFrame(SpriteFrame *spriteFrame);
	static CatwalkUnit* createWithSpriteFrameName(std::string const& spriteFrameName);

	float getLightness() const { return m_lightness; }
	void setLightness(float lightness);

	cocos2d::Rect getTrimmedBoundingBox() const;

private:

	bool initWithTexture(Texture2D *texture);
	bool initWithTexture(Texture2D *texture, cocos2d::Rect const& rect);
	virtual bool initWithTexture(Texture2D *texture, cocos2d::Rect const& rect, bool rotated);
	virtual bool initWithSpriteFrame(SpriteFrame *spriteFrame);

	float m_lightness;

};

NS_END

#endif // __CATWALK_UNIT_H__ 