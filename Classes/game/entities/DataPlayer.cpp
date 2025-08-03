#include "DataPlayer.h"

NS_BEGIN

const StageStat StageStat::Null;

DataPlayer::DataPlayer():
	m_isGM(false),
	m_viewport(Size::ZERO),
	m_money(0),
	m_pickupTarget(ObjectGuid::EMPTY),
	m_isSelf(false)

{
	m_type |= DATA_TYPEMASK_PLAYER;
	m_typeId = DataTypeID::DATA_TYPEID_PLAYER;

	m_updateMask.setCount(CPLAYER_END);
}

DataPlayer::~DataPlayer()
{
}

void DataPlayer::clearFields()
{
	m_isGM = false;
	m_money = 0;
	for (int32 slot = 0; slot < UNIT_SLOTS_COUNT; ++slot)
	{
		m_items[slot] = ObjectGuid::EMPTY;
	}
	m_pickupTarget = ObjectGuid::EMPTY;

	DataUnit::clearFields();
}

bool DataPlayer::readFields(UpdateType updateType, uint32 updateFlags, FieldUpdateMask const& updateMask, DataInputStream* input)
{
	CHECK_READ(DataUnit::readFields(updateType, updateFlags, updateMask, input));

	if (Updatable::isNeedReadField(updateMask, SPLAYER_FIELD_IS_GM))
	{
		CHECK_READ((Parcel::readBool(input, &m_isGM)));
		this->setUpdatedField(CPLAYER_FIELD_IS_GM);
	}

	if ((updateFlags & UPDATE_FLAG_SELF) != 0)
	{
		if (Updatable::isNeedReadField(updateMask, SPLAYER_FIELD_EXPERIENCE))
		{
			CHECK_READ((Parcel::readInt32(input, &m_experience)));
			this->setUpdatedField(CPLAYER_FIELD_EXPERIENCE);
		}

		if (Updatable::isNeedReadField(updateMask, SPLAYER_FIELD_NEXTLEVEL_XP))
		{
			CHECK_READ((Parcel::readInt32(input, &m_nextLevelXP)));
			this->setUpdatedField(CPLAYER_FIELD_NEXTLEVEL_XP);
		}

		if (Updatable::isNeedReadField(updateMask, SPLAYER_FIELD_MONEY))
		{
			CHECK_READ(Parcel::readInt32(input, &m_money));
			this->setUpdatedField(CPLAYER_FIELD_MONEY);
		}

		for (int32 slot = 0; slot < UNIT_SLOTS_COUNT; ++slot)
		{
			if (Updatable::isNeedReadField(updateMask, SPLAYER_FIELD_ITEM_HEAD + slot))
			{
				uint32 _guid;
				CHECK_READ((Parcel::readUInt32(input, &_guid)));
				m_items[slot] = ObjectGuid(_guid);
				this->setUpdatedField(CPLAYER_FIELD_ITEM_HEAD + slot);
			}
		}

		if (Updatable::isNeedReadField(updateMask, SPLAYER_FIELD_CONCEALMENT_STATE))
		{
			uint32 concealmentState;
			CHECK_READ(Parcel::readUInt32(input, &concealmentState));
			m_concealmentState = static_cast<ConcealmentState>(concealmentState);
			this->setUpdatedField(CPLAYER_FIELD_CONCEALMENT_STATE);
		}

		if (Updatable::isNeedReadField(updateMask, SPLAYER_FIELD_PICKUP_DURATION))
		{
			CHECK_READ(Parcel::readInt32(input, &m_pickupDuration));
		}

		if (Updatable::isNeedReadField(updateMask, SPLAYER_FIELD_PICKUP_TARGET))
		{
			uint32 _guid;
			CHECK_READ((Parcel::readUInt32(input, &_guid)));
			m_pickupTarget = ObjectGuid(_guid);
			this->setUpdatedField(CPLAYER_FIELD_PICKUP_TARGET);
		}

		if (Updatable::isNeedReadField(updateMask, SPLAYER_FIELD_KILL_COUNT))
		{
			CHECK_READ((Parcel::readInt32(input, &m_killCount)));
			this->setUpdatedField(CPLAYER_FIELD_KILL_COUNT);
		}
	}

	return true;
}

void DataPlayer::setMaxStamina(int32 maxStamina)
{
	if (m_staminaInfo.maxStamina != maxStamina)
	{
		m_staminaInfo.maxStamina = maxStamina;
		this->setUpdatedField(CPLAYER_FIELD_MAX_STAMINA);
	}
}

void DataPlayer::setStaminaRegenRate(float rate)
{
	if (m_staminaInfo.staminaRegenRate != rate)
	{
		m_staminaInfo.staminaRegenRate = rate;
		this->setUpdatedField(CPLAYER_FIELD_STAMINA_REGEN_RATE);
	}
}


NS_END