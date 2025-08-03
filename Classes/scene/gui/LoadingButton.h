//
//  LoadingButton.h
//  snowfight
//
//  Created by Luthier on 2021/8/1.
//

#ifndef __LOADING_BUTTON_H__
#define __LOADING_BUTTON_H__

#include "ui/UIButton.h"

#include "common/Common.h"
#include "scene/gui/LoadingIndicator.h"

USING_NS_CC;


NS_BEGIN

typedef std::function<void(Ref*)> LoadingButtonClickCallback;

class LoadingButton: public ui::Widget
{
public:
	static LoadingButton* create(std::string const& title, cocos2d::Size const& loadingIndicatorSize, std::string const& normalImage, std::string const& selectedImage = "", std::string const& disableImage = "");
	bool init(std::string const& title, cocos2d::Size const& loadingIndicatorSize, std::string const& normalImage, std::string const& selectedImage, std::string const& disableImage);
    
    LoadingButton();
    ~LoadingButton();

	void setVisible(bool visible) override;
    void setTitle(std::string const& title);
	void setTitleColor(Color4B const& color);
	void setTitleFontSize(float fontSize);
	void setEnabled(bool enabled) override;
	void setTouchEnabled(bool enabled) override;

	void setLoadingIndicatorColor(Color3B const& color);
    void setShowLoadingIndicator(bool show);

    void setClickEventListener(LoadingButtonClickCallback const& callback);

	void onEnter() override;

private:
	void layoutElements();
    void buttonClickCallback(Ref* sender);
    
    ui::Button* m_button;
    LoadingIndicator* m_loadingIndicator;
    
	LoadingButtonClickCallback m_clickCallback;
};


NS_END


#endif // __LOADING_BUTTON_H__

