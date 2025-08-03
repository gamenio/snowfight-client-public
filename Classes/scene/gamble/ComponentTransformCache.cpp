#include "ComponentTransformCache.h"

#include "json/error/en.h"
#include "json/error/error.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#ifdef GetObject
#undef GetObject
#endif

#endif

NS_BEGIN

static ComponentTransformCache *s_SharedManager = nullptr;

ComponentTransformCache* ComponentTransformCache::getInstance()
{
	if (!s_SharedManager)
	{
		s_SharedManager = new (std::nothrow) ComponentTransformCache;
		NS_ASSERT_LOG(s_SharedManager, "FATAL: Not enough memory");
	}

	return s_SharedManager;
}

void ComponentTransformCache::destroyInstance()
{
	CC_SAFE_DELETE(s_SharedManager);
}

void ComponentTransformCache::addComponentTransfromsWithFile(std::string const& jsonFile)
{
	rapidjson::Document document;
	bool ret = this->praseJSONData(document, jsonFile);
	if (ret)
	{
		if (document.IsObject() && document.HasMember("components"))
		{
			auto& value = document["components"];
			if (value.IsArray())
			{
				auto componentList = value.GetArray();
				for (auto& comp : componentList)
				{
					auto componentType = comp["component_type"].GetInt();
					auto scale = comp["scale"].GetFloat();
					auto transformList = comp["transforms"].GetArray();
					for (auto& trans : transformList)
					{
						auto frameName = trans["frame_name"].GetString();
						auto position = trans["position"].GetObject();
						float x = position["x"].GetFloat();
						float y = position["y"].GetFloat();
						Transform& compTrans = m_transforms[frameName][componentType];
						compTrans.position.x = x / scale;
						compTrans.position.y = y / scale;
						compTrans.source = jsonFile;
					}
				}
			}
		}
	}
}

void ComponentTransformCache::removeComponentTransfromsFromFile(std::string const& jsonFile)
{
	for (auto it = m_transforms.begin(); it != m_transforms.end();)
	{
		auto& compTransMap = (*it).second;
		for (auto compTransIt = compTransMap.begin(); compTransIt != compTransMap.end();)
		{
			auto const& transform = (*compTransIt).second;
			if (transform.source == jsonFile)
				compTransIt = compTransMap.erase(compTransIt);
			else
				++compTransIt;
		}

		if (compTransMap.empty())
			it = m_transforms.erase(it);
		else
			++it;
	}
}

Point const& ComponentTransformCache::getComponentPosition(int32 componentType, std::string const& frameName)
{
	Transform* transfrom = this->getTransform(componentType, frameName);
	if (transfrom)
		return (*transfrom).position;

	return Point::ZERO;
}

ComponentTransformCache::ComponentTransformCache()
{
}


ComponentTransformCache::~ComponentTransformCache()
{
}

bool ComponentTransformCache::praseJSONData(rapidjson::Document& doc, std::string const& jsonFile)
{
	if (FileUtils::getInstance()->isFileExist(jsonFile))
	{
		std::string data = FileUtils::getInstance()->getStringFromFile(jsonFile);
		rapidjson::ParseResult ret = doc.Parse<rapidjson::kParseCommentsFlag | rapidjson::kParseTrailingCommasFlag>(data.c_str());
		if (ret)
			return true;
		else
			CCLOG("ComponentTransformCache: JSON '%s' parsing error: %s (%u)", jsonFile.c_str(), rapidjson::GetParseError_En(ret.Code()), (uint32)ret.Offset());
	}
	else
		CCLOG("ComponentTransformCache: JSON '%s' does not exist.", jsonFile.c_str());

	return false;
}

ComponentTransformCache::Transform* ComponentTransformCache::getTransform(int32 componentType, std::string const& frameName)
{
	auto it = m_transforms.find(frameName);
	if (it != m_transforms.end())
	{
		ComponentTransformMap& compTransMap = (*it).second;
		auto compTransIt = compTransMap.find(componentType);
		if (compTransIt != compTransMap.end())
		{
			return &((*compTransIt).second);
		}
	}
	return nullptr;
}

NS_END