#ifndef __MY_HERO_H__
#define __MY_HERO_H__

#include "common/Common.h"
#include "game/entities/DataPlayer.h"
#include "game/WorldListeners.h"
#include "Hero.h"
#include "AimPointer.h"
#include "EquipmentBar.h"
#include "PropBubble.h"
#include "HidingSpotSearcher.h"

USING_NS_CC;


NS_BEGIN

class MyHero : public Hero, MyCharacterListener,
							MessageListener
{
public:
	MyHero();
	virtual ~MyHero();

	static MyHero* createWithData(DataPlayer* data);
	bool initWithData(DataPlayer* data);

	virtual void update(float delta) override;
	void onEnter() override;

	void onActivated() override;
	void onInactivated() override;

	virtual DataPlayer* getData() const override { return static_cast<DataPlayer*>(m_data); }

	void hurl() override;
	bool moveBy(float direction);
	void moveStop();

	void setAimingDirection(float direction);
	void fire(float direction);
	void fire(cocos2d::Point const& aimPoint);
	void fireStop(bool isFinal);
	void charge();

	void showArrow(float delay = 0.f, float duration = 2.f);
	void toggleEquipmentBar();
private:
	// MyCharacterListener
	void onCharacterInfo(CharacterInfo const& info) override;
	void onRewardMessage(RewardMessage const& message) override;
	void onItemPickupResult(ItemPickupResult const& result) override;

	// MessageListener
	void onItemActionMessage(ItemActionMessage const& message) override;

	void saveLevelAndXPIfUpdated(bool force);

	void updatePositionInTiles(bool force);
	void updateHidingSpots(bool force);
	void updateAlive();
	void updateLevel();
	void updatePropBubbleVisibility(bool force);
	void updateMagicBeanCount();
	void updateReceiveDamage();

	void updateAimPointer(float delta);
	void checkSoundTriggers();

	void updateGlobalZOrderOnAlive() override;

	void onItemApplicationUpdate(bool apply, uint32 itemId, int32 duration, int32 remainingTime) override;
	void applyItem(ItemApplicationTemplate const* appTmpl, int32 duration, int32 remainingTime) override;
	void unapplyItem(ItemApplicationTemplate const* appTmpl) override;

	bool addFootprint(cocos2d::Point const& pos, float orient) override;

	void startPropReceivedAnimation(uint32 itemId);
	void addFloatingLabel(std::string const& text, Color4B const& textColor);
	bool hasPropBubble(ItemTemplate const* itemTemplate) const;

	bool m_fired;
	AimPointer* m_aimPointer;
	EquipmentBar* m_equipmentBar;
	Node* m_floatingLabelContainer;
	PropBubble* m_propBubble;

	TileCoord m_currTileCoord;
	bool m_isPositionInTilesDirty;
	std::unordered_map<std::string, NSTime> m_soundTriggerTimers;
	HidingSpotSearcher* m_hidingSpotSearcher;
};

NS_END

#endif // __MY_HERO_H__

