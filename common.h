/*
	This is part of pyahocorasick Python module.
	
	common definitions and includes

	Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
	License   : public domain
*/

#ifndef ahocorasick_common_h_included__
#define ahocorasick_common_h_included__

#include <Python.h>
#include <structmember.h>	// PyMemberDef

#include <iso646.h>

#define DEBUG

#if defined(_MSC_VER)       // Visual Studio compiler
#   include "windows.h"
#else
#	include "posix.h"
#endif

#if PY_MAJOR_VERSION >= 3
    #define PY3K
#else
    #ifdef AHOCORASICK_UNICODE
        #warning "No support for unicode in version for Python2"
    #endif
    #undef AHOCORASICK_UNICODE
#endif

// setup supported character set
#ifdef AHOCORASICK_UNICODE
#	ifdef Py_UNICODE_WIDE
		// Python use UCS-4
#		define TRIE_LETTER_TYPE	uint32_t
#		define TRIE_LETTER_SIZE 4
#	else
		// Python use UCS-2
#		define TRIE_LETTER_TYPE	uint16_t
#		define TRIE_LETTER_SIZE 2
#       define VARIABLE_LEN_CHARCODES 1
#	endif
#else
	// only bytes are supported
#	define TRIE_LETTER_TYPE	uint16_t
#	define TRIE_LETTER_SIZE 2
#endif


#define memalloc	PyMem_Malloc
#define memfree		PyMem_Free
#define memrealloc	PyMem_Realloc

#ifdef __GNUC__
#	define	LIKELY(x)	__builtin_expect(x, 1)
#	define	UNLIKELY(x)	__builtin_expect(x, 0)
#	define	ALWAYS_INLINE	__attribute__((always_inline))
#	define	PURE			__attribute__((pure))
#else
#	define	LIKELY(x)	x
#	define	UNLIKELY(x)	x
#	define	ALWAYS_INLINE
#	define	PURE
#endif

#ifdef DEBUG
#	include <assert.h>
#	define	ASSERT(expr)	do {if (!(expr)) {fprintf(stderr, "%s:%s:%d - %s failed!\n", __FILE__, __FUNCTION__, __LINE__, #expr); fflush(stderr); exit(1);} }while(0)
#else
#	define	ASSERT(expr)
#endif

typedef char	bool;
#define true 1
#define false 0

#endif
