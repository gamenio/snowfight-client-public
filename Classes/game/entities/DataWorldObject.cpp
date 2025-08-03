#include "DataWorldObject.h"

NS_BEGIN

DataWorldObject::DataWorldObject():
	m_mapData(nullptr),
	m_objectSize(Size::ZERO),
	m_anchorPoint(Point::ZERO),
	m_objectRadiusInMap(0)
{
	m_type |= DataTypeMask::DATA_TYPEMASK_WORLDOBJECT;
	m_typeId = DataTypeID::DATA_TYPEID_WORLDOBJECT;
}


DataWorldObject::~DataWorldObject()
{
	CC_SAFE_RELEASE_NULL(m_mapData);
}


void DataWorldObject::setMapData(MapData* mapData)
{
	CC_SAFE_RETAIN(mapData);
	CC_SAFE_RELEASE_NULL(m_mapData);
	m_mapData = mapData;
}

Rect DataWorldObject::getBoundingBox() const
{
    return Rect(this->getPosition().x - (m_objectSize.width * m_anchorPoint.x),
				this->getPosition().y - (m_objectSize.height * m_anchorPoint.y),
                m_objectSize.width,
                m_objectSize.height);
}

NS_END