#include <Python.h>
#include <structmember.h>	// PyMemberDef
#include <pyerrors.h>	// PyErr_Occured

#include <iso646.h>

#define DEBUG

#define memalloc	PyMem_Malloc
#define memfree		PyMem_Free
#define memrealloc	PyMem_Realloc

#if 1
#	include <assert.h>
#	define	ASSERT(expr)	do {if (!(expr)) printf("%s:%d:%s failed!\n", __FUNCTION__, __LINE__, #expr); }while(0)
#else
#	define	ASSERT(expr)
#endif

typedef char	bool;
#define true 1
#define false 0

/*

TRIE		= 0
AHOCORASICK	= 1

STORE_STRINGS	= 0
STORE_INTS		= 1
STORE_ANY		= 2

class AutomatonException(Exception):
	pass

class Automaton:
	def __init__(self, store)
		"store in STORE_STRINGS, STORE_INTS, STORE_ANY"
		pass

	def add_word(self, word, value)
		"adds new word to dictionary"
		pass

	def clear(self):
		"removes all words"
		pass
	
	def match(self, word):
		"returns true if word is present in a dictionary"
		pass

	def match_prefix(self, word):
		"""
		Returns true if word is present in a dictionary,
		even if word has no associacted value.
		"""
		pass

	def get(self, word, def=None):
		"Returns object associated with word."
		pass

	def make_automaton(self):
		"Constuct Aho-Corsick automaton."
		pass

	def search_all(self, string, callback):
		"""
		Callback is called on every occurence of any word from dictionary
		Callback must accept two arguments: position and a tuple
		"""
		pass

*/

#include "slist.c"

/* Types */
typedef struct TrieNode {
	void*				output;	///< output function
	struct TrieNode*	fail;	///< fail node

	uint16_t			n;		///< length of next
	uint8_t				byte;	///< incoming edge label
	uint8_t				eow;	///< end of word label
	struct TrieNode**	next;	///< table of pointers
} TrieNode;


typedef enum {
	EMPTY,
	TRIE,
	AHOCORASICK
} AutomatonKind;


typedef enum {
	STORE_STRINGS,
	STORE_INTS,
	STORE_ANY
} KeysStore;


typedef struct Automaton {
	PyObject_HEAD

	AutomatonKind	kind;	///< current kind of automaton
	KeysStore		store;
	int				count;
	TrieNode* root;
} Automaton;


static PyTypeObject automaton_type;


typedef struct AutomatonSearchIter {
	PyObject_HEAD

	Automaton*	automaton;
	TrieNode* 	state;
	bool		return_all;
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
	TrieNode* 	state;
	List		stack;
	ItemsType	type;
	
	size_t		n;
	char*		buffer;
} AutomatonItemsIter;


static PyTypeObject automaton_items_iter_type;


/* inline code! */
#include "utils.c"
#include "trie.c"
#include "AutomatonItemsIter.c"
#include "Automaton.c"


static
PyMethodDef
ahocorasick_module_methods[] = {
	{
		"Automaton",
		automaton_new,
		METH_VARARGS,
		"Create new Automaton object"
	},

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
#define add_enum_const(name) PyModule_AddIntConstant(module, #name, name)
	add_enum_const(TRIE);
	add_enum_const(AHOCORASICK);
	add_enum_const(EMPTY);

	add_enum_const(STORE_STRINGS);
	add_enum_const(STORE_INTS);
	add_enum_const(STORE_ANY);
#undef add_enum_const

	return module;
}
