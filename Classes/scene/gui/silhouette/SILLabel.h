#ifndef __SIL_LABEL_H__
#define __SIL_LABEL_H__

#include "common/Common.h"
#include "SilhouetteLayer.h"

USING_NS_CC;

NS_BEGIN

class SILLabel : public Label, public SilhouettableProtocol
{
public:
	static SILLabel* createWithSystemFont(std::string const& text, std::string const& font, float fontSize, cocos2d::Size const& dimensions = cocos2d::Size::ZERO, TextHAlignment hAlignment = TextHAlignment::LEFT, TextVAlignment vAlignment = TextVAlignment::TOP);
	static SILLabel* createWithBMFont(std::string const& bmfontPath, std::string const& text, TextHAlignment const& hAlignment = TextHAlignment::LEFT, int maxLineWidth = 0, Vec2 const& imageOffset = Vec2::ZERO);

	SILLabel(TextHAlignment hAlignment = TextHAlignment::LEFT, TextVAlignment vAlignment = TextVAlignment::TOP);
	~SILLabel();

	// 如果创建的是BMFont，启用阴影后剪影效果将不工作
	void enableShadow(Color4B const& shadowColor = Color4B::BLACK, Size const& offset = Size(2,-2), int blurRadius = 0) override;
	void updateContent() override;

	bool isSilhouetted() const override { return m_isSilhouetted; }
	void setSilhouetted(bool silhouetted) override;

	GLubyte getEffectOpacity() const override { return m_effectOpacity; }
	void setEffectOpacity(GLubyte opacity) override;

	void setScreenTexture(Texture2D* texture) override;

	void visit(Renderer* renderer, Mat4 const& parentTransform, uint32_t parentFlags) override;
	void draw(Renderer* renderer, Mat4 const& transform, uint32_t flags) override;

protected:
	void setFontAtlas(FontAtlas* atlas, bool distanceFieldEnabled = false, bool useA8Shader = false) override;

private:
	void createSpriteForSystemFont(FontDefinition const& fontDef);
	void createShadowSpriteForSystemFont(FontDefinition const& fontDef);

	bool m_isSilhouetted;
	GLubyte m_effectOpacity;
	Texture2D* m_screenTexture;

	QuadCommand m_maskQuadCommand;
	QuadCommand m_effectQuadCommand;
};

NS_END

#endif // __SIL_LABEL_H__
