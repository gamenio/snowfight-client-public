#ifndef __PROP_BUBBLE_H__
#define __PROP_BUBBLE_H__

#include "common/Common.h"
#include "game/entities/DataItem.h"
#include "scene/gui/silhouette/SILLabel.h"
#include "scene/gui/silhouette/SILScale9Sprite.h"
#include "scene/gui/silhouette/SILSprite.h"

USING_NS_CC;


NS_BEGIN

class PropBubble: public Node
{
public:
    static PropBubble* create();
    
    bool init() override;
    
    PropBubble();
    ~PropBubble();

	void show(ItemTemplate const* itemTemplate, PickupStatus status, float duration = 0.f);
	bool isShown() const { return m_isShown; }
	void hide(bool animated = true);

	void setGlobalZOrder(float globalZOrder) override;

private:
	void startPopAnimation(float duration);
	void startPushAnimation();
	void stopAnimation();

	void layoutElements();

	std::string getItemPickupErrorMessage(uint32 errorCode);

	bool m_isShown;

	Node* mainNode;
	SILLabel* m_titleLabel;
	SILLabel* m_introLabel;
	SILScale9Sprite* m_bubbleSp;
	SILSprite* m_arrowSp;
	SILSprite* m_iconSp;
	SILSprite* m_iconShadowSp;
};


NS_END


#endif // __PROP_BUBBLE_H__

