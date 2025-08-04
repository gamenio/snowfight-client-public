#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "game/entities/DataProjectile.h"
#include "game/WorldListeners.h"
#include "WorldObject.h"

NS_BEGIN

// The ratio of projectile scale to stamina
#define PROJECTILE_SCALE_TO_STAMINA_RATIO			0.02f // Projectile size scale / stamina per point

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
