#pragma once

#ifdef _WIN32
#define ARC_WIN32
#endif

#ifdef __linux__
#define ARC_LINUX
#include <signal.h>
#endif

#ifdef _DEBUG
#define ARC_DEBUG
#endif

#define ARC_UNUSED(x) (&reinterpret_cast<const int &>(x))

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint32_t bool32;
typedef float f32;
typedef double f64;

#if defined(ARC_WIN32)
#define ARC_BREAK() __debugbreak()
#elif defined(ARC_LINUX)
#define ARC_BREAK() raise(SIGTRAP)
#endif

#ifdef ARC_DEBUG
#define ARC_ASSERT(expr) do { if (!expr) ARC_BREAK(); } while (false)
#else
#define ARC_ASSERT(expr) expr
#endif

#define VK_ASSERT(expr) do { if (expr != VK_SUCCESS) ARC_BREAK(); } while (false)

#define ARC_DISABLE_COPY(CLASSNAME) \
	CLASSNAME(const CLASSNAME &) = delete;\
	CLASSNAME &operator=(const CLASSNAME &) = delete;\
	CLASSNAME(CLASSNAME &&) = delete;\
	CLASSNAME &operator=(CLASSNAME &&) = delete;

#define ARC_DEFINE_SINGLETON(CLASSNAME)\
	ARC_DISABLE_COPY(CLASSNAME)\
	public:\
		static CLASSNAME *Instance() { return sInstance; }\
	private:\
		static CLASSNAME *sInstance;
