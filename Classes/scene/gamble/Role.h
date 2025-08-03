#ifndef __ROLE_H__
#define __ROLE_H__

#include "common/Common.h"
#include "game/entities/DataUnit.h"
#include "GameObject.h"

USING_NS_CC;

NS_BEGIN

class Role : public GameObject
{
public:
	Role();
	virtual ~Role();

	virtual bool init() override;
	virtual bool initWithData(DataUnit* data);

	virtual void hurl() { }
	virtual DataUnit* getData() const override { return m_data; }

protected:
	DataUnit* m_data;
};

NS_END


#endif //__ROLE_H__
