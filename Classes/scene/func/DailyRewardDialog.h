#ifndef __DAILY_REWARD_DIALOG_H__
#define __DAILY_REWARD_DIALOG_H__

#include "scene/gui/ModalDialog.h"

#include "common/Common.h"
#include "game/LocaleMgr.h"
#include "game/ShopMgr.h"
#include "scene/gui/IconButton.h"
#include "scene/gui/FingerTap.h"

USING_NS_CC;

NS_BEGIN

class DailyRewardDialog : public ModalDialog
{
public:
	typedef std::function<void(Ref*, DailyRewardType, int32)> DialogCallback;

	static DailyRewardDialog* create();
	bool init() override;

	DailyRewardDialog();
	~DailyRewardDialog();

	void setShowFingerTap(bool isShown);
	void setWatchAdButtonCallback(DialogCallback const& callback) { m_watchAdBtnCallback = callback; }
	void setClaimButtonCallback(DialogCallback const& callback) { m_claimBtnCallback = callback; }

	void onEnter() override;

protected:
	void onAppeared() override;
	void onWillDisappear() override;

private:
	void initButtons();
	void initContents();

	void layoutGroupElements();

	void loadData();

	void setMessage(std::string const& message);
	void setTodayReward(DailyRewardType rewardType, int32 value);
	void setDaysReward(int32 days, DailyRewardType rewardType, int32 value);

	DailyRewardType m_todayRewardType;
	int32 m_todayValue;

	ui::Scale9Sprite* m_titleBg;
	Label* m_titleLabel;
	Node* m_titleNode;
	Label* m_messageLabel;
	Node* m_contentPanel;;
	Node* m_groupContainer;
	Sprite* m_dividerSp;

	Node* m_todayContainer;
	Label* m_todayTitleLabel;
	Sprite* m_todayPicSp;
	Label* m_todayCaptionLabel;
	Node* m_daysContainer;
	Label* m_daysTitleLabel;
	Sprite* m_daysPicSp;
	Label* m_daysCaptionLabel;

	IconButton* m_watchAdBtn;
	DialogCallback m_watchAdBtnCallback;
	ui::Button* m_claimBtn;
	DialogCallback m_claimBtnCallback;

	FingerTap* m_fingerTap;
};


NS_END

#endif // __DAILY_REWARD_DIALOG_H__
