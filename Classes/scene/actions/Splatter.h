#ifndef __SPLATTER_H__
#define __SPLATTER_H__

#include "common/Common.h"
#include "scene/gui/silhouette/SILSprite.h"

USING_NS_CC;

NS_BEGIN

struct PieceConfig
{
	PieceConfig() : 
		maxJumpRange(0),
		minJumpRange(0),
		maxJumpHeight(0),
		minJumpHeight(0),
		minNumber(0),
		maxNumber(0),
		minScale(0),
		maxScale(0),
		fadeoutDuration(0) { }

	int32 maxJumpRange;
	int32 minJumpRange;
	int32 maxJumpHeight;
	int32 minJumpHeight;
	int32 minNumber;
	int32 maxNumber;
	float minScale;
	float maxScale;
	float fadeoutDuration;
	cocos2d::Size contentSize;
};

class Splatter: public Node
{
public:
	Splatter();
	virtual ~Splatter();

	static Splatter* create(std::string frameNameFormat, int32 numberOfFrames = 1, int32 defaultFrameIndex = 0);
	bool init(std::string frameNameFormat, int32 numberOfFrames, int32 defaultFrameIndex);

	void initAnimation();

	void run(PieceConfig const& config, std::function<void()> const& complete);
	bool isRunning() const { return m_isRunning; }
	void stop();

	void update(float delta);

private:
	void initBatchOfBieces();

	bool m_isRunning;
	int32 m_remainingPieceCount;
	std::string m_frameNameFormat;
	int32 m_numberOfFrames;
	int32 m_defaultFrameIndex;
	Animation* m_animation;
	cocos2d::Size m_pieceContentSize;
};

NS_END

#endif // __SPLATTER_H__


