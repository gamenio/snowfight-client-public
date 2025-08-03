//
//  BuyButton.h
//  snowfight
//
//  Created by Luthier on 2019/5/9.
//

#ifndef __BUY_BUTTON_H__
#define __BUY_BUTTON_H__

#include "ui/UIButton.h"

#include "common/Common.h"
#include "game/ShopMgr.h"
#include "scene/gui/LoadingIndicator.h"

USING_NS_CC;


NS_BEGIN

enum MoneyType
{
	MONEY_TYPE_CURRENCY,
	MONEY_TYPE_GAME_COIN
};

typedef std::function<void(Ref*, MoneyType)> BuyButtonClickCallback;

class BuyButton: public Node
{
public:
    static BuyButton* create(MoneyType type);
    
    bool init(MoneyType type);
    
    BuyButton();
    ~BuyButton();

    void setVisible(bool visible) override;
    void setTitle(std::string const& title);
    void setEnabled(bool enabled);
    void setShowLoadingIndicator(bool show);
    void setClickEventListener(BuyButtonClickCallback const& callback);
    
private:
    void buttonClickCallback(Ref* sender);

	MoneyType m_type;
    ui::Button* m_button;
    LoadingIndicator* m_loadingIndicator;
    
    BuyButtonClickCallback m_clickCallback;
};


NS_END


#endif // __BUY_BUTTON_H__

