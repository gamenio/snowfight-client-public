#ifndef __TUTORIAL_VIEW_H__
#define __TUTORIAL_VIEW_H__

#include "ui/UIScale9Sprite.h"
#include "ui/UIRichText.h"

#include "common/Common.h"
#include "scene/TutorialService.h"

USING_NS_CC;

NS_BEGIN

enum TutorialStyle
{
	TUTORIAL_STYLE_NONE,
	TUTORIAL_STYLE_ANNOUNCER,
	TUTORIAL_STYLE_BUBBLE_RIGHT,
};

class TutorialView : public Node
{
public:
	static TutorialView* create();
	bool init() override;

	TutorialView();
	virtual ~TutorialView();

	bool isShown() const { return m_shownProcess != TUTORIAL_PROCESS_NONE; }
	void show(TutorialProcess process);
	void hide();
	TutorialProcess getShownProcess() const { return m_shownProcess; }

private:
	void setupTutorial();
	void setTutorialStyle(TutorialStyle style);

	void setIconSpriteFrame(std::string const& frameName);
	void setupIconAnimation(std::string const& frameNameFormat, int32 numberOfFrames, float delayPerFrame, int32 defaultFrameIndex = 0);

	void layoutElements();
	void setupElementsForAction();

	void startShowAnimation();
	void startHideAnimation();
	void startHideAnnouncerBubbleAnimation(std::function<void()> complete = nullptr);

	void setText(std::string const& text);

	float m_characterStartX;

	TutorialProcess m_shownProcess;
	TutorialStyle m_style;
	ui::Scale9Sprite* m_bubbleSp;
	Sprite* m_arrowSp;
	Sprite* m_characterSp;
	Sprite* m_iconSp;
	Animation* m_iconAnimation;
	ui::RichText* m_textLabel;
};

NS_END

#endif // __TUTORIAL_VIEW_H__
