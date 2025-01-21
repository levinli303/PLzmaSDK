//
// By using this Software, you are accepting original [LZMA SDK] and MIT license below:
//
// The MIT License (MIT)
//
// Copyright (c) 2015 - 2025 Oleh Kulykov <olehkulykov@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//


#ifndef __PLZMA_PRIVATE_H__
#define __PLZMA_PRIVATE_H__ 1

#include "../libplzma.h"

#include <string.h>
#include <assert.h>

// Mark of the libplzma build.
// This definition uses by the patched, original [LZMA SDK] via 'StdAfx.h' files in Win env.
#if !defined(LIBPLZMA)
#  define LIBPLZMA 1
#endif

#if !defined(DEBUG) && (defined(DEBUG) || defined(_DEBUG))
#  define DEBUG 1
#endif

// plzma_c_bindings_private.hpp
#if !defined(LIBPLZMA_HAVE_STD) && (defined(SWIFT_PACKAGE) || defined(COCOAPODS))
#  define LIBPLZMA_HAVE_STD 1
#endif

#if __has_include(<sys/sys_types.h>) || defined(HAVE_SYS_SYS_TYPES_H)
#  include <sys/sys_types.h>
#endif

#if __has_include(<sys/sysmacros.h>) || defined(HAVE_SYS_SYSMACROS_H)
#  include <sys/sysmacros.h>
#endif

#if __has_include(<sys/utime.h>) || defined(HAVE_SYS_UTIME_H)
#  include <sys/utime.h>
#endif

#if __has_include(<utime.h>) || defined(HAVE_UTIME_H)
#  include <utime.h>
#endif

#if __has_include(<semaphore.h>) || defined(HAVE_SEMAPHORE_H)
#  include <semaphore.h>
#endif

#if __has_include(<unistd.h>) || defined(HAVE_UNISTD_H) || defined(_UNISTD_H_)
#  include <unistd.h>
#  define LIBPLZMA_INCLUDED_UNISTD_H 1
#endif

#if __has_include(<pthread.h>) || defined(HAVE_PTHREAD_H) || (defined(_POSIX_THREADS) && (_POSIX_THREADS > 0))
#  include <pthread.h>
#  define LIBPLZMA_INCLUDED_PTHREAD_H 1
#endif

#if __has_include(<android/api-level.h>) || defined(HAVE_ANDROID_API_LEVEL_H)
#  include <android/api-level.h>
#endif

#if defined(__APPLE__) || defined(__unix__) || defined(__unix) || defined(__linux__) || defined(__linux) || defined(__gnu_linux__)
#  if !defined(LIBPLZMA_INCLUDED_UNISTD_H)
#    include <unistd.h>
#    define LIBPLZMA_INCLUDED_UNISTD_H 1
#  endif
#  if !defined(LIBPLZMA_INCLUDED_PTHREAD_H)
#    include <pthread.h>
#    define LIBPLZMA_INCLUDED_PTHREAD_H 1
#  endif
#endif

#if defined(__ANDROID__) || defined(__ANDROID_API__)
#  if !defined(LIBPLZMA_INCLUDED_UNISTD_H)
#    include <unistd.h>
#    define LIBPLZMA_INCLUDED_UNISTD_H 1
#  endif
#  if !defined(LIBPLZMA_INCLUDED_PTHREAD_H)
#    include <pthread.h>
#    define LIBPLZMA_INCLUDED_PTHREAD_H 1
#  endif
#endif

#if !defined(LIBPLZMA_POSIX) && (defined(_POSIX_VERSION) && (_POSIX_VERSION > 0))
#  define LIBPLZMA_POSIX 1
#endif

#if !defined(LIBPLZMA_POSIX) && (defined(_POSIX2_VERSION) && (_POSIX2_VERSION > 0))
#  define LIBPLZMA_POSIX 1
#endif

#if !defined(LIBPLZMA_POSIX) && (defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE > 0))
// Define this macro to a positive integer to control which POSIX functionality is made available.
// The greater the value of this macro, the more functionality is made available.
#  define LIBPLZMA_POSIX 1
#endif

#if !defined(LIBPLZMA_POSIX) && defined(_GNU_SOURCE)
// If you define this macro, everything is included: ISO C89, ISO C99, POSIX.1, POSIX.2, BSD, SVID, X/Open, LFS, and GNU extensions.
// In the cases where POSIX.1 conflicts with BSD, the POSIX definitions take precedence.
#  define LIBPLZMA_POSIX 1
#endif

#if !defined(LIBPLZMA_POSIX) && defined(_XOPEN_SOURCE)
// If you define this macro, everything is included: ISO C89, ISO C99, POSIX.1, POSIX.2, BSD, SVID, X/Open, LFS, and GNU extensions.
// In the cases where POSIX.1 conflicts with BSD, the POSIX definitions take precedence.
#  define LIBPLZMA_POSIX 1
#endif

#if !defined(LIBPLZMA_POSIX) && defined(_DARWIN_C_SOURCE)
#  define LIBPLZMA_POSIX 1
#endif

#if !defined(LIBPLZMA_POSIX) && defined(LIBPLZMA_INCLUDED_PTHREAD_H)
#  define LIBPLZMA_POSIX 1
#endif

#if !defined(LIBPLZMA_MINGW)
#  if (defined(__MINGW32__) && (__MINGW32__ > 0)) || (defined(__MINGW64__) && (__MINGW64__ > 0))
#    define LIBPLZMA_MINGW 1
#  endif
#endif // !LIBPLZMA_MINGW

#if !defined(LIBPLZMA_POSIX) && defined(LIBPLZMA_MINGW)
#  error "Configuration error: MinGW without posix."
#endif

#if defined(LIBPLZMA_OS_WINDOWS)

#  if !defined(LIBPLZMA_MSC) && (defined(_MSC_VER) || defined(_MSC_FULL_VER))
#    define LIBPLZMA_MSC 1
#  endif

#else // !LIBPLZMA_OS_WINDOWS

// Types
typedef void* PVOID;
typedef void* LPVOID;
typedef void* HANDLE;

#  if !defined(DWORD_SIZE)
#    define DWORD_SIZE 4
typedef uint32_t DWORD;
#  endif

#  if !defined(TRUE)
#    define TRUE 1
#  endif

#  if !defined(FALSE)
#    define FALSE 0
#  endif

#endif // LIBPLZMA_OS_WINDOWS


#ifndef USE_MIXER_ST
#  define USE_MIXER_ST
#endif

#ifndef _7ZIP_ST
#  define _7ZIP_ST 1
#endif

#ifndef Z7_ST
#  define Z7_ST 1
#endif

#if defined(LIBPLZMA_NO_TAR)
#  define LIBPLZMA_NO_TAR_EXCEPTION_WHAT "The tar(tarball) support was explicitly disabled. Use cmake option 'LIBPLZMA_OPT_NO_TAR:BOOL=OFF' or undefine 'LIBPLZMA_NO_TAR' preprocessor definition globally to enable tar(tarball) support."
#endif

#if defined(LIBPLZMA_NO_CRYPTO)
#  define _NO_CRYPTO 1
#  define Z7_NO_CRYPTO 1
#  define LIBPLZMA_NO_CRYPTO_EXCEPTION_WHAT "The crypto functionality was explicitly disabled. Use cmake option 'LIBPLZMA_OPT_NO_CRYPTO:BOOL=OFF' or undefine 'LIBPLZMA_NO_CRYPTO' preprocessor definition globally to enable crypto functionality."
#endif // LIBPLZMA_NO_CRYPTO

#if defined(DEBUG)
#  define LIBPLZMA_DEBUG_ASSERT(ASSERT_CONDITION) assert(ASSERT_CONDITION);
#else // !DEBUG
#  define LIBPLZMA_DEBUG_ASSERT(ASSERT_CONDITION)
#endif // DEBUG


#if !defined(LIBPLZMA_USING_REGISTRATORS)
#  define LIBPLZMA_USING_REGISTRATORS 1
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_1(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_2(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_3(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_4(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_5(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_6(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_7(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_8(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_9(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_10(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_11(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_12(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_13(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_14(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_15(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_16(void);
#  if !defined(LIBPLZMA_NO_TAR)
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_17(void);
#  endif
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_18(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_19(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_20(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_21(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_22(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_23(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_24(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_25(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_26(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_27(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_28(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_29(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_30(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_31(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_32(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_33(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_34(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_35(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_36(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_37(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_38(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_39(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_40(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_41(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_42(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_43(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_44(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_45(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_46(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_47(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_48(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_49(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_50(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_51(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_52(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_53(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_54(void);
// LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_55(void);
// LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_56(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_57(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_58(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_59(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_60(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_61(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_62(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_63(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_64(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_65(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_66(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_67(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_68(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_69(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_70(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_71(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_72(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_73(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_74(void);
LIBPLZMA_C_API_PRIVATE(uint64_t) plzma_registrator_75(void);
#endif // !LIBPLZMA_USING_REGISTRATORS

#if 0
LIBPLZMA_C_API_PRIVATE(void) plzma_print_memory(int line, const void * LIBPLZMA_NULLABLE mem, const size_t len);
#endif // #if 0

#endif // !__PLZMA_PRIVATE_H__
