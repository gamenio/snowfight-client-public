#ifndef __MINIMAP_H__
#define __MINIMAP_H__

#include "ui/UIButton.h"
#include "ui/UIScale9Sprite.h"

#include "common/Common.h"
#include "game/tiles/TileCoord.h"
#include "dangerzone/DangerZone.h"
#include "PlayerCountBar.h"

USING_NS_CC;

NS_BEGIN

const float MINIMAP_ZOOM_BUTTON_SAFE_RADIUS = 14.f;

class Minimap : public Node, public SafeZoneListener
{
public:
	static Minimap* create();

	Minimap();
	~Minimap();

	bool init() override;

	void visit(Renderer* renderer, Mat4 const& parentTransform, uint32_t parentFlags) override;
	void draw(Renderer* renderer, Mat4 const& transform, uint32_t flags) override;
	void update(float delta) override;

	bool isZoomedIn() const { return m_isZoomedIn; }
	void zoomIn();
	void zoomOut();

	void setZoomedInPosition(cocos2d::Point const& position);
	cocos2d::Point const& getZoomedInPosition() const { return m_zoomedInPosition; }

	cocos2d::Point getZoomButtonZoomedInPosition() const;
	PlayerCountBar* getPlayerCountBar() const { return m_playerCountBar; }

	// SafeZoneListener
	void onSafeZoneStarted(TileCoord const& center, int32 initialRadius) override;
	void onSafeZoneUpdated(int32 currRadius) override;

private:
	void buttonZoomCallback(Ref* sender);

	void layoutElements();

	cocos2d::Point calcMapPositionInZoomOut() const;
	cocos2d::Point calcMyLocatorPositionInZoomIn() const;
	cocos2d::Point calcSafeZonePositionInZoomOut() const;
	cocos2d::Point calcSafeZonePositionInZoomIn() const;

	bool isZoomAnimating() const;
	void stopZoomAnimation();
	void startZoomInAnimation();
	void startZoomOutAnimation();

	void setSafeZoneCenter(TileCoord const& center);
	void setSafeZoneRadius(int32 radius);
	void updateSafeZone();

	cocos2d::Point getFrameOriginPosition() const;
	cocos2d::Point getFrameZoomedInPosition() const;
	cocos2d::Point getZoomButtonOriginPosition() const;

	bool m_isZoomedIn;
	bool m_isLayoutDirty;
	cocos2d::Size m_frameZoomedInSize;
	cocos2d::Size m_frameZoomedOutSize;
	float m_mapScaleX;
	float m_mapScaleY;
	TileCoord m_safeZoneCenter;
	int32 m_safeZoneRadius;
	bool m_isSafeZoneDirty;
	cocos2d::Point m_zoomedInPosition;

	PlayerCountBar* m_playerCountBar;
	RenderTexture* m_renderTexture;
	ClippingNode* m_frameClipper;
	Sprite* m_frameSp;
	ui::Button* m_zoomBtn;
	Sprite* m_mapSp;
	Sprite* m_mapMaskSp;
	ClippingNode* m_mapClipper;
	Sprite* m_myLocatorSp;
	ui::Scale9Sprite* m_dangerZoneSp;
	ui::Scale9Sprite* m_safeZoneBorderSp;
	ClippingNode* m_dangerZoneClipper;
	ClippingNode* m_safeZoneClipper;
	ui::Scale9Sprite* m_safeZoneMaskSp;
};


NS_END

#endif // __MINIMAP_H__
