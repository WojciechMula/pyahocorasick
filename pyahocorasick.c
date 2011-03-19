#include <Python.h>
#include <structmember.h>	// PyMemberDef
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


static PyObject*
automaton_new(PyObject* self, PyObject* args) {
	Automaton* automaton;
	int store;

	if (PyArg_ParseTuple(args, "i", &store)) {
		switch (store) {
			case STORE_STRINGS:
			case STORE_INTS:
			case STORE_ANY:
				// ok
				break;

			default:
				PyErr_SetString(
					PyExc_ValueError,
					"store must have value STORE_STRINGS, STORE_INTS or STORE_ANY"
				);
				return NULL;
		} // switch
	}
	else {
		PyErr_Clear();
		store = STORE_ANY;
	}
	
	automaton = (Automaton*)PyObject_New(Automaton, &automaton_type);
	if (automaton == NULL)
		return NULL;

	automaton->count = 0;
	automaton->kind  = EMPTY;
	automaton->store = store;
	automaton->root  = NULL;

	return (PyObject*)automaton;
}

/* inline code! */
#include "utils.c"
#include "trie.c"
#include "slist.c"


static PyObject*
automaton_clear(PyObject* self, PyObject* args);

static void
automaton_del(PyObject* self) {
#define automaton ((Automaton*)self)
	automaton_clear(self, NULL);
	PyObject_Del(self);
#undef automaton
}


#define automaton_len_doc \
	"returns count of words"

ssize_t
automaton_len(PyObject* self) {
#define automaton ((Automaton*)self)
	return automaton->count;
#undef automaton
}


#define automaton_add_word_doc \
	"adds news word to dictionary"

static PyObject*
automaton_add_word(PyObject* self, PyObject* args) {
#define automaton ((Automaton*)self)
	// argument
	PyObject* py_word;
	PyObject* py_value;

	ssize_t wordlen;
	char* word;

	py_word = pymod_get_string(args, 0, &word, &wordlen);
	if (not py_word)
		return NULL;

	py_value = PyTuple_GetItem(args, 1);
	if (not py_value)
		return NULL;

	if (wordlen > 0) {
		bool new_word;
		TrieNode* node = trie_add_word(automaton, word, wordlen, &new_word);
		Py_DECREF(py_word);
		if (node) {
			if (new_word) {
				if (node->output)
					Py_DECREF(node->output);

				Py_INCREF(py_value);
				node->output = py_value;
				Py_RETURN_TRUE;
			}
			else {
				Py_INCREF(py_value);
				node->output = py_value;
				Py_RETURN_FALSE;
			}
		}
		else
			return NULL;
	}

	Py_DECREF(py_word);
	Py_RETURN_FALSE;
}


static
void clear_aux(TrieNode* node, KeysStore store) {
	if (node) {
		switch (store) {
			case STORE_INTS:
				// nop
				break;

			case STORE_STRINGS:
				memfree(node->output);
				break;

			case STORE_ANY:
				if (node->output)
					Py_DECREF(node->output);
				break;
		}

		const int n = node->n;
		int i;
		for (i=0; i < n; i++) {
			TrieNode* child = node->next[i];
			if (child != node) // avoid loops!
				clear_aux(child, store);
		}

		memfree(node);
	}
#undef automaton
}


#define automaton_clear_doc\
	"Removes all objects from dictionary"


static PyObject*
automaton_clear(PyObject* self, PyObject* args) {
#define automaton ((Automaton*)self)
	clear_aux(automaton->root, automaton->store);
	automaton->count = 0;
	automaton->kind = EMPTY;
	automaton->root = NULL;

	Py_RETURN_NONE;
#undef automaton
}


static int
automaton_contains(PyObject* self, PyObject* args) {
#define automaton ((Automaton*)self)
	ssize_t wordlen;
	char* word;
	PyObject* py_word;

	py_word = pymod_get_string(args, 0, &word, &wordlen);
	if (py_word == NULL)
		return -1;

	TrieNode* node = trie_find(automaton->root, word, wordlen);
	Py_DECREF(py_word);
	return (node and node->eow);
#undef automaton
}


#define automaton_match_doc \
	""

static PyObject*
automaton_match(PyObject* self, PyObject* args) {
	if (automaton_contains(self, args))
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;
}


#define automaton_match_prefix_doc \
	""

static PyObject*
automaton_match_prefix(PyObject* self, PyObject* args) {
#define automaton ((Automaton*)self)
	ssize_t wordlen;
	char* word;
	PyObject* py_word;

	py_word = pymod_get_string(args, 0, &word, &wordlen);
	if (py_word == NULL)
		return NULL;

	TrieNode* node = trie_find(automaton->root, word, wordlen);
	if (node)
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;
#undef automaton
}


#define automaton_get_doc \
	""

static PyObject*
automaton_get(PyObject* self, PyObject* args) {
#define automaton ((Automaton*)self)
	ssize_t wordlen;
	char* word;
	PyObject* py_word;

	py_word = pymod_get_string(args, 0, &word, &wordlen);
	if (py_word == NULL)
		return NULL;

	TrieNode* node = trie_find(automaton->root, word, wordlen);
	if (node and node->eow) {
		switch (automaton->store) {
			case STORE_STRINGS:
				return Py_BuildValue("s", node->output);

			case STORE_INTS:
				return Py_BuildValue("i", (int)(node->output));

			case STORE_ANY:
				Py_INCREF(node->output);
				return node->output;

			default:
				PyErr_SetNone(PyExc_ValueError);
				return NULL;
		}
	}
	else {
		PyErr_SetNone(PyExc_KeyError);
		return NULL;
	}
#undef automaton
}

#define automaton_make_automaton_doc \
	""

static PyObject*
automaton_make_automaton(PyObject* self, PyObject* args) {
#define automaton ((Automaton*)self)
	if (automaton->kind != TRIE)
		Py_RETURN_FALSE;
		
	List queue;
	int i;

	list_init(&queue);

	// 1. setup root
	ASSERT(automaton->root);
	for (i=0; i < 256; i++) {
		TrieNode* child = trienode_get_next(automaton->root, i);
		if (child) {
			// fail edges go to root
			child->fail = automaton->root;
			if (not list_append(&queue, child))
				goto no_mem;
		}
		else
			// loop on root
			if (not trienode_set_next(automaton->root, i, automaton->root))
				goto no_mem;
	}

	// 2. make links
	TrieNode* node;
	TrieNode* child;
	TrieNode* state;
	while (true) {
		node = list_pop_first(&queue);
		if (node == NULL)
			break;

		const size_t n = node->n;
		for (i=0; i < n; i++) {
			child = node->next[i];
			ASSERT(child);
			if (not list_append(&queue, child))
				goto no_mem;

			state = node->fail;
			ASSERT(state);
			ASSERT(child);
			while (state != automaton->root and\
				   not trienode_get_next(state, child->byte)) {

				state = state->fail;
				ASSERT(state);
			}

			child->fail = trienode_get_next(state, child->byte);
			if (child->fail == NULL)
				child->fail = automaton->root;
			
			ASSERT(child->fail);
		}
	}

	automaton->kind = AHOCORASICK;
	list_delete(&queue);
	Py_RETURN_NONE;
#undef automaton

no_mem:
	list_delete(&queue);
	PyErr_NoMemory();
	return NULL;
}


#define automaton_search_all_doc \
	""

static PyObject*
automaton_search_all(PyObject* self, PyObject* args) {
#define automaton ((Automaton*)self)
	if (automaton->kind != AHOCORASICK)
		Py_RETURN_NONE;

	ssize_t wordlen;
	char* word;
	PyObject* py_word;
	PyObject* callback;
	PyObject* callback_ret;

	py_word = pymod_get_string(args, 0, &word, &wordlen);
	if (py_word == NULL)
		return NULL;

	callback = PyTuple_GetItem(args, 1);
	if (callback == NULL)
		return callback;

	ssize_t i;
	TrieNode* state;
	TrieNode* tmp;

	state = automaton->root;
	for (i=0; i < wordlen; i++) {
		while (state != automaton->root and\
			   not trienode_get_next(state, word[i])) {

			state = state->fail;
			ASSERT(state);
		}

		state = trienode_get_next(state, word[i]);
		if (state == NULL)
			state = automaton->root;

		tmp = state;
		while (tmp and tmp->output) {
			callback_ret = PyObject_CallFunction(callback, "iO", i, tmp->output);
			if (callback_ret == NULL)
				return NULL;
			else
				Py_DECREF(callback_ret);

			tmp = tmp->fail;
		}
	}
#undef automaton

	Py_RETURN_NONE;
}


#define method(name, kind) {#name, automaton_##name, kind, automaton_##name##_doc}
static
PyMethodDef automaton_methods[] = {
	method(add_word,		METH_VARARGS),
	method(clear,			METH_NOARGS),
	method(match,			METH_VARARGS),
	method(match_prefix,	METH_VARARGS),
	method(get,				METH_VARARGS),
	method(make_automaton,	METH_NOARGS),
	method(search_all,		METH_VARARGS),

	{NULL, NULL, 0, NULL}
};
#undef method


static
PySequenceMethods automaton_as_sequence;


static
PyMemberDef automaton_members[] = {
	{
		"kind",
		T_INT,
		offsetof(Automaton, kind),
		READONLY,
		"current kind of automaton"
	},

	{NULL}
};

static PyTypeObject automaton_type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	"Automaton",								/* tp_name */
	sizeof(Automaton),							/* tp_size */
	0,																			/* tp_itemsize? */
	(destructor)automaton_del,          	  	/* tp_dealloc */
	0,                                      	/* tp_print */
	0,                                         	/* tp_getattr */
	0,                                          /* tp_setattr */
	0,                                          /* tp_reserved */
	0,											/* tp_repr */
	0,                                          /* tp_as_number */
	0,                                          /* tp_as_sequence */
	0,                                          /* tp_as_mapping */
	0,                                          /* tp_hash */
	0,                                          /* tp_call */
	0,                                          /* tp_str */
	PyObject_GenericGetAttr,                    /* tp_getattro */
	0,                                          /* tp_setattro */
	0,                                          /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,                         /* tp_flags */
	0,                                          /* tp_doc */
	0,                                          /* tp_traverse */
	0,                                          /* tp_clear */
	0,                                          /* tp_richcompare */
	0,                                          /* tp_weaklistoffset */
	0,                                          /* tp_iter */
	0,                                          /* tp_iternext */
	automaton_methods,							/* tp_methods */
	automaton_members,			                /* tp_members */
	0,                                          /* tp_getset */
	0,                                          /* tp_base */
	0,                                          /* tp_dict */
	0,                                          /* tp_descr_get */
	0,                                          /* tp_descr_set */
	0,                                          /* tp_dictoffset */
	0,                                          /* tp_init */
	0,                                          /* tp_alloc */
	0,                                          /* tp_new */
};


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
