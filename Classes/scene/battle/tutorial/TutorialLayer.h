#ifndef __TUTORIAL_LAYER_H__
#define __TUTORIAL_LAYER_H__

#include "common/Common.h"
#include "TutorialView.h"
#include "scene/battle/InventoryBar.h"
#include "scene/GameMapLayer.h"
#include "scene/TutorialService.h"

USING_NS_CC;

NS_BEGIN

class BattleScene;

class TutorialLayer : public Layer, TutorialEventListener
{
public:
	enum SlideInDirection
	{
		SLIDEIN_FROM_LEFT,
		SLIDEIN_FROM_RIGHT,
		SLIDEIN_FROM_BOTTOM,
	};
	static TutorialLayer* create(BattleScene* scene);

	TutorialLayer();
	virtual ~TutorialLayer();

	bool init(BattleScene* scene);

	void startTutorial();

	// TutorialEventListener
	void onTutorialEventTriggered(TutorialEventType eventType) override;

private:
	void setNextTutorialProcess(TutorialProcess process, float delay, bool isStepByStep = true);
	void onTutorialProcessCompleted(TutorialProcess process);
	void saveTutorialProcess(TutorialProcess process);

	void showTutorial(TutorialProcess process);
	void hideTutorial(float delay);

	void showControlWithSlideIn(Node* control, float duration, float delay, SlideInDirection dir);
	void showControlWithFadeIn(Node* control, float duration, float delay);

	void playNext(float delay);

	BattleScene* m_scene;
	float m_safeInsetLeft;
	float m_safeInsetRight;

	TutorialView* m_tutorialView;
	int32 m_currTutorialProcess;
};

NS_END

#endif // __TUTORIAL_LAYER_H__
