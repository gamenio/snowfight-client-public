#include "DataProjectile.h"

NS_BEGIN

DataProjectile::DataProjectile():
	m_launcher(ObjectGuid::EMPTY),
	m_launcherOrigin(Point::ZERO),
	m_attackRange(0),
	m_launchCenter(Point::ZERO),
	m_launchRadiusInMap(0),
	m_position(Point::ZERO),
	m_orientation(0),
	m_elapsed(0),
	m_duration(0),
	m_attackCounter(0),
	m_consumedStamina(0),
	m_scale(0),
	m_attackInfoCounter(0),
	m_status(LAUNCHSTATUS_NONE)
{
	m_type |= DataTypeMask::DATA_TYPEMASK_PROJECTILE;
	m_typeId = DataTypeID::DATA_TYPEID_PROJECTILE;
    
    m_updateMask.setCount(CPROJECTILE_END);
}

DataProjectile::~DataProjectile()
{
}

void DataProjectile::clearFields()
{
	m_launcher = ObjectGuid::EMPTY;
	m_launcherOrigin = Point::ZERO;
	m_attackRange = 0;
	m_launchCenter = Point::ZERO;
	m_launchRadiusInMap = 0;
	m_orientation = 0;
	m_elapsed = 0;
	m_duration = 0;
	m_attackCounter = 0;
	m_consumedStamina = 0;
	m_scale = 0;
	m_attackInfoCounter = 0;
	m_status = LAUNCHSTATUS_NONE;

	DataWorldObject::clearFields();
}

bool DataProjectile::readFields(UpdateType updateType, uint32 updateFlags, FieldUpdateMask const& updateMask, DataInputStream* input)
{
    CHECK_READ(DataWorldObject::readFields(updateType, updateFlags, updateMask, input));
    
	if (Updatable::isNeedReadField(updateMask, SPROJECTILE_FIELD_LAUNCHER))
	{
		uint32 _guid;
		CHECK_READ((Parcel::readUInt32(input, &_guid)));
		m_launcher = ObjectGuid(_guid);
	}

	if (Updatable::isNeedReadField(updateMask, SPROJECTILE_FIELD_LAUNCHER_ORIGIN))
	{
		CHECK_READ(Parcel::readFloat(input, &m_launcherOrigin.x));
		CHECK_READ(Parcel::readFloat(input, &m_launcherOrigin.y));
	}

	if (Updatable::isNeedReadField(updateMask, SPROJECTILE_FIELD_ATTACK_RANGE))
		CHECK_READ((Parcel::readFloat(input, &m_attackRange)));

	if (Updatable::isNeedReadField(updateMask, SPROJECTILE_FIELD_LAUNCH_CENTER))
	{
		CHECK_READ(Parcel::readFloat(input, &m_launchCenter.x));
		CHECK_READ(Parcel::readFloat(input, &m_launchCenter.y));
	}

	if (Updatable::isNeedReadField(updateMask, SPROJECTILE_FIELD_LAUNCH_RADIUS_IN_MAP))
		CHECK_READ(Parcel::readFloat(input, &m_launchRadiusInMap));

	if (Updatable::isNeedReadField(updateMask, SPROJECTILE_FIELD_ORIENTATION))
		CHECK_READ((Parcel::readFloat(input, &m_orientation)));

	if (Updatable::isNeedReadField(updateMask, SPROJECTILE_FIELD_ELAPSED))
		CHECK_READ((Parcel::readInt32(input, &m_elapsed)));

	if (Updatable::isNeedReadField(updateMask, SPROJECTILE_FIELD_DURATION))
		CHECK_READ((Parcel::readInt32(input, &m_duration)));

	if (Updatable::isNeedReadField(updateMask, SPROJECTILE_FIELD_ATTACK_COUNTER))
		CHECK_READ(Parcel::readUInt32(input, &m_attackCounter));

	if (Updatable::isNeedReadField(updateMask, SPROJECTILE_FIELD_CONSUMED_STAMINA))
		CHECK_READ(Parcel::readInt32(input, &m_consumedStamina));

	if (Updatable::isNeedReadField(updateMask, SPROJECTILE_FIELD_SCALE))
		CHECK_READ(Parcel::readFloat(input, &m_scale));

	if (Updatable::isNeedReadField(updateMask, SPROJECTILE_FIELD_ATTACK_INFO_COUNTER))
		CHECK_READ(Parcel::readUInt32(input, &m_attackInfoCounter));

	if (Updatable::isNeedReadField(updateMask, SPROJECTILE_FIELD_STATUS))
	{
		int32 _status;
		CHECK_READ(Parcel::readInt32(input, &_status));
		m_status = static_cast<LaunchStatus>(_status);
	}

    return true;
}

NS_END
