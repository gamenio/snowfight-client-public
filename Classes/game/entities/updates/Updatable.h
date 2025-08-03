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
	// 设置被更新的字段
	// 当数据的更新类型为CREATE时调用该函数无效
	void setUpdatedField(uint32 fieldIndex) { m_updateMask.setBit(fieldIndex); }
	bool hasUpdatedField(uint32 fieldIndex) const { return m_updateMask.testBit(fieldIndex); }
	void clearUpdateFlags() { m_updateMask.reset(); }
	std::string getUpdateFlagsString() const { return m_updateMask.description(); };

protected:
	FieldUpdateMask m_updateMask;
};

NS_END

#endif // __UPDATABLE_H__
