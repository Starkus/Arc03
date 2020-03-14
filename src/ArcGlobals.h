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
