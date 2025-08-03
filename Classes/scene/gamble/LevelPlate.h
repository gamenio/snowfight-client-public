#ifndef __LEVEL_PLATE_H__
#define __LEVEL_PLATE_H__

#include "common/Common.h"
#include "scene/gui/silhouette/SILSprite.h"
#include "scene/gui/silhouette/SILLabel.h"

USING_NS_CC;

NS_BEGIN

class LevelPlate : public Node
{
public:
	static LevelPlate* create();
	bool init() override;

	LevelPlate();
	~LevelPlate();

	void setLevel(uint8 level);

	void setGlobalZOrder(float globalZOrder) override;

private:
	SILSprite* m_bgSp;
	SILLabel* m_levelLabel;

};

NS_END


#endif // __LEVEL_PLATE_H__
