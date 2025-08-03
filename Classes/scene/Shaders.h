#ifndef __SHADER_H__
#define __SHADER_H__

#include "common/Common.h"

NS_BEGIN

extern const GLchar* Lightness_vert;
extern const GLchar* Lightness_frag;

extern const GLchar* SilhouettableNodeEffect_vert;
extern const GLchar* SilhouettableNodeEffect_noMVP_vert;
extern const GLchar* SilhouettableNodeEffect_frag;

extern const GLchar* SilhouettableNodeMask_vert;
extern const GLchar* SilhouettableNodeMask_noMVP_vert;
extern const GLchar* SilhouettableNodeMask_frag;

extern const GLchar* DiscardAlpha_vert;
extern const GLchar* DiscardAlpha_frag;

NS_END

#endif // __SHADER_H__
