#ifndef __SNOWBALL_H__
#define __SNOWBALL_H__

#include "cocos2d.h"

#include "common/Common.h"
#include "Bullet.h"
#include "scene/actions/Splatter.h"
#include "scene/gui/silhouette/SILSprite.h"
#include "scene/ObjectPools.h"

USING_NS_CC;

NS_BEGIN

class Snowball : public Bullet
{
public:
	static Snowball* createWithData(DataProjectile* data);

	Snowball();
	virtual ~Snowball();

	bool initWithData(DataProjectile* data) override;

    void update(float delta) override;

protected:
	void reset() override;

private:
	void initRotationAnimation();
	void startRotationAnimation(float speedScale);
	void stopRotationAnimation();

	void didLaunch() override;
	void didShadowRelocation(cocos2d::Point const& position, float vertexZ) override;
	void didHit(ObjectGuid const& target) override;
	void didFallToGround() override;

	void runSplatter(Splatter* splatter, PieceConfig const& config);
	void runSlide(std::function<void()> const& complete);

	SILSprite* m_main;
	SILSprite* m_shadow;
    
    bool m_isSliding;
	Splatter* m_snowSplatter;
	Splatter* m_waterSplatter;
	Animation* m_rotationAnim;
};

NS_END

#endif //__SNOWBALL_H__
