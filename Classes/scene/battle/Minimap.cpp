#include "Minimap.h"

#include "ui/UIScale9Sprite.h"

#include "game/World.h"
#include "scene/AssetsLoader.h"
#include "scene/SoundMgr.h"

NS_BEGIN

using namespace cocos2d::ui;

#define FRAMENAME_FRAME_ZOOMOUT				"minimap_frame_zoomout.png"
#define FRAMENAME_FRAME_ZOOMIN				"minimap_frame_zoomin.png"
#define FRAMENAME_MAP_MASK_ZOOMOUT			"minimap_map_mask_zoomout.png"
#define FRAMENAME_MAP_MASK_ZOOMIN			"minimap_map_mask_zoomin.png"
#define FRAMENAME_ZOOMIN					"minimap_btn_zoomin.png"
#define FRAMENAME_ZOOMOUT					"minimap_btn_zoomout.png"
#define FRAMENAME_DANGER_ZONE				"minimap_danger_zone.png"
#define FRAMENAME_SAFEZONE_BORDER			"minimap_safezone_border.png"

#define MYLOCATOR_FRAME_FORMAT				"guidepost_head%d.png"
#define MYLOCATOR_SIZE						Size(19, 19)

#define FRAME_ZOOMOUT_SCALE					0.342f
#define FRAME_MARGIN_TOP					2
#define ZOOM_BUTTON_ZOOMOUT_SCALE			0.8f

#define SAFEZONE_BORDER_SIZE				1.5f

#define RENDER_SPRITE_OPACITY_ZOOMOUT		(255 * 0.65f)
#define RENDER_SPRITE_OPACITY_ZOOMIN		(255 * 0.8f)

#define MAP_MARGIN							26.8f

#define ACTION_ZOOMING						1
#define ZOOM_DURATION						0.4f
#define PLAYER_COUNTBAR_FADE_DURATION		0.1f

Minimap::Minimap() :
	m_isZoomedIn(false),
	m_isLayoutDirty(false),
	m_mapScaleX(0.f),
	m_mapScaleY(0.f),
	m_safeZoneRadius(-1),
	m_isSafeZoneDirty(false),
	m_playerCountBar(nullptr),
	m_renderTexture(nullptr),
	m_frameClipper(nullptr),
	m_frameSp(nullptr),
	m_zoomBtn(nullptr),
	m_mapSp(nullptr),
	m_mapMaskSp(nullptr),
	m_mapClipper(nullptr),
	m_myLocatorSp(nullptr),
	m_dangerZoneSp(nullptr),
	m_safeZoneBorderSp(nullptr),
	m_dangerZoneClipper(nullptr),
	m_safeZoneClipper(nullptr),
	m_safeZoneMaskSp(nullptr)
{
}

Minimap::~Minimap()
{
	m_playerCountBar = nullptr;
	m_renderTexture = nullptr;
	m_frameClipper = nullptr;
	m_frameSp = nullptr;
	m_zoomBtn = nullptr;
	m_mapSp = nullptr;
	m_mapMaskSp = nullptr;
	m_mapClipper = nullptr;
	m_myLocatorSp = nullptr;
	m_dangerZoneSp = nullptr;
	m_safeZoneBorderSp = nullptr;
	m_dangerZoneClipper = nullptr;
	m_safeZoneClipper = nullptr;
	m_safeZoneMaskSp = nullptr;
}

Minimap* Minimap::create()
{
	Minimap* pRet = new Minimap();
	if (pRet && pRet->init())
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

bool Minimap::init()
{
	if (!Node::init())
		return false;

	DataPlayer* myChar = World::getInstance()->getMyself();
	NS_ASSERT(myChar);
	Texture2D* mapTex = Director::getInstance()->getTextureCache()->getTextureForKey(StringUtils::format(MINIMAP_IMAGE, myChar->getMapData()->getMapId()));
	if (!mapTex)
		return false;

	this->setIgnoreAnchorPointForPosition(false);

	SpriteFrame* frameZoomInFrame = SpriteFrameCache::getInstance()->getSpriteFrameByName(FRAMENAME_FRAME_ZOOMIN);
	SpriteFrame* frameZoomOutFrame = SpriteFrameCache::getInstance()->getSpriteFrameByName(FRAMENAME_FRAME_ZOOMOUT);
	m_frameZoomedInSize = frameZoomInFrame->getOriginalSize();
	m_frameZoomedOutSize = frameZoomOutFrame->getOriginalSize();

	m_playerCountBar = PlayerCountBar::create();
	m_playerCountBar->setAnchorPoint(Point::ANCHOR_TOP_RIGHT);
	this->addChild(m_playerCountBar);

	m_renderTexture = RenderTexture::create(m_frameZoomedInSize.width, m_frameZoomedInSize.height, Texture2D::PixelFormat::RGBA8888, GL_DEPTH24_STENCIL8);
	m_renderTexture->setAutoDraw(false);
	m_renderTexture->setClearColor(Color4F(0.f, 0.f, 0.f, 0.f));
	m_renderTexture->setClearFlags(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	Sprite* renderSprite = m_renderTexture->getSprite();
	renderSprite->setAnchorPoint(Point::ANCHOR_TOP_RIGHT);
	renderSprite->setOpacity(RENDER_SPRITE_OPACITY_ZOOMOUT);
	this->addChild(m_renderTexture);

	m_mapMaskSp = Sprite::createWithSpriteFrameName(FRAMENAME_MAP_MASK_ZOOMOUT);
	m_mapClipper = ClippingNode::create(m_mapMaskSp);
	m_mapClipper->setAlphaThreshold(0.0f);
	m_renderTexture->addChild(m_mapClipper);

	m_mapSp = Sprite::createWithTexture(mapTex);
	m_mapSp->setRotation(45);
	m_mapClipper->addChild(m_mapSp);

	m_myLocatorSp = Sprite::createWithSpriteFrameName(StringUtils::format(MYLOCATOR_FRAME_FORMAT, myChar->getDisplayId()));
	m_myLocatorSp->setContentSize(MYLOCATOR_SIZE);
	m_renderTexture->addChild(m_myLocatorSp);

	m_dangerZoneClipper = ClippingNode::create(m_mapMaskSp);
	m_dangerZoneClipper->setAlphaThreshold(0.0f);
	m_dangerZoneClipper->setVisible(false);
	m_renderTexture->addChild(m_dangerZoneClipper);

	m_safeZoneMaskSp = Scale9Sprite::createWithSpriteFrameName(FRAMENAME_SAFEZONE_BORDER);
	m_safeZoneMaskSp->setRotation(45);
	m_safeZoneClipper = ClippingNode::create(m_safeZoneMaskSp);
	m_safeZoneClipper->setAlphaThreshold(1);
	m_safeZoneClipper->setInverted(true);
	m_dangerZoneClipper->addChild(m_safeZoneClipper);

	m_dangerZoneSp = Scale9Sprite::createWithSpriteFrameName(FRAMENAME_DANGER_ZONE);
	m_dangerZoneSp->setContentSize(m_frameZoomedInSize);
	m_dangerZoneSp->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_safeZoneClipper->addChild(m_dangerZoneSp);

	m_safeZoneBorderSp = Scale9Sprite::createWithSpriteFrameName(FRAMENAME_SAFEZONE_BORDER);
	m_safeZoneBorderSp->setRotation(45);
	m_safeZoneBorderSp->setVisible(false);
	m_dangerZoneClipper->addChild(m_safeZoneBorderSp);

	m_frameSp = Sprite::createWithSpriteFrameName(FRAMENAME_FRAME_ZOOMOUT);
	m_renderTexture->addChild(m_frameSp);

	m_zoomBtn = Button::create(FRAMENAME_ZOOMIN, "", "", Widget::TextureResType::PLIST);
	m_zoomBtn->setAnchorPoint(Point::ANCHOR_MIDDLE);
	m_zoomBtn->setScale(ZOOM_BUTTON_ZOOMOUT_SCALE);
	m_zoomBtn->addClickEventListener(CC_CALLBACK_1(Minimap::buttonZoomCallback, this));
	this->addChild(m_zoomBtn);

	MapData* mapData = myChar->getMapData();
	float mapWidth = mapData->getMapSize().width * mapData->getTileSize().width;
	float mapHeight = mapData->getMapSize().height * mapData->getTileSize().height;
	m_mapScaleX = (m_mapSp->getBoundingBox().size.width - MAP_MARGIN * 2) / mapWidth;
	m_mapScaleY = (m_mapSp->getBoundingBox().size.height - MAP_MARGIN * 2) / mapHeight;

	m_isLayoutDirty = true;

	return true;
}

void Minimap::visit(Renderer* renderer, Mat4 const& parentTransform, uint32_t parentFlags)
{
	if (m_isLayoutDirty)
	{
		this->layoutElements();
		m_isLayoutDirty = false;
	}

	Node::visit(renderer, parentTransform, parentFlags);
}

void Minimap::draw(Renderer* renderer, Mat4 const& transform, uint32_t flags)
{
	Node::draw(renderer, transform, flags);

	m_renderTexture->setAutoDraw(true);
	m_renderTexture->draw(renderer, Mat4::IDENTITY, flags);
	m_renderTexture->setAutoDraw(false);
}

void Minimap::update(float delta)
{
	m_playerCountBar->update(delta);

	DataPlayer* myChar = World::getInstance()->getMyself();
	if (myChar && !this->isZoomAnimating())
	{
		this->updateSafeZone();

		if (myChar->hasUpdatedField(CUNIT_FIELD_POSITION))
		{
			if (m_isZoomedIn)
				m_myLocatorSp->setPosition(this->calcMyLocatorPositionInZoomIn());
			else
			{
				m_mapSp->setPosition(this->calcMapPositionInZoomOut());
				if (m_dangerZoneClipper->isVisible())
				{
					m_safeZoneMaskSp->setPosition(this->calcSafeZonePositionInZoomOut());
					m_safeZoneBorderSp->setPosition(m_safeZoneMaskSp->getPosition());
				}
			}
		}
	}
}

void Minimap::zoomIn()
{
	if (m_isZoomedIn)
		return;

	m_zoomBtn->getRendererNormal()->setSpriteFrame(FRAMENAME_ZOOMOUT);
	this->startZoomInAnimation();
	m_isZoomedIn = true;
}

void Minimap::zoomOut()
{
	if (!m_isZoomedIn)
		return;

	m_zoomBtn->getRendererNormal()->setSpriteFrame(FRAMENAME_ZOOMIN);
	this->startZoomOutAnimation();
	m_isZoomedIn = false;
}

void Minimap::setZoomedInPosition(Point const& position)
{
	m_zoomedInPosition = position;
	m_isLayoutDirty = true;
}

Point Minimap::getZoomButtonZoomedInPosition() const
{
	Point pos;
	pos.x = this->getContentSize().width + m_zoomedInPosition.x - 25;
	pos.y = this->getContentSize().height + m_zoomedInPosition.y - 25;
	return pos;
}

void Minimap::onSafeZoneStarted(TileCoord const& center, int32 initialRadius)
{
	this->setSafeZoneCenter(center);
	this->setSafeZoneRadius(initialRadius);
}

void Minimap::onSafeZoneUpdated(int32 currRadius)
{
	this->setSafeZoneRadius(currRadius);
}

void Minimap::buttonZoomCallback(Ref* sender)
{
	sSoundMgr->play(SOUND_BUTTON);
	if (m_isZoomedIn)
		this->zoomOut();
	else
		this->zoomIn();
}

void Minimap::layoutElements()
{
	float width = m_frameSp->getContentSize().width;
	float height = m_frameSp->getContentSize().height + FRAME_MARGIN_TOP + m_playerCountBar->getContentSize().height;
	this->setContentSize(Size(width, height));

	m_playerCountBar->setPosition(this->getContentSize().width, this->getContentSize().height);
	Sprite* renderSprite = m_renderTexture->getSprite();
	renderSprite->setPosition(this->getContentSize().width + m_zoomedInPosition.x, this->getContentSize().height + m_zoomedInPosition.y);
	Point frameOriginPosition = this->getFrameOriginPosition();
	m_mapMaskSp->setPosition(frameOriginPosition);
	m_frameSp->setPosition(frameOriginPosition);
	m_zoomBtn->setPosition(this->getZoomButtonOriginPosition());
	m_myLocatorSp->setPosition(frameOriginPosition);
	m_mapSp->setPosition(this->calcMapPositionInZoomOut());
	m_dangerZoneSp->setPosition(Point::ZERO);
	m_safeZoneMaskSp->setPosition(Point::ZERO);
	m_safeZoneBorderSp->setPosition(Point::ZERO);
}

Point Minimap::calcMapPositionInZoomOut() const
{
	DataPlayer* myChar = World::getInstance()->getMyself();
	NS_ASSERT(myChar);

	Point frameOriginPosition = this->getFrameOriginPosition();
	Point pos;
	pos.x = frameOriginPosition.x + m_mapSp->getBoundingBox().size.width / 2 - MAP_MARGIN - myChar->getPosition().x * m_mapScaleX;
	pos.y = frameOriginPosition.y + m_mapSp->getBoundingBox().size.height / 2 - MAP_MARGIN - myChar->getPosition().y * m_mapScaleY;
	return pos;
}

Point Minimap::calcMyLocatorPositionInZoomIn() const
{
	DataPlayer* myChar = World::getInstance()->getMyself();
	NS_ASSERT(myChar);

	Point frameZoomedInPosition = this->getFrameZoomedInPosition();
	Point pos;
	pos.x = frameZoomedInPosition.x - m_mapSp->getBoundingBox().size.width / 2 + MAP_MARGIN + myChar->getPosition().x * m_mapScaleX;
	pos.y = frameZoomedInPosition.y - m_mapSp->getBoundingBox().size.height / 2 + MAP_MARGIN + myChar->getPosition().y * m_mapScaleY;
	return pos;
}

Point Minimap::calcSafeZonePositionInZoomOut() const
{
	DataPlayer* myChar = World::getInstance()->getMyself();
	NS_ASSERT(myChar);
	MapData* mapData = myChar->getMapData();

	Point centerPos = m_safeZoneCenter.computePosition(mapData->getMapSize());
	Point pos = this->calcMapPositionInZoomOut();
	pos.x = pos.x - m_mapSp->getBoundingBox().size.width / 2 + MAP_MARGIN + centerPos.x * m_mapScaleX;
	pos.y = pos.y - m_mapSp->getBoundingBox().size.height / 2 + MAP_MARGIN + (centerPos.y - mapData->getTileSize().height / 2) * m_mapScaleY;
	return pos;
}

Point Minimap::calcSafeZonePositionInZoomIn() const
{
	DataPlayer* myChar = World::getInstance()->getMyself();
	NS_ASSERT(myChar);
	MapData* mapData = myChar->getMapData();

	Point centerPos = m_safeZoneCenter.computePosition(mapData->getMapSize());
	Point frameZoomedInPosition = this->getFrameZoomedInPosition();
	Point pos;
	pos.x = frameZoomedInPosition.x - m_mapSp->getBoundingBox().size.width / 2 + MAP_MARGIN + centerPos.x * m_mapScaleX;
	pos.y = frameZoomedInPosition.y - m_mapSp->getBoundingBox().size.height / 2 + MAP_MARGIN + (centerPos.y - mapData->getTileSize().height / 2) * m_mapScaleY;
	return pos;
}

bool Minimap::isZoomAnimating() const
{
	return m_frameSp->getActionByTag(ACTION_ZOOMING) != nullptr;
}

void Minimap::stopZoomAnimation()
{
	if (Action* action = m_zoomBtn->getActionByTag(ACTION_ZOOMING))
		m_zoomBtn->stopAction(action);
	if (Action* action = m_frameSp->getActionByTag(ACTION_ZOOMING))
		m_frameSp->stopAction(action);
	if (Action* action = m_mapMaskSp->getActionByTag(ACTION_ZOOMING))
		m_mapMaskSp->stopAction(action);
	if (Action* action = m_mapSp->getActionByTag(ACTION_ZOOMING))
		m_mapSp->stopAction(action);
	if (Action* action = m_myLocatorSp->getActionByTag(ACTION_ZOOMING))
		m_myLocatorSp->stopAction(action);
	if (Action* action = m_safeZoneMaskSp->getActionByTag(ACTION_ZOOMING))
		m_safeZoneMaskSp->stopAction(action);
	if (Action* action = m_safeZoneBorderSp->getActionByTag(ACTION_ZOOMING))
		m_safeZoneBorderSp->stopAction(action);
	Sprite* renderSprite = m_renderTexture->getSprite();
	if (Action* action = renderSprite->getActionByTag(ACTION_ZOOMING))
		renderSprite->stopAction(action);
	if (Action* action = m_playerCountBar->getActionByTag(ACTION_ZOOMING))
		m_playerCountBar->stopAction(action);
}

void Minimap::startZoomInAnimation()
{
	ActionInterval* scale = nullptr;
	ActionInterval* move = nullptr;
	ActionInterval* spawn = nullptr;
	ActionInterval* fade = nullptr;

	if (!this->isZoomAnimating())
	{
		m_frameSp->setSpriteFrame(FRAMENAME_FRAME_ZOOMIN);
		m_frameSp->setScale(FRAME_ZOOMOUT_SCALE);
		m_mapMaskSp->setSpriteFrame(FRAMENAME_MAP_MASK_ZOOMIN);
		m_mapMaskSp->setScale(FRAME_ZOOMOUT_SCALE);
	}

	this->stopZoomAnimation();

	scale = ScaleTo::create(ZOOM_DURATION, 1.f);
	move = MoveTo::create(ZOOM_DURATION, this->getZoomButtonZoomedInPosition());
	spawn = EaseQuadraticActionOut::create(Spawn::create(scale, move, nullptr));
	spawn->setTag(ACTION_ZOOMING);
	m_zoomBtn->runAction(spawn);

	Point frameZoomedInPosition = this->getFrameZoomedInPosition();
	scale = ScaleTo::create(ZOOM_DURATION, 1.f);
	move = MoveTo::create(ZOOM_DURATION, frameZoomedInPosition);
	spawn = EaseQuadraticActionOut::create(Spawn::create(scale, move, nullptr));
	spawn->setTag(ACTION_ZOOMING);
	m_frameSp->runAction(spawn);

	move = EaseQuadraticActionOut::create(MoveTo::create(ZOOM_DURATION, frameZoomedInPosition));
	move->setTag(ACTION_ZOOMING);
	m_mapSp->runAction(move);

	move = EaseQuadraticActionOut::create(MoveTo::create(ZOOM_DURATION, this->calcMyLocatorPositionInZoomIn()));
	move->setTag(ACTION_ZOOMING);
	m_myLocatorSp->runAction(move);

	Point safeZonePos = this->calcSafeZonePositionInZoomIn();
	move = EaseQuadraticActionOut::create(MoveTo::create(ZOOM_DURATION, safeZonePos));
	move->setTag(ACTION_ZOOMING);
	m_safeZoneMaskSp->runAction(move);

	move = EaseQuadraticActionOut::create(MoveTo::create(ZOOM_DURATION, safeZonePos));
	move->setTag(ACTION_ZOOMING);
	m_safeZoneBorderSp->runAction(move);

	Sprite* renderSprite = m_renderTexture->getSprite();
	fade = FadeTo::create(ZOOM_DURATION, RENDER_SPRITE_OPACITY_ZOOMIN);
	fade->setTag(ACTION_ZOOMING);
	renderSprite->runAction(fade);

	scale = ScaleTo::create(ZOOM_DURATION, 1.0f);
	move = MoveTo::create(ZOOM_DURATION, frameZoomedInPosition);
	spawn = EaseQuadraticActionOut::create(Spawn::create(scale, move, nullptr));
	spawn->setTag(ACTION_ZOOMING);
	m_mapMaskSp->runAction(spawn);

	fade = FadeTo::create(PLAYER_COUNTBAR_FADE_DURATION, 0);
	fade->setTag(ACTION_ZOOMING);
	m_playerCountBar->runAction(fade);
}

void Minimap::startZoomOutAnimation()
{
	this->stopZoomAnimation();

	ActionInterval* scale = nullptr;
	ActionInterval* move = nullptr;
	ActionInterval* spawn = nullptr;
	ActionInterval* fade = nullptr;
	ActionInterval* sequence = nullptr;
	CallFuncN* callFunc = nullptr;

	scale = ScaleTo::create(ZOOM_DURATION, ZOOM_BUTTON_ZOOMOUT_SCALE);
	move = MoveTo::create(ZOOM_DURATION, this->getZoomButtonOriginPosition());
	spawn = EaseQuadraticActionOut::create(Spawn::create(scale, move, nullptr));
	spawn->setTag(ACTION_ZOOMING);
	m_zoomBtn->runAction(spawn);

	Point frameOriginPosition = this->getFrameOriginPosition();
	scale = ScaleTo::create(ZOOM_DURATION, FRAME_ZOOMOUT_SCALE);
	move = MoveTo::create(ZOOM_DURATION, frameOriginPosition);
	spawn = EaseQuadraticActionOut::create(Spawn::create(scale, move, nullptr));
	callFunc = CallFuncN::create([](Node* node) {
		Sprite* sprite = static_cast<Sprite*>(node);
		sprite->setSpriteFrame(FRAMENAME_FRAME_ZOOMOUT);
		sprite->setScale(1.0f);
	});
	sequence = Sequence::create(spawn, callFunc, nullptr);
	sequence->setTag(ACTION_ZOOMING);
	m_frameSp->runAction(sequence);

	scale = ScaleTo::create(ZOOM_DURATION, FRAME_ZOOMOUT_SCALE);
	move = MoveTo::create(ZOOM_DURATION, frameOriginPosition);
	spawn = EaseQuadraticActionOut::create(Spawn::create(scale, move, nullptr));
	callFunc = CallFuncN::create([this](Node* node) {
		Sprite* sprite = static_cast<Sprite*>(node);
		sprite->setSpriteFrame(FRAMENAME_MAP_MASK_ZOOMOUT);
		sprite->setScale(1.0f);
	});
	sequence = Sequence::create(spawn, callFunc, nullptr);
	sequence->setTag(ACTION_ZOOMING);
	m_mapMaskSp->runAction(sequence);

	move = EaseQuadraticActionOut::create(MoveTo::create(ZOOM_DURATION, this->calcMapPositionInZoomOut()));
	move->setTag(ACTION_ZOOMING);
	m_mapSp->runAction(move);

	move = EaseQuadraticActionOut::create(MoveTo::create(ZOOM_DURATION, frameOriginPosition));
	move->setTag(ACTION_ZOOMING);
	m_myLocatorSp->runAction(move);

	Point safeZonePos = this->calcSafeZonePositionInZoomOut();
	move = EaseQuadraticActionOut::create(MoveTo::create(ZOOM_DURATION, safeZonePos));
	move->setTag(ACTION_ZOOMING);
	m_safeZoneMaskSp->runAction(move);

	move = EaseQuadraticActionOut::create(MoveTo::create(ZOOM_DURATION, safeZonePos));
	move->setTag(ACTION_ZOOMING);
	m_safeZoneBorderSp->runAction(move);

	Sprite* renderSprite = m_renderTexture->getSprite();
	fade = FadeTo::create(ZOOM_DURATION, RENDER_SPRITE_OPACITY_ZOOMOUT);
	fade->setTag(ACTION_ZOOMING);
	renderSprite->runAction(fade);

	sequence = Sequence::create(DelayTime::create(ZOOM_DURATION - PLAYER_COUNTBAR_FADE_DURATION), FadeTo::create(PLAYER_COUNTBAR_FADE_DURATION, 255), nullptr);
	sequence->setTag(ACTION_ZOOMING);
	m_playerCountBar->runAction(sequence);
}

void Minimap::setSafeZoneCenter(TileCoord const& center)
{
	if (m_safeZoneCenter != center)
	{
		m_safeZoneCenter = center;
		m_isSafeZoneDirty = true;
	}
}

void Minimap::setSafeZoneRadius(int32 radius)
{
	if (m_safeZoneRadius != radius)
	{
		m_safeZoneRadius = radius;
		m_isSafeZoneDirty = true;
	}
}

void Minimap::updateSafeZone()
{
	if (!m_isSafeZoneDirty)
		return;

	if (m_isZoomedIn)
		m_safeZoneMaskSp->setPosition(this->calcSafeZonePositionInZoomIn());
	else
		m_safeZoneMaskSp->setPosition(this->calcSafeZonePositionInZoomOut());
	m_safeZoneBorderSp->setPosition(m_safeZoneMaskSp->getPosition());

	if (m_safeZoneRadius > 0)
	{
		DataPlayer* myChar = World::getInstance()->getMyself();
		NS_ASSERT(myChar);
		MapData* mapData = myChar->getMapData();
		float a = mapData->getTileSize().width * m_safeZoneRadius * m_mapScaleX;
		float b = mapData->getTileSize().height * m_safeZoneRadius * m_mapScaleY;
		float c = std::sqrt(a * a + b * b);
		m_safeZoneMaskSp->setContentSize(Size(c + SAFEZONE_BORDER_SIZE * 2, c + SAFEZONE_BORDER_SIZE * 2));
		m_safeZoneBorderSp->setContentSize(m_safeZoneMaskSp->getContentSize());
		m_safeZoneMaskSp->setVisible(true);
		m_safeZoneBorderSp->setVisible(true);
	}
	else
	{
		m_safeZoneMaskSp->setContentSize(Size::ZERO);
		m_safeZoneBorderSp->setContentSize(Size::ZERO);
		m_safeZoneMaskSp->setVisible(false);
		m_safeZoneBorderSp->setVisible(false);
	}

	if (m_safeZoneRadius < 0)
		m_dangerZoneClipper->setVisible(false);
	else
		m_dangerZoneClipper->setVisible(true);

	m_isSafeZoneDirty = false;
}

Point Minimap::getFrameOriginPosition() const
{
	Point pos;
	Sprite* renderSprite = m_renderTexture->getSprite();
	pos.x = renderSprite->getContentSize().width - m_zoomedInPosition.x - m_frameZoomedOutSize.width / 2;
	pos.y = renderSprite->getContentSize().height - m_zoomedInPosition.y - (this->getContentSize().height - m_playerCountBar->getBoundingBox().getMinY() + FRAME_MARGIN_TOP) - m_frameZoomedOutSize.height / 2;
	return pos;
}

Point Minimap::getFrameZoomedInPosition() const
{
	Point pos;
	pos.x = m_frameZoomedInSize.width / 2;
	pos.y = m_frameZoomedInSize.height / 2;

	return pos;
}

Point Minimap::getZoomButtonOriginPosition() const
{
	Point pos;
	pos.x = this->getContentSize().width - 8;
	pos.y = m_playerCountBar->getBoundingBox().getMinY() - FRAME_MARGIN_TOP - 8;

	return pos;
}

NS_END
