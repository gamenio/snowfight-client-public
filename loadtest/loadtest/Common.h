#ifndef __TEST_COMMON_H__
#define __TEST_COMMON_H__

#define WIN_SIZE						Size(800, 320)

static const int MAX_LOG_LENGTH = 16 * 1024;

void testlog(const char* format, ...);

// 输出日志
#define TESTLOG(format, ...)    testlog(format, ##__VA_ARGS__)

// 断言
#if TEST_DEBUG > 0
#define TEST_ASSERT(cond)		assert(cond)
#define TEST_ASSERT_LOG(cond, msg) do {									\
			if (!(cond)) {												\
				if (strlen(msg))										\
					testlog("Assert failed: %s", msg);             \
				TEST_ASSERT(cond);                                      \
			}															\
		} while (0)
#else
#define TEST_ASSERT(cond)
#define TEST_ASSERT_LOG(cond, msg)
#endif

#endif // __TEST_COMMON_H__