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

#define UNUSED(x) (&reinterpret_cast<const int &>(x))

#include <stdint.h>

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t

#define s8 int8_t
#define s16 int16_t
#define s32 int32_t
#define s64 int64_t

#define bool32 uint32_t
#define f32 float
#define f64 double

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
