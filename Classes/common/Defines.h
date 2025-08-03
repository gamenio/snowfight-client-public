#ifndef __DEFINES_H__
#define __DEFINES_H__

#include <cinttypes>

#define UPDATE_PRIORITY_DEBUG		-3
#define UPDATE_PRIORITY_NTS			-2
#define UPDATE_PRIORITY_WORLD		-1
#define UPDATE_PRIORITY_SCENE		0


#define UI64FMTD "%" PRIu64
#define UI64LIT(N) UINT64_C(N)

#define SI64FMTD "%" PRId64
#define SI64LIT(N) INT64_C(N)

#define SZFMTD "%" PRIuPTR

// 基本数据类型定义
typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;
typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;


typedef int32 NSTime;



#endif //__DEFINES_H__