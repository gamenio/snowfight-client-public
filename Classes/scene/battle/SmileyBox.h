#ifndef __SMILEY_BOX_H__
#define __SMILEY_BOX_H__

#include "ui/UIButton.h"

#include "common/Common.h"
#include "game/entities/DataUnit.h"

USING_NS_CC;

NS_BEGIN

typedef std::function<void(
                           Ref*,                // Sender
                           uint16               // Smiley Code
                           )> SmileyBoxSelectCallback;

class SmileyBox : public Node
{
public:
	static SmileyBox* create();
	bool init() override;

	SmileyBox();
	virtual ~SmileyBox();
    
    void setSelectEventListener(SmileyBoxSelectCallback const& callback);
    void setEnabled(bool enabled);
    bool isEnabled() const { return m_isEnabled; }
private:
	void layoutElements();
	void arrangeSmileysInContainer();

	void setPopupPanelTouchEnabled(bool enabled);
	void switchPopupPanel();
	void showPopupPanel();
	void hidePopupPanel();

	void buttonSwitchCallback(Ref* sender);
	void onSmileySelected(Ref* sender);

	bool m_isShowPopupPanel;
	bool m_isPopupPanelEnabled;
    bool m_isEnabled;

	ui::Button* m_switchBtn;
	ClippingNode* m_popupPanel;
	Node* m_smileyContainer;
    DrawNode* m_debugDrawNode;
    
    SmileyBoxSelectCallback m_selectCallback;
};

NS_END

#endif // __SMILEY_BOX_H__
