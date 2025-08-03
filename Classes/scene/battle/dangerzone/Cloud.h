#ifndef __CLOUD_H__
#define __CLOUD_H__

#include "common/Common.h"

USING_NS_CC;

NS_BEGIN

class Cloud;

class CloudStateListener
{
public:
	virtual void onCloudWillAppear(Cloud* sender) {}
	virtual void onCloudDidDisappear(Cloud* sender) {}
};

class Cloud : public Node
{
public:
	static const float ANIMATION_DURATION;

	struct RepeatedActionConfig
	{
		Vec2 move1Offset;
		float move1Duration;
		float scale1;
		float scale1Duration;
		Vec2 move2Offset;
		float move2Duration;
		float scale2;
		float scale2Duration;
		float scale3;
		float scale3Duration;
		std::function<void(Node*)> willAppearCallback;
		std::function<void(Node*)> didAppearCallback;
		std::function<void(Node*)> didDisappearCallback;
	};

	static Cloud* create(float interval, bool isAheadInterval);
	bool init(float interval, bool isAheadInterval);

	Cloud();
	~Cloud();

	void setColor(Color3B const& color) override;

	void startAnimation(float delay = 0.f, int32 loops = 0);
	void stopAnimation();
	void resetAnimation();

	void setCloudStateListener(CloudStateListener* listener) { m_stateListener = listener; }
	void removeCloudStateListener() { m_stateListener = nullptr; }

	void step(float dt);
	void update(float delta) override;

private:
	void initRepeatedActions();
	ActionInterval* createRepeatedAction(RepeatedActionConfig const& config);

	void startRepeatedAction(Node* target, int32 loops, ActionInterval* repeatedAction);

	void onCloudWillAppear(Node* node);
	void onCloudDidAppear(Node* node);
	void onCloudDidDisappear(Node* node);

	CloudStateListener* m_stateListener;

	float m_interval;
	float m_isAheadInterval;

	float m_delay;
	float m_elapsed;
	bool m_isFirstTick;
	bool m_isInDelay;

	Sprite* m_fragmentSp1;
	Sprite* m_fragmentSp2;
	Sprite* m_fragmentSp3;
	Map<Node*, ActionInterval*> m_repeatedActions;
	Vector<Action*> m_actions;
	Action* m_currAction;
	bool m_isStopActionsDelayed;
};

NS_END

#endif // __CLOUD_H__
