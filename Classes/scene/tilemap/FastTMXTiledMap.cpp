/****************************************************************************
Copyright (c) 2009-2010 Ricardo Quesada
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2011      Zynga Inc.
Copyright (c) 2013-2016 Chukong Technologies Inc.
Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/
#include "FastTMXTiledMap.h"

#include "FastTMXLayer.h"
#include "base/ccUTF8.h"

NS_BEGIN

// implementation FastTMXTiledMap

TMXTiledMap * TMXTiledMap::create(const std::string& tmxFile)
{
    TMXTiledMap *ret = new (std::nothrow) TMXTiledMap();
    if (ret->initWithTMXFile(tmxFile))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

TMXTiledMap* TMXTiledMap::createWithXML(const std::string& tmxString, const std::string& resourcePath)
{
    TMXTiledMap *ret = new (std::nothrow) TMXTiledMap();
    if (ret->initWithXML(tmxString, resourcePath))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}


TMXTiledMap* TMXTiledMap::createWithMapInfo(TMXMapInfo* mapInfo)
{
	TMXTiledMap *ret = new (std::nothrow) TMXTiledMap();
	if (ret->initWithMapInfo(mapInfo))
	{
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

bool TMXTiledMap::initWithTMXFile(const std::string& tmxFile)
{
	NS_ASSERT_LOG(tmxFile.size()>0, "FastTMXTiledMap: tmx file should not be empty");
    
    setContentSize(Size::ZERO);

    TMXMapInfo *mapInfo = TMXMapInfo::create(tmxFile);

    if (! mapInfo)
    {
        return false;
    }
	NS_ASSERT_LOG( !mapInfo->getTilesets().empty(), "FastTMXTiledMap: Map not found. Please check the filename.");
    buildWithMapInfo(mapInfo);

    return true;
}

bool TMXTiledMap::initWithXML(const std::string& tmxString, const std::string& resourcePath)
{
    setContentSize(Size::ZERO);

    TMXMapInfo *mapInfo = TMXMapInfo::createWithXML(tmxString, resourcePath);

	NS_ASSERT_LOG( !mapInfo->getTilesets().empty(), "FastTMXTiledMap: Map not found. Please check the filename.");
    buildWithMapInfo(mapInfo);

    return true;
}


bool TMXTiledMap::initWithMapInfo(TMXMapInfo* mapInfo)
{
	setContentSize(Size::ZERO);

	NS_ASSERT_LOG(!mapInfo->getTilesets().empty(), "FastTMXTiledMap: Map not found. Please check the filename.");
	buildWithMapInfo(mapInfo);

	return true;
}

TMXTiledMap::TMXTiledMap()
    :m_mapInfo(nullptr)
{
}

TMXTiledMap::~TMXTiledMap()
{
	CC_SAFE_RELEASE_NULL(m_mapInfo);

	for (auto it = _layers.begin(); it != _layers.end(); )
	{
		auto layer = (*it).second;
		it = _layers.erase(it);
		CC_SAFE_RELEASE(layer);
	}
}

// private
TMXLayer * TMXTiledMap::parseLayer(TMXLayerInfo *layerInfo, TMXMapInfo *mapInfo)
{
    TMXTilesetInfo *tileset = tilesetForLayer(layerInfo, mapInfo);
    if (tileset == nullptr)
        return nullptr;
    
    TMXLayer *layer = TMXLayer::create(tileset, layerInfo, mapInfo);
	layer->setupTiles();

    return layer;
}

TMXTilesetInfo * TMXTiledMap::tilesetForLayer(TMXLayerInfo *layerInfo, TMXMapInfo *mapInfo)
{
    Size size = layerInfo->_layerSize;
    auto& tilesets = mapInfo->getTilesets();

    for (auto iter = tilesets.crbegin(), iterCrend = tilesets.crend(); iter != iterCrend; ++iter)
    {
        TMXTilesetInfo* tilesetInfo = *iter;
        if (tilesetInfo)
        {
            for( int y=0; y < size.height; y++ )
            {
                for( int x=0; x < size.width; x++ )
                {
                    uint32_t pos = static_cast<uint32_t>(x + size.width * y);
                    uint32_t gid = layerInfo->_tiles[ pos ];
                    
                    // gid are stored in little endian.
                    // if host is big endian, then swap
                    //if( o == CFByteOrderBigEndian )
                    //    gid = CFSwapInt32( gid );
                    /* We support little endian.*/
                    
                    // FIXME: gid == 0 --> empty tile
                    if( gid != 0 )
                    {
                        // Optimization: quick return
                        // if the layer is invalid (more than 1 tileset per layer) an CCAssert will be thrown later
                        if( (gid & kTMXFlippedMask)
                            >= static_cast<uint32_t>(tilesetInfo->_firstGid))
                        {
                            return tilesetInfo;
                        }
                    }
                }
            }
        }
    }

    // If all the tiles are 0, return empty tileset
    CCLOG("cocos2d: Warning: TMX Layer '%s' has no tiles", layerInfo->_name.c_str());
    return nullptr;
}

void TMXTiledMap::buildWithMapInfo(TMXMapInfo* mapInfo)
{
	m_mapInfo = mapInfo;
	CC_SAFE_RETAIN(mapInfo);

    int idx=0;

    auto& layers = mapInfo->getLayers();
    for(const auto &layerInfo : layers) {
        if (layerInfo->_visible)
        {
            TMXLayer *child = parseLayer(layerInfo, mapInfo);
            if (child == nullptr) {
                idx++;
                continue;
            }
            addChild(child, idx, idx);
			_layers[child->getLayerName()] = child;
			CC_SAFE_RETAIN(child);
            
            // update content size with the max size
            const Size& childSize = child->getContentSize();
            Size currentSize = this->getContentSize();
            currentSize.width = std::max( currentSize.width, childSize.width );
            currentSize.height = std::max( currentSize.height, childSize.height );
            this->setContentSize(currentSize);
            
            idx++;
        }
    }
}

// public
TMXLayer * TMXTiledMap::getLayer(const std::string& layerName) const
{
	CCASSERT(layerName.size() > 0, "Invalid layer name!");

	auto it = _layers.find(layerName);
	if (it != _layers.end())
		return (*it).second;

	return nullptr;
}

TMXObjectGroup* TMXTiledMap::getObjectGroup(const std::string& groupName) const
{
	NS_ASSERT_LOG(groupName.size() > 0, "Invalid group name!");

	const Vector<TMXObjectGroup*>& objectGroups = m_mapInfo->getObjectGroups();
    if (objectGroups.size()>0)
    {
        for (const auto& objectGroup : objectGroups)
        {
            if (objectGroup && objectGroup->getGroupName() == groupName)
            {
                return objectGroup;
            }
        }
    }

    // objectGroup not found
    return nullptr;
}

Value TMXTiledMap::getProperty(const std::string& propertyName) const
{
	const ValueMap& properties = m_mapInfo->getProperties();
    if (properties.find(propertyName) != properties.end())
        return properties.at(propertyName);
    
    return Value();
}

Value TMXTiledMap::getPropertiesForGID(int GID) const
{
	const ValueMapIntKey& tileProperties = m_mapInfo->getTileProperties();
    if (tileProperties.find(GID) != tileProperties.end())
        return tileProperties.at(GID);
    
    return Value();
}

const cocos2d::Size& TMXTiledMap::getMapSize() const
{
	return m_mapInfo->getMapSize();
}

void TMXTiledMap::setMapSize(const cocos2d::Size& mapSize)
{
	m_mapInfo->setMapSize(mapSize);
}

const cocos2d::Size& TMXTiledMap::getTileSize() const
{
	return m_mapInfo->getTileSize();
}

void TMXTiledMap::setTileSize(const cocos2d::Size& tileSize)
{
	m_mapInfo->setTileSize(tileSize);
}

int TMXTiledMap::getMapOrientation() const
{
	return m_mapInfo->getOrientation();
}

void TMXTiledMap::setMapOrientation(int mapOrientation)
{
	m_mapInfo->setOrientation(mapOrientation);
}

Vector<TMXObjectGroup*>& TMXTiledMap::getObjectGroups()
{
	return m_mapInfo->getObjectGroups();
}

const Vector<TMXObjectGroup*>& TMXTiledMap::getObjectGroups() const
{
	return m_mapInfo->getObjectGroups();
}

void TMXTiledMap::setObjectGroups(const Vector<TMXObjectGroup*>& groups)
{
	m_mapInfo->setObjectGroups(groups);
}

const cocos2d::ValueMap& TMXTiledMap::getProperties() const
{
	return m_mapInfo->getProperties();
}

void TMXTiledMap::setProperties(const ValueMap& properties)
{
	m_mapInfo->setProperties(properties);
}

std::string TMXTiledMap::getDescription() const
{
    return StringUtils::format("<FastTMXTiledMap | Tag = %d, Layers = %d", _tag, static_cast<int>(_children.size()));
}

void TMXTiledMap::removeChild(Node* child, bool cleanup)
{
	Node::removeChild(child, cleanup);

	TMXLayer* layer = dynamic_cast<TMXLayer*>(child);
	if (layer)
	{
		auto it = _layers.find(layer->getLayerName());
		if (it != _layers.end())
		{
			layer = (*it).second;
			_layers.erase(it);
			CC_SAFE_RELEASE(layer);
		}
	}
}

NS_END

