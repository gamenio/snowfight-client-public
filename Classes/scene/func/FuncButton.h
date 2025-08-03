//
//  FuncButton.h
//  snowfight
//
//  Created by Luthier on 2020/07/05.
//

#ifndef __FUNC_BUTTON_H__
#define __FUNC_BUTTON_H__

#include "ui/UIButton.h"

#include "common/Common.h"
#include "game/GameConfig.h"
#include "BuyButton.h"

USING_NS_CC;


NS_BEGIN

class FuncButton: public Node
{
public:
    static FuncButton* create();
    bool init();
    
	FuncButton();
    ~FuncButton();

	void setHeroTemplate(HeroTemplate const* tmpl);
	HeroTemplate const* getHeroTemplate() const { return m_template; }
	void updateButton();
	bool canPlay() const { return m_playBtn->isVisible(); }

	void setEnabled(bool enabled);

	void setBuyButtonClickEventListener(BuyButtonClickCallback const& callback);
	void setPlayButtonClickEventListener(ui::Widget::ccWidgetClickCallback const& callback);

private:
	HeroTemplate const* m_template;

	ui::Button* m_playBtn;
	BuyButton* m_currencyBuyBtn;
	BuyButton* m_gameCoinBuyBtn;
};


NS_END


#endif // __FUNC_BUTTON_H__

