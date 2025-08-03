#ifndef __ICON_BUTTON_H__
#define __ICON_BUTTON_H__

#include "ui/UIWidget.h"
#include "ui/UIScale9Sprite.h"

#include "common/Common.h"

USING_NS_CC;

NS_BEGIN

class IconButton : public ui::Widget
{
public:
	static IconButton* create(std::string const& backgroundImage, std::string const& title, std::string const& iconImage);
	virtual bool init(std::string const& backgroundImage, std::string const& title, std::string const& iconImage);

	IconButton();
	~IconButton();

	void setBackgroundIconImage(std::string const& image);
	Sprite* getBackgroundIcon() { return m_backgroundIconRenderer; }
	void setBackgroundImage(std::string const& image);

	void setTitleText(std::string const& text);
	void setTitleColor(Color4B const& color);
	void setTitleFontSize(float size);
	Label* getTitleLabel() { return m_titleRenderer; }

	void setZoomScale(float scale);
	float getZoomScale() const;

protected:
	virtual void initRenderer() override;
	virtual void onPressStateChangedToNormal() override;
	virtual void onPressStateChangedToPressed() override;
	virtual void onPressStateChangedToDisabled() override;
	virtual void onSizeChanged() override;
	virtual void adaptRenderers() override;

private:
	void updateBackgroundIconPosition();

	float m_zoomScale;
	bool m_backgroundAdaptDirty;
	bool m_titleAdaptDirty;

	Node* m_backgroundContainer;
	ui::Scale9Sprite* m_backgroundRenderer;
	Sprite* m_backgroundIconRenderer;
	Sprite* m_iconRenderer;
	Label* m_titleRenderer;
	Node* m_titleContainer;
};

NS_END


#endif // __ICON_BUTTON_H__
