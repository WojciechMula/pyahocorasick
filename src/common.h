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
#include <stdbool.h>

/* Critical sections were added in Python 3.13. Older interpreters use the GIL. */
#if PY_VERSION_HEX < 0x030D0000
#   define Py_BEGIN_CRITICAL_SECTION(obj) {
#   define Py_END_CRITICAL_SECTION() }
#   define Py_BEGIN_CRITICAL_SECTION2(a, b) {
#   define Py_END_CRITICAL_SECTION2() }
#endif

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

#if PY_MAJOR_VERSION >= 3
    #define PY3K
    #if PY_MINOR_VERSION >= 3 || PY_MAJOR_VERSION > 3
    #define PEP393
    #ifdef AHOCORASICK_UNICODE
        #define PEP393_UNICODE
    #endif
    #endif
#else
    #ifdef AHOCORASICK_UNICODE
        #warning "No support for unicode in version for Python2"
    #endif
    #undef AHOCORASICK_UNICODE
#endif

// setup supported character set
#ifdef AHOCORASICK_UNICODE
#       if defined PEP393_UNICODE || defined Py_UNICODE_WIDE
        // Either Python uses UCS-4 or we don't know what Python uses,
        // but we use UCS-4
#       define TRIE_LETTER_TYPE uint32_t
#       define TRIE_LETTER_SIZE 4
#   else
        // Python use UCS-2
#       define TRIE_LETTER_TYPE uint16_t
#       define TRIE_LETTER_SIZE 2
#       define VARIABLE_LEN_CHARCODES 1
#   endif
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

#if defined(PYCALLS_INJECT_FAULTS) && defined(PY3K)
#   include "pycallfault/pycallfault.h"
#else
#   define F(name) name
#endif

#endif
