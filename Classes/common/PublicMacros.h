#ifndef __PUBLIC_MACROS_H__
#define __PUBLIC_MACROS_H__


// 顶级命名空间
#define NS_BEGIN				namespace snowfight {
#define NS_END					}
#define USING_NS				using namespace ::snowfight
#define NS_PREFIX				::snowfight

// 断言
#if NS_DEBUG

#define NS_ASSERT(cond) CC_ASSERT(cond)
#define NS_ASSERT_LOG(cond, msg) CCASSERT(cond, msg)

#else
#define NS_ASSERT(cond)
#define NS_ASSERT_LOG(cond, msg)

#endif // NS_DEBUG

template <typename T> inline T* assertNotNull(T* pointer)
{
	NS_ASSERT(pointer);
	return pointer;
}

// 以下宏用于抑制编译器警告
#define NS_UNUSED_VARIABLE(x) (void)(x)


#endif //__PUBLIC_MACROS_H__
