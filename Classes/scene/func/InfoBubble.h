//
//  InfoBubble.h
//  snowfight
//
//  Created by Luthier on 2020/8/27.
//

#ifndef __INFO_BUBBLE_H__
#define __INFO_BUBBLE_H__

#include "ui/UIScale9Sprite.h"

#include "common/Common.h"

USING_NS_CC;


NS_BEGIN

class InfoBubble: public Node
{
public:
	enum ArrowDirection
	{
		ARROW_UP,
		ARROW_DOWN,
	};

    static InfoBubble* create(ArrowDirection direction);
    
    bool init(ArrowDirection direction);
    
    InfoBubble();
    ~InfoBubble();

	void show(std::string const& text);
	bool isShown() const { return m_isShown; }
	void hide();

	void setArrowPosition(float position);
	float getArrowPosition() const { return m_arrowPosition;  }

private:
	void startPopAnimation();
	void startPushAnimation();
	void stopAnimation();

	void layoutElements();
    
	bool m_isShown;
	ArrowDirection m_direction;
	float m_arrowPosition;

	Node* mainNode;
	Label* m_infoLabel;
	ui::Scale9Sprite* m_bubbleSp;
	Sprite* m_arrowSp;
	Sprite* m_iconSp;
};


NS_END


#endif // __INFO_BUBBLE_H__

