#include "HidingSpotSearcher.h"

#include "common/utils/TimeUtil.h"

#define DEBUG_TILES_ON_VIEWPORT				1
#define VISIBLE_TILES_EXTENDED_RANGE		1

NS_BEGIN

class HidingSpotSearcher::HidingSpotInfoRef
{
public:
	HidingSpotInfoRef(HidingSpotInfo const& target) :
		info(target),
		refCount(0)
	{
	}
	HidingSpotInfo info;
	int32 refCount;
};

HidingSpotSearcher::HidingSpotSearcher(MapData* mapData) :
	m_debugDraw(nullptr),
	m_mapData(mapData),
	m_visibleRange(POINT),
	m_extendedRangeInTiles(0),
	m_visibleHidingSpotFlags(nullptr)
{
	m_visibleHidingSpotFlags = new bool[(int32)(m_mapData->getMapSize().width * m_mapData->getMapSize().height)]();
	CC_SAFE_RETAIN(mapData);
}

HidingSpotSearcher::~HidingSpotSearcher()
{
	CC_SAFE_RELEASE_NULL(m_debugDraw);
	CC_SAFE_RELEASE_NULL(m_mapData);
	CC_SAFE_DELETE_ARRAY(m_visibleHidingSpotFlags);
}

void HidingSpotSearcher::setDebugDraw(DrawNode* node)
{
	CC_SAFE_RETAIN(node);
	CC_SAFE_RELEASE_NULL(m_debugDraw);
	m_debugDraw = node;
}

void HidingSpotSearcher::update(TileCoord const& center, std::vector<HidingSpotInfo>& visibleList, std::vector<HidingSpotInfo>& outOfRangeList)
{
	this->updateHidingSpotOnVisibleRange(center, visibleList);
	this->cleanupHidingSpotsOutsideVisibleRange(outOfRangeList);
}

void HidingSpotSearcher::setVisibleRange(VisibleRange range)
{
	if (m_visibleRange == range)
		return;

	int32 nFlags = (int32)(m_mapData->getMapSize().width * m_mapData->getMapSize().height);
	std::fill(m_visibleHidingSpotFlags, m_visibleHidingSpotFlags + nFlags, false);

	if (range == VIEWPORT)
	{
		m_viewport = Director::getInstance()->getVisibleSize();
		m_extendedRangeInTiles = VISIBLE_TILES_EXTENDED_RANGE;
	}
	else
	{
		m_viewport = Size::ZERO;
		m_extendedRangeInTiles = 0;
	}

	m_visibleRange = range;
}

void HidingSpotSearcher::drawRhombus(DrawNode* drawNode, Vec2 const& origin, Size const& size, Color4F const& color)
{
	float widthHalf = size.width / 2;
	float heightHalf = size.height / 2;
	Point left(origin.x, origin.y + heightHalf);
	Point top(left.x + widthHalf, left.y + heightHalf);
	Point right(top.x + widthHalf, left.y);
	Point bottom(top.x, origin.y);
	drawNode->drawLine(left, top, color);
	drawNode->drawLine(top, right, color);
	drawNode->drawLine(right, bottom, color);
	drawNode->drawLine(bottom, left, color);
}

void HidingSpotSearcher::drawTileBorder(DrawNode* drawNode, TileCoord const& coord, Color4F const& color)
{
	Point center = coord.computePosition(m_mapData->getMapSize());
	this->drawRhombus(drawNode, center - m_mapData->getTileSize() / 2, m_mapData->getTileSize(), color);
}

void HidingSpotSearcher::updateHidingSpotOnVisibleRange(TileCoord const& center, std::vector<HidingSpotInfo>& visibleList)
{
	//auto currTime = time_util::getHighResolutionTimeMillis();

	int32 nFlags = (int32)(m_mapData->getMapSize().width * m_mapData->getMapSize().height);
	std::fill(m_visibleHidingSpotFlags, m_visibleHidingSpotFlags + nFlags, false);

	TileCoord leftTop;
	TileCoord leftBottom;
	TileCoord rightTop;
	TileCoord rightBottom;

	int32 rows = (int32)std::ceil(m_viewport.height / m_mapData->getTileSize().height) + 1;
	int32 columns = (int32)std::ceil(m_viewport.width / m_mapData->getTileSize().width) + 1;
	int32 halfRows = (int32)(rows / 2);
	int32 halfColumns = (int32)(columns / 2);
	leftTop.x = center.x - halfColumns - halfRows;
	leftTop.y = center.y + halfColumns - halfRows;
	leftBottom.x = center.x - halfColumns + halfRows;
	leftBottom.y = center.y + halfColumns + halfRows;
	rightTop.x = center.x + halfColumns - halfRows;
	rightTop.y = center.y - halfColumns - halfRows;
	rightBottom.x = center.x + halfColumns + halfRows;
	rightBottom.y = center.y - halfColumns + halfRows;

	leftTop.x = leftTop.x - m_extendedRangeInTiles;
	rightTop.y = rightTop.y - m_extendedRangeInTiles;
	leftBottom.y = leftBottom.y + m_extendedRangeInTiles;
	rightBottom.x = rightBottom.x + m_extendedRangeInTiles;

	int32 dy = leftBottom.y - leftTop.y;
	int32 dx = rightTop.x - leftTop.x;
	int32 nRows = dy + 1 + dy;
	int32 count = 0;
	for (int32 i = 0; i < nRows; i++)
	{
		int32 col;
		int32 k = i / 2;
		int32 h = 0;
		if (i % 2 == 0)
		{
			col = dx + 1;
		}
		else
		{
			col = dx;
			h = 1;
		}
		for (int32 j = 0; j < col; j++)
		{
			count++;
			int32 x = leftBottom.x + j - k;
			int32 y = leftBottom.y - j - k - h;

			TileCoord coord(x, y);
			if (coord.x >= 0 && coord.x < m_mapData->getMapSize().width && coord.y >= 0 && coord.y < m_mapData->getMapSize().height)
			{
				if (m_mapData->isConcealable(coord))
				{
					HidingSpotInfo info;
					if (m_mapData->getHidingSpotInfo(coord, info))
					{
						int32 tileIndex = m_mapData->getTileIndex(coord);
						m_visibleHidingSpotFlags[tileIndex] = true;
						auto it = m_hidingSpots.find(tileIndex);
						if (it == m_hidingSpots.end())
						{
							int32 refIndex = m_mapData->getTileIndex(info.originCoord);
							auto refIt = m_hidingSpotInfoRefs.find(refIndex);
							if (refIt == m_hidingSpotInfoRefs.end())
							{
								auto ret = m_hidingSpotInfoRefs.emplace(refIndex, new HidingSpotInfoRef(info));
								CC_ASSERT(ret.second);
								refIt = ret.first;
							}
							HidingSpotInfoRef* ref = (*refIt).second;
							++ref->refCount;
							m_hidingSpots.emplace(tileIndex, refIndex);
						}
						visibleList.push_back(info);
					}
				}

#if DEBUG_TILES_ON_VIEWPORT
				if (m_debugDraw)
					this->drawTileBorder(m_debugDraw, coord, Color4F::MAGENTA);
#endif
			}
		}
	}

#if DEBUG_TILES_ON_VIEWPORT
	if (m_debugDraw)
	{
		this->drawTileBorder(m_debugDraw, center, Color4F::BLUE);
		this->drawTileBorder(m_debugDraw, leftTop, Color4F::BLUE);
		this->drawTileBorder(m_debugDraw, leftBottom, Color4F::BLUE);
		this->drawTileBorder(m_debugDraw, rightTop, Color4F::BLUE);
		this->drawTileBorder(m_debugDraw, rightBottom, Color4F::BLUE);
	}
#endif

	//CCLOG("updateHidingSpotOnVisibleRange: %f ms", time_util::getHighResolutionTimeMillis() - currTime);
}

void HidingSpotSearcher::cleanupHidingSpotsOutsideVisibleRange(std::vector<HidingSpotInfo>& outOfRangeList)
{
	for (auto it = m_hidingSpots.begin(); it != m_hidingSpots.end();)
	{
		int32 tileIndex = (*it).first;
		if (!m_visibleHidingSpotFlags[tileIndex])
		{
			int32 refIndex = (*it).second;
			auto refIt = m_hidingSpotInfoRefs.find(refIndex);
			NS_ASSERT(refIt != m_hidingSpotInfoRefs.end());
			auto* ref = (*refIt).second;
			--ref->refCount;
			if (ref->refCount <= 0)
			{
				outOfRangeList.push_back(ref->info);
				m_hidingSpotInfoRefs.erase(refIt);
				CC_SAFE_DELETE(ref);
			}
			it = m_hidingSpots.erase(it);
		}
		else
			++it;
	}
}

NS_END