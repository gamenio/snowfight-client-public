#ifndef __OPTION_BUTTON_H__
#define __OPTION_BUTTON_H__

#include "ui/UIWidget.h"

#include "common/Common.h"

USING_NS_CC;

NS_BEGIN

class OptionButton : public cocos2d::ui::Widget
{
public:
	static OptionButton* create(cocos2d::Size const& contentSize, std::string const& title, std::string const& iconImage, TextureResType texType = Widget::TextureResType::PLIST);
	virtual bool init(cocos2d::Size const& contentSize, std::string const& title, std::string const& iconImage, TextureResType texType);

	OptionButton();
	~OptionButton();

	void setTitle(std::string const& title);
	Label* getTitleRender() { return m_titleRender; }
	void setIconImage(std::string const& iconImage);

protected:
	virtual void onSizeChanged() override;
	virtual void adaptRenderers() override;
	virtual void initRenderer() override;
	virtual void onPressStateChangedToNormal() override;
	virtual void onPressStateChangedToPressed() override;
	virtual void onPressStateChangedToDisabled() override;
    virtual void releaseUpEvent() override;

private:
	void loadTextureIconImage(std::string const& image);
	void updateTitleRenderColor(Color4B const& color);

	Label* m_titleRender;
	Sprite* m_iconImageRender;
	cocos2d::ui::Scale9Sprite* m_backgroundPressedRender;
	cocos2d::ui::Scale9Sprite* m_backgroundNormalRender;
	TextureResType m_texType;

	bool m_rendersAdaptDirty;

};

NS_END


#endif // __OPTION_BUTTON_H__
