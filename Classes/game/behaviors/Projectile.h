#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "game/entities/DataProjectile.h"
#include "game/WorldListeners.h"
#include "WorldObject.h"

NS_BEGIN

// 抛射体大小比例与体力值的比率
#define PROJECTILE_SCALE_TO_STAMINA_RATIO			0.02f // 抛射体大小比例/每点体力

class Projectile : public WorldObject
{
public:
	Projectile();
    virtual ~Projectile();

	void update(float delta) override;

    DataBasic* loadData(ObjectGuid const& guid);
    DataProjectile* getData() const override { return static_cast<DataProjectile*>(m_data); }

	void updateObject(UpdateType updateType, uint32 updateFlags, DataBasic* data) override;

private:
};


NS_END


#endif // __PROJECTILE_H__
