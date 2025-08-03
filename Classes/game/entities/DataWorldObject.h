#ifndef __DATA_WORLDOBJECT_H__
#define __DATA_WORLDOBJECT_H__

#include "game/entities/DataBasic.h"
#include "game/maps/MapData.h"

USING_NS_CC;

NS_BEGIN

class DataWorldObject: public DataBasic
{
public:
	DataWorldObject();
	virtual ~DataWorldObject();

	void clearFields() override {}
	bool readFields(UpdateType updateType, uint32 updateFlags, FieldUpdateMask const& updateMask, DataInputStream* input) override { return true; }

	void setMapData(MapData* mapData);
	MapData* getMapData() { return m_mapData; }
	MapData const* getMapData() const { return m_mapData; }

	// 获取对象的世界坐标
	virtual cocos2d::Point const& getPosition() const = 0;
	
	void setAnchorPoint(cocos2d::Point const& anchor) { m_anchorPoint = anchor; }
	cocos2d::Point const& getAnchorPoint() const { return m_anchorPoint; }
	virtual cocos2d::Rect getBoundingBox() const;
	cocos2d::Size getObjectSize() const { return m_objectSize; }
	void setObjectSize(cocos2d::Size const& size) { m_objectSize = size; }

	// 对象在地图里的半径
	float getObjectRadiusInMap() const { return m_objectRadiusInMap; };
	void setObjectRadiusInMap(float radius) { m_objectRadiusInMap = radius; }

protected:
	MapData* m_mapData;

	cocos2d::Point m_anchorPoint;
	cocos2d::Size m_objectSize;
	float m_objectRadiusInMap;
};

NS_END

#endif // __DATA_WORLDOBJECT_H__