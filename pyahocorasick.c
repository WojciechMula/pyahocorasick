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


#include "slist.c"

/* Types */
typedef struct TrieNode {
	union {
		PyObject*	object;		///< valid when kind = STORE_ANY
		int			integer;	///< valid when kind in [STORE_LENGTH, STORE_INTS]
	} output; ///< output function, valid when eow is true

	struct TrieNode*	fail;	///< fail node

	uint16_t			n;		///< length of next
	uint8_t				byte;	///< incoming edge label
	uint8_t				eow;	///< end of word marker
	struct TrieNode**	next;	///< table of pointers
} TrieNode;


typedef enum {
	EMPTY,
	TRIE,
	AHOCORASICK
} AutomatonKind;


typedef enum {
	STORE_INTS,
	STORE_LENGTH,
	STORE_ANY
} KeysStore;


typedef struct AutomatonStatistics {
	int		version;

	int		nodes_count;		///< total number of nodes
	int		words_count;		///< len(automaton)
	int		longest_word;		///< longest word
	int		links_count;		///< links count
	int		sizeof_node;		///< size of single node (a C structure)
	int		total_size;			///< total size in bytes
} AutomatonStatistics;


typedef struct Automaton {
	PyObject_HEAD

	AutomatonKind	kind;	///< current kind of automaton
	KeysStore		store;	///< type of values: copy of string, bare integer, python  object
	int				count;	///< number of distinct words
	TrieNode*		root;	///< root of a trie

	int				version;	///< current version of automaton, incremented by add_word, clean and make_automaton; used to lazy invalidate iterators

	AutomatonStatistics	stats;	///< statistics
} Automaton;


static PyTypeObject automaton_type;


typedef struct AutomatonSearchIter {
	PyObject_HEAD

	Automaton*	automaton;
	int			version;	///< automaton version
	PyObject*	object;		///< unicode or buffer
	void*		data;		///< Py_UNICODE or char*
	TrieNode*	state;		///< current state of automaton
	TrieNode*	output;		///< current node, i.e. yielded value
	
	int			index;		///< current index
	int			end;		///< end index
	bool		is_unicode;	///< is data unicode or bytes
} AutomatonSearchIter;


static PyTypeObject automaton_search_iter_type;


typedef enum {
	ITER_KEYS,
	ITER_VALUES,
	ITER_ITEMS
} ItemsType;


typedef struct AutomatonItemsIter {
	PyObject_HEAD

	Automaton*	automaton;
	int			version;		///< automaton version
	TrieNode* 	state;			///< current automaton node
	List		stack;			///< stack
	ItemsType	type;			///< type of iterator (KEYS/VALUES/ITEMS)
	
	size_t		n;				///< length of buffer
	char*		buffer;			///< buffer to construct key representation
} AutomatonItemsIter;


static PyTypeObject automaton_items_iter_type;


/* inline code! */
#include "utils.c"
#include "trie.c"
#include "AutomatonItemsIter.c"
#include "AutomatonSearchIter.c"
#include "Automaton.c"


static
PyMethodDef
ahocorasick_module_methods[] = {
	{NULL, NULL, 0, NULL}
};


static
PyModuleDef ahocorasick_module = {
	PyModuleDef_HEAD_INIT,
	"ahocorasick",
	"ahocorasick module",
	-1,
	ahocorasick_module_methods
};

PyMODINIT_FUNC
PyInit_ahocorasick(void) {
	PyObject* module;

	automaton_as_sequence.sq_length   = automaton_len;
	automaton_as_sequence.sq_contains = automaton_contains;

	automaton_type.tp_as_sequence = &automaton_as_sequence;
	
	module = PyModule_Create(&ahocorasick_module);
	if (module == NULL)
		return NULL;

	if (PyType_Ready(&automaton_type) < 0) {
		Py_DECREF(module);
		return NULL;
	}
	else
		PyModule_AddObject(module, "Automaton", (PyObject*)&automaton_type);

#define add_enum_const(name) PyModule_AddIntConstant(module, #name, name)
	add_enum_const(TRIE);
	add_enum_const(AHOCORASICK);
	add_enum_const(EMPTY);

	add_enum_const(STORE_LENGTH);
	add_enum_const(STORE_INTS);
	add_enum_const(STORE_ANY);
#undef add_enum_const

	return module;
}
