#ifndef __CLOUD_CLUSTER_H__
#define __CLOUD_CLUSTER_H__

#include "common/Common.h"
#include "scene/ObjectPools.h"
#include "Cloud.h"
#include "CloudGenerator.h"

USING_NS_CC;

NS_BEGIN

class CloudCluster : public Node , CloudStateListener
{
public:
	static const float LARGE_CLOUD_DELAY;
	static const float SMALL_CLOUD_DELAY;
	static const float ACTIVE_CLOUD_DURATION;

	static CloudCluster* create();
	bool init() override;

	CloudCluster();
	~CloudCluster();

	void activate();
	bool isActivated() const { return m_isActivated; }
	void reset();
	
	float getDeviation() const { return m_deviation; }

	void setData(CloudData* data);
	CloudData* getData() const { return m_data; }

	// CloudStateListener
	void onCloudWillAppear(Cloud* sender) override;
	void onCloudDidDisappear(Cloud* sender) override;

	void update(float delta) override;
	void onEnter() override;
	void onExit() override;

private:
	void inactivate();
	void updateLoopsAction(float delta);

	DrawNode* m_debugDraw;

	CloudData* m_data;

	bool m_isActivated;
	float m_deviation;

	float m_elapsed;
	float m_duration;
	bool m_isFirstTick;
	CloudData::Action m_performingAction;

	Cloud* m_activeCloud;
	Cloud* m_cloud1;
	Cloud* m_cloud2;
};

typedef ObjectPool<CloudCluster> CloudClusterPool;
typedef ObjectPoolListener<CloudCluster> CloudClusterPoolListener;

NS_END

#endif // __CLOUD_CLUSTER_H__
