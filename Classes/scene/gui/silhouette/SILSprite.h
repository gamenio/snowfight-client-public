#ifndef __SIL_SPRITE_H__
#define __SIL_SPRITE_H__

#include "common/Common.h"
#include "SilhouetteLayer.h"

USING_NS_CC;

NS_BEGIN

class SILSprite : public Sprite, public SilhouettableProtocol
{
public:
	static SILSprite* create();
	static SILSprite* create(std::string const& filename);
	static SILSprite* create(std::string const& filename, cocos2d::Rect const& rect);
	static SILSprite* createWithTexture(Texture2D* texture);
	static SILSprite* createWithTexture(Texture2D* texture, cocos2d::Rect const& rect, bool rotated = false);
	static SILSprite* createWithSpriteFrame(SpriteFrame* spriteFrame);
	static SILSprite* createWithSpriteFrameName(std::string const& spriteFrameName);

	SILSprite();
	~SILSprite();

	using Sprite::initWithTexture;
	using Sprite::setTexture;

	bool initWithTexture(Texture2D* texture, cocos2d::Rect const& rect, bool rotated) override;

	void setBatchNode(SpriteBatchNode* spriteBatchNode) override;
	void setTexture(Texture2D* texture) override;

	bool isSilhouetted() const override { return m_isSilhouetted; }
	void setSilhouetted(bool silhouetted) override;

	GLubyte getEffectOpacity() const override { return m_effectOpacity; }
	void setEffectOpacity(GLubyte opacity) override;

	void setScreenTexture(Texture2D* texture) override;

	GLProgramState* getMaskGLProgramState() const { return m_maskGLProgramState; }
	GLProgramState* getEffectGLProgramState() const { return m_effectGLProgramState; }

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

#endif // __SIL_SPRITE_H__
