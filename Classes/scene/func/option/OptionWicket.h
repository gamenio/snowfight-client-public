#ifndef __OPTION_WICKET_H__
#define __OPTION_WICKET_H__

#include "common/Common.h"
#include "game/LocaleMgr.h"
#include "game/store/Store.h"
#include "scene/gui/Wicket.h"
#include "scene/gui/SwitchButton.h"
#include "scene/gui/OptionButton.h"
#include "scene/gui/LoadingView.h"
#include "scene/gui/CheckLabelBox.h"


USING_NS_CC;

NS_BEGIN

class OptionWicket : public Wicket, StoreListener
{
public:
	static OptionWicket* create(Node* owner);

	virtual bool init(Node* owner);

	OptionWicket();
	~OptionWicket();

	virtual void onEnter() override;
	virtual void update(float delta) override;

	void onClosed() override;

private:
	void switchSoundEffectCallback(Ref* sender);
	void switchMusicCallback(Ref* sender);
	void buttonRateClickCallback(Ref* sender);
	void buttonRestoreBuyClickCallback(Ref* sender);
	void restoreCompletedTransactions();

	void showLoadingView();
	void dismissLoadingView();

	// StoreListener
	void onPaymentTransactionUpdated(PaymentTransaction const& transaction) override;
	void onRestoreCompletedTransactionsFinished() override;
	void onRestoreCompletedTransactionsFailed(StoreError const& error) override;

	Node* m_main;
	SwitchButton* m_musicSwitch;
	SwitchButton* m_soundEffectSwitch;
	Label* m_musicLabel;
	Label* m_soundEffectLabel;
	ui::Button* m_rateBtn;
	ui::Button* m_restoreBuyBtn;
	Label* m_verLabel;
	LoadingView* m_loadingView;

	int32 m_restoredTransactions;

};

NS_END

#endif // __OPTION_WICKET_H__