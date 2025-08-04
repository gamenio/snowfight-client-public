#ifndef __UPDATABLE_H__
#define __UPDATABLE_H__

#include "common/Common.h"
#include "FieldUpdateMask.h"

NS_BEGIN


class Updatable
{
public:
	static bool isNeedReadField(FieldUpdateMask const& updateMask, uint32 fieldIndex)
	{
		return updateMask.testBit(fieldIndex);
	}

	Updatable(){ }
	virtual ~Updatable() { }

	Updatable(Updatable const& right) = delete;
	Updatable& operator=(Updatable const& right) = delete;

	uint32 getNumberOfFields() const { return m_updateMask.count(); }
	// Set the field to be updated
	// Call this function is invalid when the update type of data is CREATE
	void setUpdatedField(uint32 fieldIndex) { m_updateMask.setBit(fieldIndex); }
	bool hasUpdatedField(uint32 fieldIndex) const { return m_updateMask.testBit(fieldIndex); }
	void clearUpdateFlags() { m_updateMask.reset(); }
	std::string getUpdateFlagsString() const { return m_updateMask.description(); };

protected:
	FieldUpdateMask m_updateMask;
};

NS_END

#endif // __UPDATABLE_H__
