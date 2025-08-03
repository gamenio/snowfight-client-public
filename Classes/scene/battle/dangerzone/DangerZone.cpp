#include "DangerZone.h"

#include "CloudCluster.h"
#include "common/utils/TimeUtil.h"
#include "common/utils/MathTools.h"

NS_BEGIN

#if NS_DEBUG
#define DEBUG_TILES_ON_VIEWPORT				0
#define DEBUG_SAFE_ZONE						1
#endif // NS_DEBUG

#define VISIBLE_TILES_EXTENDED_RANGE			1
#define CLOUD_CLUSTER_OFFSET					Vec2(0, 32)	// 云团相对地图瓦片的偏移

DangerZone* DangerZone::create(MapData* mapData, Rect const& viewport)
{
	DangerZone* pRet = new DangerZone();
	if (pRet && pRet->init(mapData, viewport))
	{
		pRet->autorelease();
		return pRet;
	}
	else
	{
		CC_SAFE_DELETE(pRet);
		return nullptr;
	}
}

bool DangerZone::init(MapData* mapData, Rect const& viewport)
{
	if (!Node::init())
		return false;

	m_mapData = mapData;
	CC_SAFE_RETAIN(mapData);
	m_viewport = viewport;

	m_visibleCloudFlags = new bool[(int32)(m_mapData->getMapSize().width * m_mapData->getMapSize().height)]();
	CloudClusterPool::getInstance()->setObjectPoolListener(this);

	this->setIgnoreAnchorPointForPosition(false);
	this->setAnchorPoint(Point::ANCHOR_MIDDLE);

#if NS_DEBUG
	m_debugDraw = DebugDrawer::instance()->getDrawByTag("DangerZone");
	CC_SAFE_RETAIN(m_debugDraw);
#endif // NS_DEBUG

	this->initCloudsOnViewport();

	return true;
}

DangerZone::DangerZone() :
	m_debugDraw(nullptr),
	m_mapData(nullptr),
	m_safeZone(nullptr),
	m_visibleCloudFlags(nullptr),
	m_currSafeZoneRadius(0),
	m_isStopped(true),
	m_startTime(0),
	m_elapsed(0),
	m_lastUpdateTime(0)
{
}

DangerZone::~DangerZone()
{
	this->unscheduleUpdate();
	CloudClusterPool::destroyInstance();

	CC_SAFE_DELETE_ARRAY(m_visibleCloudFlags);
	CC_SAFE_RELEASE_NULL(m_debugDraw);
	CC_SAFE_RELEASE_NULL(m_mapData);
	CC_SAFE_RELEASE_NULL(m_safeZone);
}

void DangerZone::setSafeZone(SafeZone* safeZone)
{
	CC_SAFE_RETAIN(safeZone);
	CC_SAFE_RELEASE_NULL(m_safeZone);
	m_safeZone = safeZone;

	int32 nTiles = (int32)(m_mapData->getMapSize().width * m_mapData->getMapSize().height);

	std::fill(m_visibleCloudFlags, m_visibleCloudFlags + nTiles, false);
	m_cloudClusters.clear();
	CloudClusterPool::getInstance()->reset();
}

bool DangerZone::isSafeZoneReady() const
{
	return m_safeZone && !m_safeZone->getCloudDataSet().empty();
}

void DangerZone::addSafeZoneListener(SafeZoneListener* listener)
{
	auto it = std::find(m_safeZoneListeners.begin(), m_safeZoneListeners.end(), listener);
	if (it == std::end(m_safeZoneListeners))
		m_safeZoneListeners.push_back(listener);
}

void DangerZone::removeSafeZoneListener(SafeZoneListener* listener)
{
	m_safeZoneListeners.erase(std::remove(m_safeZoneListeners.begin(), m_safeZoneListeners.end(), listener), m_safeZoneListeners.end());
}

void DangerZone::start(NSTime sartTime)
{
	if (!m_isStopped)
		return;

	NS_ASSERT(m_safeZone && !m_safeZone->getCloudDataSet().empty());

	m_startTime = sartTime;
	m_lastUpdateTime = -1;
	m_currSafeZoneRadius = m_safeZone->getRadius();

	this->scheduleUpdate();
	m_isStopped = false;
	for (SafeZoneListener* listen : m_safeZoneListeners)
		listen->onSafeZoneStarted(m_safeZone->getCenter(), m_safeZone->getRadius());
}

void DangerZone::stop()
{
	if (m_isStopped)
		return;

	this->unscheduleUpdate();
	m_isStopped = true;

	for (SafeZoneListener* listen : m_safeZoneListeners)
		listen->onSafeZoneStopped();
}

void DangerZone::update(float delta)
{
	if (m_isStopped)
		return;

	NSTime currTime = time_util::getUptimeMillis();

	if (m_startTime == -1)
		m_startTime = currTime;
	m_elapsed = currTime - m_startTime;

	if (m_lastUpdateTime == -1)
		m_lastUpdateTime = currTime;
	float diff = (currTime - m_lastUpdateTime) / 1000.f;
	m_lastUpdateTime = currTime;

#if NS_DEBUG
	if(m_debugDraw)
		m_debugDraw->clear();
#endif

	this->updateCloudOnViewport(diff);
	this->updateSafeZone(diff);

	this->cleanupCloudsOutsideViewport();
}

void DangerZone::onEnter()
{
	Node::onEnter();
}

void DangerZone::initCloudsOnViewport()
{
	int32 rows = (int32)std::ceil(m_viewport.size.height / m_mapData->getTileSize().height) + 2;
	int32 columns = (int32)std::ceil(m_viewport.size.width / m_mapData->getTileSize().width) + 2;
	int32 nTiles = rows * columns;
	nTiles += rows * 2 * VISIBLE_TILES_EXTENDED_RANGE + columns * 2 * VISIBLE_TILES_EXTENDED_RANGE;

	CloudClusterPool::getInstance()->init(nTiles, nTiles / 3);
	CCLOG("MaxNumOfTilesOnViewport: %d", nTiles);
}

void DangerZone::drawRhombus(Brush* drawNode, Vec2 const& origin, Size const& size, Color4F const& color)
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

void DangerZone::drawSafeZoneBorder(Brush* drawNode, TileCoord const& center, int32 radius, Color4F const& color)
{
	float widthHalf = radius * m_mapData->getTileSize().width;
	float heightHalf = radius * m_mapData->getTileSize().height;
	Point centerPos = center.computePosition(m_mapData->getMapSize());
	drawNode->drawPoint(centerPos, 10, Color4F::GREEN);
	Point origin;
	origin.x = centerPos.x - widthHalf;
	origin.y = centerPos.y - heightHalf;
	origin.y -= m_mapData->getTileSize().height / 2;
	Size size;
	size.width = widthHalf * 2;
	size.height = heightHalf * 2;
	this->drawRhombus(drawNode, origin, size, color);
}

void DangerZone::drawTileBorder(Brush* drawNode, TileCoord const& coord, Color4F const& color)
{
	Point center = coord.computePosition(m_mapData->getMapSize());
	this->drawRhombus(drawNode, center - m_mapData->getTileSize() / 2, m_mapData->getTileSize(), color);
	//m_debugDraw->drawPoint(center, 2, Color4F::MAGENTA);
	//CCLOG("tile: %d, %d", coord.x, coord.y);
}
void DangerZone::updateSafeZone(float delta)
{
	float scale = std::min(1.0f, m_elapsed / (float)m_safeZone->getDuration());
	int32 currRadius = m_safeZone->getRadius() - (int32)(m_safeZone->getRadius() * scale);
	if (currRadius != m_currSafeZoneRadius)
	{
		m_currSafeZoneRadius = currRadius;
		for (SafeZoneListener* listen : m_safeZoneListeners)
			listen->onSafeZoneUpdated(m_currSafeZoneRadius);
	}

#if DEBUG_SAFE_ZONE
	if (m_debugDraw)
	{
		this->drawSafeZoneBorder(m_debugDraw, m_safeZone->getCenter(), m_safeZone->getRadius(), Color4F::GRAY);
		if (currRadius > 0)
			this->drawSafeZoneBorder(m_debugDraw, m_safeZone->getCenter(), currRadius, Color4F::RED);
		else
		{
			Point centerPos = m_safeZone->getCenter().computePosition(m_mapData->getMapSize());
			m_debugDraw->drawPoint(centerPos, 5, Color4F::RED);
		}
	}
#endif // DEBUG_SAFE_ZONE
}

void DangerZone::updateCloudOnViewport(float delta)
{
	Node* parent = this->getParent();
	if (!parent)
		return;

	int32 nFlags = (int32)(m_mapData->getMapSize().width * m_mapData->getMapSize().height);
	std::fill(m_visibleCloudFlags, m_visibleCloudFlags + nFlags, false);

	Point ctr;
	ctr.x = m_viewport.getMidX() - parent->getBoundingBox().getMinX() - CLOUD_CLUSTER_OFFSET.x;
	ctr.y = m_viewport.getMidY() - parent->getBoundingBox().getMinY() - CLOUD_CLUSTER_OFFSET.y;

	TileCoord center(m_mapData->getMapSize(), ctr);
	TileCoord leftTop;
	TileCoord leftBottom;
	TileCoord rightTop;
	TileCoord rightBottom;

	int32 rows = (int32)std::ceil(m_viewport.size.height / m_mapData->getTileSize().height) + 1;
	int32 columns = (int32)std::ceil(m_viewport.size.width / m_mapData->getTileSize().width) + 1;
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

	leftTop.x = leftTop.x - VISIBLE_TILES_EXTENDED_RANGE;
	rightTop.y = rightTop.y - VISIBLE_TILES_EXTENDED_RANGE;
	leftBottom.y = leftBottom.y + VISIBLE_TILES_EXTENDED_RANGE;
	rightBottom.x = rightBottom.x + VISIBLE_TILES_EXTENDED_RANGE;

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
				int32 tileIndex = this->getTileIndex(coord);
				m_visibleCloudFlags[tileIndex] = true;
				this->updateCloud(coord, delta);

			}

#if DEBUG_TILES_ON_VIEWPORT
			if (m_debugDraw)
				this->drawTileBorder(m_debugDraw, coord, Color4F::MAGENTA);
#endif
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
}

void DangerZone::updateCloud(TileCoord const& coord, float delta)
{
	NS_ASSERT(coord.x >= 0 && coord.x < m_mapData->getMapSize().width && coord.y >= 0 && coord.y < m_mapData->getMapSize().height);

	int32 tileIndex = this->getTileIndex(coord);
	std::vector<CloudData*> const& cloudDataSet = m_safeZone->getCloudDataSet();
	CloudData* data = cloudDataSet.at(tileIndex);
	if (!data)
		return;

	CloudCluster* cloudCluster;
	auto it = m_cloudClusters.find(tileIndex);
	if (it == m_cloudClusters.end())
	{
		float updateDt = m_elapsed / 1000.f - (data->time + data->delay);
		// 未到激活时间
		if (updateDt < 0)
			return;

		// 忽略超过循环持续时间的云数据
		if (data->loops > 0)
		{
			if(data->afterLoopsAction == CloudData::ACTION_STOP)
			{
				float duration = data->loops * CloudCluster::ACTIVE_CLOUD_DURATION;
				if (updateDt >= duration)
					return;
			}
		}

		cloudCluster = CloudClusterPool::getInstance()->take();
		NS_ASSERT(cloudCluster);
		NS_ASSERT(cloudCluster->getParent() != nullptr);
		m_cloudClusters.insert(tileIndex, cloudCluster);

		cloudCluster->setData(data);
		cloudCluster->setPosition(coord.computePosition(m_mapData->getMapSize()) + CLOUD_CLUSTER_OFFSET);
		int32 zOrder = this->getVertexZForTile(coord);
		cloudCluster->setLocalZOrder(zOrder);

		cloudCluster->activate();
		cloudCluster->update(0);
		if (updateDt > 0)
			cloudCluster->update(updateDt);
	}
	else
	{
		cloudCluster = (*it).second;
		cloudCluster->update(delta);
		NS_ASSERT(cloudCluster->getData()->tileCoord == coord);
	}
}

void DangerZone::cleanupCloudsOutsideViewport()
{
	for (auto it = m_cloudClusters.begin(); it != m_cloudClusters.end();)
	{
		int32 tileIndex = (*it).first;
		CloudCluster* cloudCluster = (*it).second;
		if (!m_visibleCloudFlags[tileIndex] || !cloudCluster->isActivated())
		{
			CloudClusterPool::getInstance()->put(cloudCluster);
			it = m_cloudClusters.erase(it);
		}
		else
			++it;
	}
}

int32 DangerZone::getTileIndex(TileCoord const& coord)
{
	int32 tileIndex = coord.x * (int32)m_mapData->getMapSize().height + coord.y;
	return tileIndex;
}

int32 DangerZone::getVertexZForTile(TileCoord const& coord)
{
	int32 maxVal = static_cast<int32>(m_mapData->getMapSize().width + m_mapData->getMapSize().height);
	int32 ret = static_cast<int32>(-(maxVal - (coord.x + coord.y)));

	return ret;
}

void DangerZone::onObjectPoolIncreased(std::unordered_set<CloudCluster*> const& objects)
{
	for (auto it = objects.begin(); it != objects.end(); ++it)
	{
		CloudCluster* cloudCluster = *it;
		NS_ASSERT(cloudCluster->getParent() == nullptr);
		this->addChild(cloudCluster);
	}
}

NS_END
