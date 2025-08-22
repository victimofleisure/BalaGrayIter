// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <stdio.h>

// TODO: reference additional headers your program requires here

#if defined(_MSC_VER)
#define FORCE_INLINE __forceinline
#elif defined(__clang__) || defined(__GNUC__)
// always_inline is only honored with -O*; fall back to plain inline at -O0
#if defined(__OPTIMIZE__)
#define FORCE_INLINE inline __attribute__((always_inline))
#else
#define FORCE_INLINE inline
#endif
#else
#define FORCE_INLINE inline
#endif

#if !defined(_MSC_VER)
#define _countof std::size
#endif
