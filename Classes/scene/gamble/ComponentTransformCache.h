#ifndef __COMPONENT_TRANSFORM_CACHE_H__
#define __COMPONENT_TRANSFORM_CACHE_H__

#include "json/document-wrapper.h"

#include "common/Common.h"

USING_NS_CC;


NS_BEGIN

class ComponentTransformCache
{
	struct Transform
	{
		cocos2d::Point position;
		std::string source;
	};

	typedef std::unordered_map<int32 /* Component Type */, Transform> ComponentTransformMap;

public:
	static ComponentTransformCache* getInstance();
	static void destroyInstance();

	void addComponentTransfromsWithFile(std::string const& jsonFile);
	void removeComponentTransfromsFromFile(std::string const& jsonFile);

	cocos2d::Point const& getComponentPosition(int32 componentType, std::string const& frameName);

private:
	ComponentTransformCache();
	~ComponentTransformCache();

	bool praseJSONData(rapidjson::Document& doc, std::string const& jsonFile);
	Transform* getTransform(int32 componentType, std::string const& frameName);

	std::unordered_map<std::string /* Frame Name */, ComponentTransformMap> m_transforms;
};

NS_END

#endif // __COMPONENT_TRANSFORM_CACHE_H__
