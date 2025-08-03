#ifndef __REMOVE_ADS_DIALOG_H__
#define __REMOVE_ADS_DIALOG_H__

#include "scene/gui/ModalDialog.h"

#include "common/Common.h"
#include "game/LocaleMgr.h"
#include "game/maps/BattleMap.h"
#include "scene/gui/LoadingButton.h"
#include "game/store/StoreProtocol.h"
#include "game/ShopMgr.h"

USING_NS_CC;

NS_BEGIN

class RemoveAdsDialog : public ModalDialog, StoreListener
{
public:
	typedef std::function<void(Ref*, ProductTemplate const*)> BuyCallback;

	static RemoveAdsDialog* create();
	bool init() override;

	RemoveAdsDialog();
	~RemoveAdsDialog();

	void onEnter() override;

	void setBuyCallback(BuyCallback const& callback) { m_buyCallback = callback; }
	void setCancelButtonCallback(DialogCallback const& callback) { m_cancelBtnCallback = callback; }

private:
	void initButtons();
	void layoutTitleBar();
	void updateBuyButton();

	ui::Button* createNormalButton(std::string const& title, Color3B const& titleColor, std::string const& normalImage);

	void validateProductIds();

	// StoreListener
	void onProductsRequestFinished() override;
	void onProductsRequestFailed(StoreError const& error) override;

	ui::Scale9Sprite* m_titleBg;
	Label* m_titleLabel;
	Node* m_titleNode;
	Label* m_messageLabel;

	LoadingButton* m_buyBtn;
	BuyCallback m_buyCallback;
	ui::Button* m_cancelBtn;
	DialogCallback m_cancelBtnCallback;
};


NS_END

#endif // __REMOVE_ADS_DIALOG_H__
