/*
    This is part of pyahocorasick Python module.

    common definitions and includes

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
    License   : public domain
*/

#ifndef ahocorasick_common_h_included__
#define ahocorasick_common_h_included__

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>   // PyMemberDef

#include <iso646.h>

#define DEBUG

#if defined(_MSC_VER)       // Visual Studio compiler
#   include "windows.h"
#else
#   if defined(__CYGWIN__)
#        include "cygwin.h"
#   elif defined(__MINGW32__)
#        include "mingw.h"
#   else
#        include "posix.h"
#   endif
#endif

// setup supported character set
#ifdef AHOCORASICK_UNICODE
    // Either Python uses UCS-4 or we don't know what Python uses,
    // but we use UCS-4
#   define TRIE_LETTER_TYPE uint32_t
#   define TRIE_LETTER_SIZE 4
#else
    // only bytes are supported
#   define TRIE_LETTER_TYPE uint16_t
#   define TRIE_LETTER_SIZE 2
#endif

#ifdef __GNUC__
#   define  LIKELY(x)   __builtin_expect(x, 1)
#   define  UNLIKELY(x) __builtin_expect(x, 0)
#   define  ALWAYS_INLINE   __attribute__((always_inline))
#   define  PURE            __attribute__((pure))
#   define  UNUSED          __attribute__((unused))
#else
#   define  LIKELY(x)   x
#   define  UNLIKELY(x) x
#   define  ALWAYS_INLINE
#   define  PURE
#   define  UNUSED
#endif

#ifdef DEBUG
#   include <assert.h>
#   define  ASSERT(expr)    do {if (!(expr)) {fprintf(stderr, "%s:%s:%d - %s failed!\n", __FILE__, __FUNCTION__, __LINE__, #expr); fflush(stderr); exit(1);} }while(0)
#else
#   define  ASSERT(expr)
#endif

#if defined(PYCALLS_INJECT_FAULTS)
#   include "pycallfault/pycallfault.h"
#else
#   define F(name) name
#endif

typedef char    bool;
#define true 1
#define false 0

#endif
