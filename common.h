#ifndef ahocorasick_common_h_included__
#define ahocorasick_common_h_included__

#include <Python.h>
#include <structmember.h>	// PyMemberDef
#include <pyerrors.h>	// PyErr_Occured

#include <iso646.h>

#define DEBUG

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
#	define	ASSERT(expr)	do {if (!(expr)) printf("%s:%d:%s failed!\n", __FUNCTION__, __LINE__, #expr); }while(0)
#else
#	define	ASSERT(expr)
#endif

typedef char	bool;
#define true 1
#define false 0

#endif
