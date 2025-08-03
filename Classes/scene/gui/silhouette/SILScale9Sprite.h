#ifndef __SIL_SCALE9_SPRITE_H__
#define __SIL_SCALE9_SPRITE_H__

#include "common/Common.h"
#include "SilhouetteLayer.h"
#include "ui/UIScale9Sprite.h"

USING_NS_CC;

NS_BEGIN

class SILScale9Sprite : public ui::Scale9Sprite, public SilhouettableProtocol
{
public:
	static SILScale9Sprite* create();
	static SILScale9Sprite* create(std::string const& filename);
	static SILScale9Sprite* create(std::string const& filename, cocos2d::Rect const& rect, cocos2d::Rect const& capInsets);
	static SILScale9Sprite* create(cocos2d::Rect const& capInsets, std::string const& filename);
	static SILScale9Sprite* create(std::string const& filename, cocos2d::Rect const& rect);
	static SILScale9Sprite* createWithSpriteFrame(SpriteFrame* spriteFrame);
	static SILScale9Sprite* createWithSpriteFrame(SpriteFrame* spriteFrame, cocos2d::Rect const& capInsets);
	static SILScale9Sprite* createWithSpriteFrameName(std::string const& spriteFrameName);
	static SILScale9Sprite* createWithSpriteFrameName(std::string const& spriteFrameName, cocos2d::Rect const& capInsets);

	SILScale9Sprite();
	~SILScale9Sprite();

	using Scale9Sprite::initWithTexture;
	using Scale9Sprite::initWithFile;
	using Scale9Sprite::initWithSpriteFrame;
	using Scale9Sprite::initWithSpriteFrameName;

	using Scale9Sprite::setTexture;
	using Scale9Sprite::setSpriteFrame;

	bool initWithTexture(Texture2D* texture, cocos2d::Rect const& rect, bool rotated) override;
	void setTexture(Texture2D* texture) override;

	bool isSilhouetted() const override { return m_isSilhouetted; }
	void setSilhouetted(bool silhouetted) override;

	GLubyte getEffectOpacity() const override { return m_effectOpacity; }
	void setEffectOpacity(GLubyte opacity) override;

	void setScreenTexture(Texture2D* texture) override;

	void visit(Renderer* renderer, Mat4 const& parentTransform, uint32_t parentFlags) override;
	void draw(Renderer* renderer, Mat4 const& transform, uint32_t flags) override;

private:
	bool m_isSilhouetted;
	Texture2D* m_screenTexture;
	GLubyte m_effectOpacity;

	GLProgramState* m_maskGLProgramState;
	GLProgramState* m_effectGLProgramState;
	TrianglesCommand m_maskTrianglesCommand;
	TrianglesCommand m_effectTrianglesCommand;
};

NS_END

#endif // __SIL_SCALE9_SPRITE_H__
