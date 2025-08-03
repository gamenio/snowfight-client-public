#ifndef __BATTLE_RESULT_DIALOG_H__
#define __BATTLE_RESULT_DIALOG_H__

#include "scene/gui/ModalDialog.h"

#include "common/Common.h"
#include "game/LocaleMgr.h"
#include "game/maps/BattleMap.h"
#include "scene/gui/IconButton.h"
#include "scene/gui/FingerTap.h"

USING_NS_CC;

NS_BEGIN

class BattleResultDialog: public ModalDialog
{
public:
	static BattleResultDialog* create();
	bool init() override;

	BattleResultDialog();
	~BattleResultDialog();

	void setBattleOutcome(BattleOutcome outcome);
	void setKillCount(int32 count);
	void setRankNo(int32 no);
	void setMoney(int32 amount, int32 extraAmount);
	void setExtraXP(int32 xp);

	void setShowFingerTap(bool isShown);
	void setWatchAdButtonVisible(bool isVisbile);
	void setHomeButtonCallback(DialogCallback const& callback) { m_homeBtnCallback = callback; }
	void setWatchAdButtonCallback(DialogCallback const& callback) { m_watchAdBtnCallback = callback; }

	void onEnter() override;

protected:
	void onAppeared() override;
	void onWillDisappear() override;

private:
	void initButtons();
	void initContents();
	void layoutGroupElements();

	BattleOutcome m_battleOutcome;
	bool m_isWatchAdButtonVisible;

	ui::Scale9Sprite* m_titleBg;
	Label* m_titleLabel;
	Node* m_titleNode;
	Node* m_contentPanel;;
	Label* m_killCountLabel;
	Label* m_rankNoLabel;
	Node* m_groupContainer;
	Sprite* m_moneyIconSp;
	Label* m_moneyAmountLabel;
	Sprite* m_xpIconSp;
	Label* m_extraAmountLabel;
	Label* m_extraXPLabel;
	FingerTap* m_fingerTap;

	IconButton* m_homeBtn;
	DialogCallback m_homeBtnCallback;
	IconButton* m_watchAdBtn;
	DialogCallback m_watchAdBtnCallback;
};


NS_END

#endif // __BATTLE_RESULT_DIALOG_H__
