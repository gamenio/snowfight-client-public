#ifndef __SIL_PROGRESS_TIMER_H__
#define __SIL_PROGRESS_TIMER_H__

#include "common/Common.h"
#include "SilhouetteLayer.h"

USING_NS_CC;

NS_BEGIN

class SILProgressTimer : public ProgressTimer, public SilhouettableProtocol
{
public:
	static SILProgressTimer* create(Sprite* sp);

	SILProgressTimer();
	~SILProgressTimer();

	bool initWithSprite(Sprite* sp);

	bool isSilhouetted() const override { return m_isSilhouetted; }
	void setSilhouetted(bool silhouetted) override;

	GLubyte getEffectOpacity() const override { return m_effectOpacity; }
	void setEffectOpacity(GLubyte opacity) override;

	void setScreenTexture(Texture2D* texture) override;

	void visit(Renderer* renderer, Mat4 const& parentTransform, uint32_t parentFlags) override;
	void draw(Renderer* renderer, Mat4 const& transform, uint32_t flags) override;

private:
	void onDraw(Mat4 const& transform, uint32_t flags);

	bool m_isSilhouetted;
	GLubyte m_effectOpacity;
	Texture2D* m_screenTexture;

	GLProgramState* m_maskGLProgramState;
	GLProgramState* m_effectGLProgramState;
	CustomCommand m_maskCustomCommand;
	CustomCommand m_effectCustomCommand;
};

NS_END

#endif // __SIL_PROGRESS_TIMER_H__
