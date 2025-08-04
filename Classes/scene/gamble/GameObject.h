#ifndef __GAME_OBJECT_H__
#define __GAME_OBJECT_H__

#include "common/Common.h"
#include "scene/GameMapLayer.h"
#include "Gamble.h"

USING_NS_CC;

NS_BEGIN

class GameObject : public Node, public Gamble
{
public:
	GameObject();
	virtual ~GameObject();

	virtual bool init() override;

	virtual void update(float delta) override;

	// Game map layer
	void setGameMapLayer(GameMapLayer* layer) { m_gameMapLayer = layer; }
	GameMapLayer* getGameMapLayer() const { return m_gameMapLayer; }

	// The object's active state
	virtual void onActivated() override;
	virtual void onInactivated() override;
	virtual bool isActive() const override { return m_isActive; }

	virtual void cleanUpdateMask() override;

protected:
	bool m_isActive;

	GameMapLayer* m_gameMapLayer;
};

NS_END

#endif

