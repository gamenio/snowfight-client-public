#ifndef __SWITCH_BUTTON_H__
#define __SWITCH_BUTTON_H__

#include "ui/UIButton.h"

#include "common/Common.h"

USING_NS_CC;

NS_BEGIN

class SwitchButton : public cocos2d::ui::Widget
{
public:
	static SwitchButton* create(std::string const& offImage, std::string const& onImage, TextureResType texType = Widget::TextureResType::PLIST);
	virtual bool init(std::string const& offImage, std::string const& onImage, TextureResType texType);

    SwitchButton();
    virtual ~SwitchButton();

    bool isOn();
    void setOn(bool on);

protected:
	virtual void onSizeChanged() override;
	virtual void adaptRenderers() override;
	virtual void initRenderer() override;

	virtual void onPressStateChangedToNormal() override;
	virtual void onPressStateChangedToPressed() override;
	virtual void onPressStateChangedToDisabled() override;

private:
	bool revertSwitch();

	virtual bool onTouchBegan(Touch *touch, Event *unusedEvent) override;
	virtual void onTouchMoved(Touch *touch, Event *unusedEvent) override;
	virtual void onTouchEnded(Touch *touch, Event *unusedEvent) override;
	virtual void onTouchCancelled(Touch *touch, Event *unusedEvent) override;


	bool m_isOn;
	cocos2d::ui::Scale9Sprite* m_switchOnRender;
	cocos2d::ui::Scale9Sprite* m_switchOffRender;
	TextureResType m_texType;

	bool m_rendersAdaptDirty;
};

NS_END

#endif // __SWITCH_BUTTON_H__
