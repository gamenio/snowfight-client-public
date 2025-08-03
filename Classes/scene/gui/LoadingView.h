#ifndef __LOADING_VIEW_H__
#define __LOADING_VIEW_H__

#include "ui/UIScale9Sprite.h"

#include "common/Common.h"
#include "LoadingIndicator.h"

USING_NS_CC;

NS_BEGIN

class LoadingView : public LayerColor
{
public:
	static LoadingView* create(Node* owner);
	bool init(Node* owner);

	LoadingView();
	virtual ~LoadingView();

	virtual bool onTouchBegan(Touch* touch, Event* event) override;
	virtual void onTouchEnded(Touch* touch, Event* event) override;
	virtual void onTouchCancelled(Touch* touch, Event* event) override;
	virtual void onTouchMoved(Touch* touch, Event* event) override;
	virtual void onKeyReleased(EventKeyboard::KeyCode keyCode, Event *event) override;

	void show();
	void dismiss();

private:
	Node* m_owner;
	Node* m_mainPanel;
    ui::Scale9Sprite* m_background;
	LoadingIndicator* m_loadingIndicator;
};

NS_END

#endif // __LOADING_VIEW_H__
