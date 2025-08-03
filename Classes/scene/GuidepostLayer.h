#ifndef __GUIDEPOST_LAYER_H__
#define __GUIDEPOST_LAYER_H__

#include "common/Common.h"
#include "game/entities/DataLocatorObject.h"
#include "scene/gamble/Guidepost.h"
#include "game/WorldListeners.h"

USING_NS_CC;


NS_BEGIN

class GuidepostLayer: public Layer, LocatorObjectLifecycleListener
{
public:
	static GuidepostLayer* create();

	GuidepostLayer();
	~GuidepostLayer();

	bool init() override;

	void update(float delta) override;
	void cleanAfterUpdate();

	Guidepost* getGuidepost(ObjectGuid const& guid) const { return m_guideposts.at(guid); }
	template<class T> T* getGuidepost(ObjectGuid const& guid) const { return static_cast<T*>(m_guideposts.at(guid)); }

	// LocatorObjectLifecycleListener
	void onLocatorObjectDestroyed(ObjectGuid const& guid) override;
	void onLocatorObjectActivated(DataLocatorObject* data) override;
	void onLocatorObjectInactivated(ObjectGuid const& guid) override;

private:
	Map<ObjectGuid, Guidepost*> m_guideposts;
};

NS_END

#endif // __GUIDEPOST_LAYER_H__
