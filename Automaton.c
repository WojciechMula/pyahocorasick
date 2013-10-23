/*
	This is part of pyahocorasick Python module.
	
	Automaton class implementation.
	(this file includes Automaton_pickle.c)

	Author    : Wojciech Mu³a, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl/proj/pyahocorasick/
	License   : 3-clauses BSD (see LICENSE)
	Date      : $Date$

	$Id$
*/

#include "Automaton.h"

static PyTypeObject automaton_type;


static bool ALWAYS_INLINE
check_store(const int store) {
	switch (store) {
		case STORE_LENGTH:
		case STORE_INTS:
		case STORE_ANY:
			return true;

		default:
			PyErr_SetString(
				PyExc_ValueError,
				"store must have value STORE_LENGTH, STORE_INTS or STORE_ANY"
			);
			return false;
	} // switch
}


static bool ALWAYS_INLINE
check_kind(const int kind) {
	switch (kind) {
		case EMPTY:
		case TRIE:
		case AHOCORASICK:
			return true;

		default:
			PyErr_SetString(
				PyExc_ValueError,
				"kind must have value EMPTY, TRIE or AHOCORASICK"
			);
			return false;
	}
}


static PyObject*
automaton_new(PyTypeObject* self, PyObject* args, PyObject* kwargs) {
	Automaton* automaton = NULL;
	int store;

	automaton = (Automaton*)PyObject_New(Automaton, &automaton_type);
	if (UNLIKELY(automaton == NULL))
		return NULL;

	// commons settings
	automaton->version = 0;
	automaton->stats.version = -1;
	automaton->count = 0;
	automaton->longest_word = 0;
	automaton->kind  = EMPTY;
	automaton->root  = NULL;

	if (UNLIKELY(PyTuple_Size(args) == 7)) {
		
		// unpickle: count, data, kind, store, version, values
		size_t			count;
		void*			data;
		size_t			size;
		int				version;
		int				longest_word;
		AutomatonKind	kind;
		KeysStore		store;
		PyObject*		values = NULL;

		if (not PyArg_ParseTuple(args, "iy#iiiiO", &count, &data, &size, &kind, &store, &version, &longest_word, &values)) {
			PyErr_SetString(PyExc_ValueError, "invalid data to restore");
			goto error;
		}

		if (not check_store(store) or not check_kind(kind))
			goto error;

		if (kind != EMPTY) {
			if (automaton_unpickle(automaton, count, data, size, values)) {
				automaton->kind		= kind;
				automaton->store	= store;
				automaton->version	= version;
				automaton->longest_word	= longest_word;
			}
			else
				goto error;
		}

		Py_DECREF(values);
	}
	else {
		// construct new object
		if (PyArg_ParseTuple(args, "i", &store)) {
			if (not check_store(store))
				goto error;
		}
		else {
			PyErr_Clear();
			store = STORE_ANY;
		}

		automaton->store = store;
	}

//ok:
	return (PyObject*)automaton;

error:
	Py_XDECREF(automaton);
	return NULL;
}


static void
automaton_del(PyObject* self) {
#define automaton ((Automaton*)self)
	automaton_clear(self, NULL);
	PyObject_Del(self);
#undef automaton
}


#define automaton_len_doc \
	"returns count of words"

static ssize_t
automaton_len(PyObject* self) {
#define automaton ((Automaton*)self)
	return automaton->count;
#undef automaton
}


#define automaton_add_word_doc \
	"add new word to dictionary"

static PyObject*
automaton_add_word(PyObject* self, PyObject* args) {
#define automaton ((Automaton*)self)
	// argument
	PyObject* py_word = NULL;
	PyObject* py_value = NULL;

	TRIE_LETTER_TYPE* word = NULL;
	ssize_t wordlen = 0;
	int integer = 0;

	py_word = pymod_get_string_from_tuple(args, 0, &word, &wordlen);
	if (not py_word)
		return NULL;

	switch (automaton->store) {
		case STORE_ANY:
			py_value = PyTuple_GetItem(args, 1);
			if (not py_value) {
				PyErr_SetString(PyExc_ValueError, "second argument required");
				return NULL;
			}
			break;

		case STORE_INTS:
			py_value = PyTuple_GetItem(args, 1);
			if (py_value) {
				if (PyNumber_Check(py_value)) {
					integer = (int)PyNumber_AsSsize_t(py_value, PyExc_ValueError);
					if (integer == -1 and PyErr_Occurred())
						return NULL;
				}
				else {
					PyErr_SetString(PyExc_TypeError, "number required");
					return NULL;
				}
			}
			else {
				// default
				PyErr_Clear();
				integer = automaton->count + 1;
			}
			break;

		case STORE_LENGTH:
			integer = wordlen;
			break;

		default:
			PyErr_SetString(PyExc_SystemError, "invalid store value");
			return NULL;
	}

	if (wordlen > 0) {
		bool new_word = false;
		TrieNode* node;
		node = trie_add_word(automaton, word, wordlen, &new_word);

		Py_DECREF(py_word);
		if (node) {
			switch (automaton->store) {
				case STORE_ANY:
					if (not new_word and node->eow)
						// replace
						Py_DECREF(node->output.object);
				
					Py_INCREF(py_value);
					node->output.object = py_value;
					break;

				default:
					node->output.integer = integer;
			} // switch

			if (new_word) {
				automaton->version += 1; // change version only when new word appeared
				if (wordlen > automaton->longest_word)
					automaton->longest_word = wordlen;
					
				Py_RETURN_TRUE;
			}
			else {
				Py_RETURN_FALSE;
			}
		}
		else
			return NULL;
	}

	Py_DECREF(py_word);
	Py_RETURN_FALSE;
}


static void
clear_aux(TrieNode* node, KeysStore store) {
	if (node) {
		switch (store) {
			case STORE_INTS:
			case STORE_LENGTH:
				// nop
				break;

			case STORE_ANY:
				if (node->output.object)
					Py_DECREF(node->output.object);
				break;
		}

		const int n = node->n;
		int i;
		for (i=0; i < n; i++) {
			TrieNode* child = node->next[i];
			if (child != node) // avoid self-loops!
				clear_aux(child, store);
		}

		memfree(node);
	}
#undef automaton
}


#define automaton_clear_doc\
	"removes all objects from dictionary"


static PyObject*
automaton_clear(PyObject* self, PyObject* args) {
#define automaton ((Automaton*)self)
	clear_aux(automaton->root, automaton->store);
	automaton->count = 0;
	automaton->longest_word = 0;
	automaton->kind = EMPTY;
	automaton->root = NULL;
	automaton->version += 1;

	Py_RETURN_NONE;
#undef automaton
}


static int
automaton_contains(PyObject* self, PyObject* args) {
#define automaton ((Automaton*)self)
	ssize_t wordlen;
	TRIE_LETTER_TYPE* word;
	PyObject* py_word;

	py_word = pymod_get_string(args, &word, &wordlen);
	if (py_word == NULL)
		return -1;

	TrieNode* node = trie_find(automaton->root, word, wordlen);
	Py_DECREF(py_word);

	return (node and node->eow);
#undef automaton
}


#define automaton_exists_doc \
	"exists(word) => bool - returns if word is in dictionary"

static PyObject*
automaton_exists(PyObject* self, PyObject* args) {
	PyObject* word;

	word = PyTuple_GetItem(args, 0);
	if (word)
		switch (automaton_contains(self, word)) {
			case 1:
				Py_RETURN_TRUE;

			case 0:
				Py_RETURN_FALSE;

			default:
				return NULL;
		}
	else
		return NULL;
}


#define automaton_match_doc \
	"match(word) => bool - returns if there is a prefix equal to word"

static PyObject*
automaton_match(PyObject* self, PyObject* args) {
#define automaton ((Automaton*)self)
	ssize_t wordlen;
	TRIE_LETTER_TYPE* word;
	PyObject* py_word;

	py_word = pymod_get_string_from_tuple(args, 0, &word, &wordlen);
	if (py_word == NULL)
		return NULL;

	TrieNode* node = trie_find(automaton->root, word, wordlen);;

	Py_DECREF(py_word);
	if (node)
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;
#undef automaton
}


#define automaton_longest_prefix_doc \
	"longest_prefix(word) => integer - length of longest prefix"

static PyObject*
automaton_longest_prefix(PyObject* self, PyObject* args) {
#define automaton ((Automaton*)self)
	ssize_t wordlen;
	TRIE_LETTER_TYPE* word;
	PyObject* py_word;

	py_word = pymod_get_string_from_tuple(args, 0, &word, &wordlen);
	if (py_word == NULL)
		return NULL;

	const int len = trie_longest(automaton->root, word, wordlen);
	Py_DECREF(py_word);

	return Py_BuildValue("i", len);
#undef automaton
}


#define automaton_get_doc \
	"get(word, [def]) => obj - returns object associated with given word; " \
	"if word isn't present, then def is returned, when def isn't defined, " \
	"raise KeyError exception"

static PyObject*
automaton_get(PyObject* self, PyObject* args) {
#define automaton ((Automaton*)self)
	ssize_t wordlen;
	TRIE_LETTER_TYPE* word;
	PyObject* py_word;
	PyObject* py_def;

	py_word = pymod_get_string_from_tuple(args, 0, &word, &wordlen);
	if (py_word == NULL)
		return NULL;

	TrieNode* node = trie_find(automaton->root, word, wordlen);

	if (node and node->eow) {
		switch (automaton->store) {
			case STORE_INTS:
			case STORE_LENGTH:
				return Py_BuildValue("i", node->output.integer);

			case STORE_ANY:
				Py_INCREF(node->output.object);
				return node->output.object;

			default:
				PyErr_SetNone(PyExc_ValueError);
				return NULL;
		}
	}
	else {
		py_def = PyTuple_GetItem(args, 1);
		if (py_def) {
			Py_INCREF(py_def);
			return py_def;
		}
		else {
			PyErr_Clear();
			PyErr_SetNone(PyExc_KeyError);
			return NULL;
		}
	}
#undef automaton
}

typedef struct AutomatonQueueItem {
	LISTITEM_data;
	TrieNode*	node;
} AutomatonQueueItem;

#define automaton_make_automaton_doc \
	"convert trie to Aho-Corasick automaton"

static PyObject*
automaton_make_automaton(PyObject* self, PyObject* args) {
#define automaton ((Automaton*)self)
	if (automaton->kind != TRIE)
		Py_RETURN_FALSE;

	AutomatonQueueItem* item;
	List queue;
	int i;

	list_init(&queue);

	// 1. setup nodes at 1-st level
	ASSERT(automaton->root);

	for (i=0; i < 256; i++) {
		TrieNode* child = trienode_get_next(automaton->root, i);
		if (child) {
			// fail edges go to root
			child->fail = automaton->root;

			item = (AutomatonQueueItem*)list_item_new(sizeof(AutomatonQueueItem));
			if (item) {
				item->node = child;
				list_append(&queue, (ListItem*)item);
			}
			else
				goto no_mem;
		}
		else
			// loop on root - implicit (see automaton_next)
			;
	}

	// 2. make links
	TrieNode* node;
	TrieNode* child;
	TrieNode* state;
	while (true) {
		AutomatonQueueItem* item = (AutomatonQueueItem*)list_pop_first(&queue);
		if (item == NULL)
			break;
		else {
			node = item->node;
			memfree(item);
		}

		const size_t n = node->n;
		for (i=0; i < n; i++) {
			child = node->next[i];
			ASSERT(child);

			item = (AutomatonQueueItem*)list_item_new(sizeof(AutomatonQueueItem));
			if (item) {
				item->node = child;
				list_append(&queue, (ListItem*)item);
			}
			else
				goto no_mem;

			state = node->fail;
			ASSERT(state);
			ASSERT(child);
			while (state != automaton->root and\
				   not trienode_get_next(state, child->letter)) {

				state = state->fail;
				ASSERT(state);
			}

			child->fail = trienode_get_next(state, child->letter);
			if (child->fail == NULL)
				child->fail = automaton->root;
			
			ASSERT(child->fail);
		}
	}

	automaton->kind = AHOCORASICK;
	automaton->version += 1;
	list_delete(&queue);
	Py_RETURN_NONE;
#undef automaton

no_mem:
	list_delete(&queue);
	PyErr_NoMemory();
	return NULL;
}


#define automaton_find_all_doc \
	"find_all(string, callback, [start, [end]])"

static PyObject*
automaton_find_all(PyObject* self, PyObject* args) {
#define automaton ((Automaton*)self)
	if (automaton->kind != AHOCORASICK)
		Py_RETURN_NONE;

	ssize_t wordlen;
	ssize_t start;
	ssize_t end;
	TRIE_LETTER_TYPE* word;
	PyObject* py_word;
	PyObject* callback;
	PyObject* callback_ret;

	// arg 1
	py_word = pymod_get_string_from_tuple(args, 0, &word, &wordlen);
	if (py_word == NULL)
		return NULL;

	// arg 2
	callback = PyTuple_GetItem(args, 1);
	if (callback == NULL)
		return NULL;
	else
	if (not PyCallable_Check(callback)) {
		PyErr_SetString(PyExc_TypeError, "second argument isn't callable");
		return NULL;
	}

	// parse start/end
	if (pymod_parse_start_end(args, 2, 3, 0, wordlen, &start, &end))
		return NULL;

	ssize_t i;
	TrieNode* state;
	TrieNode* tmp;

	state = automaton->root;
	for (i=start; i < end; i++) {
		state = tmp = ahocorasick_next(state, automaton->root, word[i]);

		// return output
		while (tmp and tmp->eow) {
			if (automaton->store == STORE_ANY)
				callback_ret = PyObject_CallFunction(callback, "iO", i, tmp->output.object);
			else
				callback_ret = PyObject_CallFunction(callback, "ii", i, tmp->output.integer);

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


static PyObject*
automaton_items_create(PyObject* self, PyObject* args, const ItemsType type) {
#define automaton ((Automaton*)self)
	PyObject* arg1 = NULL;
	PyObject* arg2 = NULL;
	PyObject* arg3 = NULL;
	TRIE_LETTER_TYPE* word;
	ssize_t wordlen;

	TRIE_LETTER_TYPE wildcard;
	bool use_wildcard = false;
	PatternMatchType matchtype = MATCH_AT_LEAST_PREFIX;

	// arg 1: prefix/prefix pattern
	if (args) 
		arg1 = PyTuple_GetItem(args, 0);
	else
		arg1 = NULL;
	
	if (arg1) {
		arg1 = pymod_get_string(arg1, &word, &wordlen);
		if (arg1 == NULL)
			goto error;
	}
	else {
		PyErr_Clear();
		word = NULL;
		wordlen = 0;
	}

	// arg 2: wildcard
	if (args)
		arg2 = PyTuple_GetItem(args, 1);
	else
		arg2 = NULL;

	if (arg2) {
		TRIE_LETTER_TYPE* tmp;
		ssize_t len;

		arg2 = pymod_get_string(arg2, &tmp, &len);
		if (arg2 == NULL)
			goto error;
		else {
			if (len == 1) {
				wildcard = tmp[0];
				use_wildcard = true;
			}
			else {
				PyErr_SetString(PyExc_ValueError, "wildcard have to be single character");
				goto error;
			}
		}
	}
	else {
		PyErr_Clear();
		wildcard = 0;
		use_wildcard = false;
	}

	// arg3: matchtype
	matchtype = MATCH_AT_LEAST_PREFIX;
	if (args) {
		arg3 = PyTuple_GetItem(args, 2);
		if (arg3) {
			Py_ssize_t val = PyNumber_AsSsize_t(arg3, PyExc_OverflowError);
			if (val == -1 and PyErr_Occurred())
				goto error;

			switch ((PatternMatchType)val) {
				case MATCH_AT_LEAST_PREFIX:
				case MATCH_AT_MOST_PREFIX:
				case MATCH_EXACT_LENGTH:
					matchtype = (PatternMatchType)val;
					break;

				default:
					PyErr_SetString(PyExc_ValueError,
						"third argument have to be one of MATCH_EXACT_LENGTH, "
						"MATCH_AT_LEAST_PREFIX, MATCH_AT_LEAST_PREFIX"
					);
					goto error;
			}
		}
		else {
			PyErr_Clear();
			if (use_wildcard)
				matchtype = MATCH_EXACT_LENGTH;
			else
				matchtype = MATCH_AT_LEAST_PREFIX;
		}
	}


	// 
	AutomatonItemsIter* iter = (AutomatonItemsIter*)automaton_items_iter_new(
									automaton,
									word,
									wordlen,
									use_wildcard,
									wildcard,
									matchtype
								);
	Py_XDECREF(arg1);
	Py_XDECREF(arg2);

	if (iter) {
		iter->type = type;
		return (PyObject*)iter;
	}
	else
		return NULL;


error:
	Py_XDECREF(arg1);
	Py_XDECREF(arg2);
	return NULL;
#undef automaton
}


#define automaton_keys_doc \
	"iterator for keys"

static PyObject*
automaton_keys(PyObject* self, PyObject* args) {
	return automaton_items_create(self, args, ITER_KEYS);
}


static PyObject*
automaton_iterate(PyObject* self) {
	return automaton_items_create(self, NULL, ITER_KEYS);
}


#define automaton_values_doc \
	"iterator for values"

static PyObject*
automaton_values(PyObject* self, PyObject* args) {
	return automaton_items_create(self, args, ITER_VALUES);
}


#define automaton_items_doc \
	"iterator for items"

static PyObject*
automaton_items(PyObject* self, PyObject* args) {
	return automaton_items_create(self, args, ITER_ITEMS);
}


#define automaton_iter_doc \
	"iter(string|buffer, [start, [end]])"

static PyObject*
automaton_iter(PyObject* self, PyObject* args) {
#define automaton ((Automaton*)self)

	if (automaton->kind != AHOCORASICK) {
		PyErr_SetString(PyExc_AttributeError, "not an automaton yet; add some words and call make_automaton");
		return NULL;
	}

	PyObject* object;
	ssize_t start;
	ssize_t end;

	object = PyTuple_GetItem(args, 0);
	if (object) {
#ifdef AHOCORASICK_UNICODE
		if (PyUnicode_Check(object)) {
			start	= 0;
			end		= PyUnicode_GET_SIZE(object);
		}
		else {
			PyErr_SetString(PyExc_TypeError, "string required");
			return NULL;
		}
#else
		if (PyBytes_Check(object)) {
			start 	= 0;
			end		= PyBytes_GET_SIZE(object);
		}
		else {
			PyErr_SetString(PyExc_TypeError, "bytes required");
			return NULL;
		}
#endif
	}
	else
		return NULL;

	if (pymod_parse_start_end(args, 1, 2, start, end, &start, &end))
		return NULL;

	return automaton_search_iter_new(
		automaton,
		object,
		start,
		end
	);
#undef automaton
}


static void
get_stats_aux(TrieNode* node, AutomatonStatistics* stats, int depth) {
	stats->nodes_count	+= 1;
	stats->words_count	+= (int)(node->eow);
	stats->links_count	+= node->n;
	stats->total_size	+= trienode_get_size(node);
	if (depth > stats->longest_word)
		stats->longest_word = depth;

	int i;
	for (i=0; i < node->n; i++)
		get_stats_aux(node->next[i], stats, depth + 1);
}

static void
get_stats(Automaton* automaton) {
	automaton->stats.nodes_count	= 0;
	automaton->stats.words_count	= 0;
	automaton->stats.longest_word	= 0;
	automaton->stats.links_count	= 0;
	automaton->stats.sizeof_node	= sizeof(TrieNode);
	automaton->stats.total_size		= 0;

	if (automaton->kind != EMPTY)
		get_stats_aux(automaton->root, &automaton->stats, 0);
	
	automaton->stats.version		= automaton->version;
}


#define automaton_get_stats_doc \
	"returns statistics about automaton"

static PyObject*
automaton_get_stats(PyObject* self, PyObject* args) {
#define automaton ((Automaton*)self)
	if (automaton->stats.version != automaton->version)
		get_stats(automaton);
	
	PyObject* dict = Py_BuildValue(
		"{s:i,s:i,s:i,s:i,s:i,s:i}",
#define emit(name) #name, automaton->stats.name
		emit(nodes_count),
		emit(words_count),
		emit(longest_word),
		emit(links_count),
		emit(sizeof_node),
		emit(total_size)
#undef emit
	);
	return dict;
#undef automaton
}


typedef struct DumpAux {
	PyObject*	nodes;
	PyObject*	edges;
	PyObject*	fail;
	char		error;
} DumpAux;

static int
dump_aux(TrieNode* node, const int depth, void* extra) {
#define Dump ((DumpAux*)extra)
	PyObject* tuple;
	TrieNode* child;
	int i;

#define append_tuple(list) \
	if (tuple == NULL) { \
		Dump->error = 1; \
		return 0; \
	} \
	else if (PyList_Append(list, tuple) < 0) { \
		Dump->error = 1; \
		return 0; \
	}
		
	
	// 1.
	tuple = Py_BuildValue("ii", node, (int)(node->eow));
	append_tuple(Dump->nodes)

	// 2.
	for (i=0; i < node->n; i++) {
		child = node->next[i];
		tuple = Py_BuildValue("ici", node, child->letter, child);
		append_tuple(Dump->edges)
	}

	// 3.
	if (node->fail) {
		tuple = Py_BuildValue("ii", node, node->fail);
		append_tuple(Dump->fail);
	}

	return 1;
#undef append_tuple
#undef Dump
}


#define automaton_dump_doc \
	"dump information about links and fail links"


static PyObject*
automaton_dump(PyObject* self, PyObject* args) {
#define automaton ((Automaton*)self)
	if (automaton->kind == EMPTY)
		Py_RETURN_NONE;
	
	DumpAux dump;
	dump.nodes = 0;
	dump.edges = 0;
	dump.fail  = 0;
	dump.error = 0;

	dump.nodes = PyList_New(0);
	dump.edges = PyList_New(0);
	dump.fail  = PyList_New(0);
	if (dump.edges == NULL or dump.fail == NULL or dump.nodes == NULL)
		goto error;

	trie_traverse(automaton->root, dump_aux, &dump);
	if (dump.error)
		goto error;
	else
		return Py_BuildValue("OOO", dump.nodes, dump.edges, dump.fail);

error:
	Py_XDECREF(dump.nodes);
	Py_XDECREF(dump.edges);
	Py_XDECREF(dump.fail);
	return NULL;

#undef automaton
}



#include "Automaton_pickle.c"


#define method(name, kind) {#name, automaton_##name, kind, automaton_##name##_doc}
static
PyMethodDef automaton_methods[] = {
	method(add_word,		METH_VARARGS),
	method(clear,			METH_NOARGS),
	method(exists,			METH_VARARGS),
	method(match,			METH_VARARGS),
	method(longest_prefix,	METH_VARARGS),
	method(get,				METH_VARARGS),
	method(make_automaton,	METH_NOARGS),
	method(find_all,		METH_VARARGS),
	method(iter,			METH_VARARGS),
	method(keys,			METH_VARARGS),
	method(values,			METH_VARARGS),
	method(items,			METH_VARARGS),
	method(get_stats,		METH_NOARGS),
	method(dump,			METH_NOARGS),
	method(__reduce__,		METH_VARARGS),

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

	{
		"store",
		T_INT,
		offsetof(Automaton, store),
		READONLY,
		"type of values (ahocorasick.STORE_ANY/STORE_INTS/STORE_LEN)"
	},

	{NULL}
};

static PyTypeObject automaton_type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	"ahocorasick.Automaton",					/* tp_name */
	sizeof(Automaton),							/* tp_size */
	0,											/* tp_itemsize? */
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
	automaton_iterate,							/* tp_iter */
	0,                                          /* tp_iternext */
	automaton_methods,							/* tp_methods */
	automaton_members,			                /* tp_members */
	0,                                          /* tp_getset */
	0,                                          /* tp_base */
	0,                                          /* tp_dict */
	0,                                          /* tp_descr_get */
	0,                                          /* tp_descr_set */
	0,                                          /* tp_dictoffset */
	0,											/* tp_init */
	0,                                          /* tp_alloc */
	automaton_new,								/* tp_new */
};

