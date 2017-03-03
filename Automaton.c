/*
	This is part of pyahocorasick Python module.

	Automaton class implementation.
	(this file includes Automaton_pickle.c)

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
    License   : BSD-3-Clause (see LICENSE)
*/

#include "Automaton.h"
#include "slist.h"

static PyTypeObject automaton_type;

#define automaton_doc \
	"Automaton(value_type=ahocorasick.STORE_ANY)\n\n" \
	"Create a new empty Automaton. value_type is optional and one of these constants:\n" \
	" - ahocorasick.STORE_ANY : The associated value can be any Python object (default).\n" \
	" - ahocorasick.STORE_LENGTH : The length of an added string key is automatically\n" \
	"   used as the associated value stored in the trie for that key.\n" \
	" - ahocorasick.STORE_INTS : The associated value must be a 32-bit integer."


static bool
check_store(const int store) {
	switch (store) {
		case STORE_LENGTH:
		case STORE_INTS:
		case STORE_ANY:
			return true;

		default:
			PyErr_SetString(
				PyExc_ValueError,
				"store value must be one of ahocorasick.STORE_LENGTH, STORE_INTS or STORE_ANY"
			);
			return false;
	} // switch
}


static bool
check_kind(const int kind) {
	switch (kind) {
		case EMPTY:
		case TRIE:
		case AHOCORASICK:
			return true;

		default:
			PyErr_SetString(
				PyExc_ValueError,
				"kind value must be one of ahocorasick.EMPTY, TRIE or AHOCORASICK"
			);
			return false;
	}
}


static bool
check_key_type(const int store) {
	switch (store) {
		case KEY_STRING:
		case KEY_SEQUENCE:
			return true;

		default:
			PyErr_SetString(
				PyExc_ValueError,
				"key_type must have value KEY_STRING or KEY_SEQUENCE"
			);
			return false;
	} // switch
}


static PyObject*
automaton_new(PyTypeObject* self, PyObject* args, PyObject* kwargs) {
	Automaton* automaton = NULL;
	int key_type;
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

	if (UNLIKELY(PyTuple_Size(args) == 9)) {

		// unpickle: count, data, kind, store, version, values
		size_t			count;
		void*			data;
		size_t			size;
		int				version;
		int				word_count;
		int				longest_word;
		AutomatonKind	kind;
		KeysStore		store;
		KeyType			key_type;
		PyObject*		values = NULL;

#ifdef PY3K
        const char* fmt = "ky#iiiiiiO";
#else
        const char* fmt = "ks#iiiiiiO";
#endif

		if (not PyArg_ParseTuple(args, fmt, &count, &data, &size, &kind, &store, &key_type, &version, &word_count, &longest_word, &values)) {
			PyErr_SetString(PyExc_ValueError, "Invalid data: unable to load from pickle.");
			goto error;
		}
		printf("Parsed count %zu\n", count);

		if (!check_store(store) || !check_kind(kind) || !check_key_type(key_type)) {
			goto error;
		}

		if (kind != EMPTY) {
			if (values == Py_None) {
				Py_XDECREF(values);
				values = NULL;
			}

			if (automaton_unpickle(automaton, count, data, size, values)) {
				automaton->kind		= kind;
				automaton->store	= store;
				automaton->key_type	= key_type;
				automaton->version	= version;
				automaton->count    = word_count;
				automaton->longest_word	= longest_word;
			}
			else
				goto error;
		}

		Py_XDECREF(values);
	}
	else {
		store    = STORE_ANY;
		key_type = KEY_STRING;

		// construct new object
		if (PyArg_ParseTuple(args, "ii", &store, &key_type)) {
			if (not check_store(store)) {
				goto error;
			}

			if (not check_key_type(key_type)) {
				goto error;
			}
		}
		else if (PyArg_ParseTuple(args, "i", &store)) {
			if (not check_store(store)) {
				goto error;
			}
		}

		PyErr_Clear();
		automaton->store    = store;
		automaton->key_type = key_type;
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
	"Return the number of distinct keys added to the trie."

static ssize_t
automaton_len(PyObject* self) {
#define automaton ((Automaton*)self)
	return automaton->count;
#undef automaton
}


#define automaton_add_word_doc \
	"add_word(key, [value])\n\n" \
	"Add a key string to the dict-like trie and associate this key with a value.\n" \
	"value is optional or mandatory depending how the Automaton instance was created.\n" \
	"Return True if the word key is inserted and did not exists in the trie or False\n" \
	"otherwise.\n\n" \
	"The value is either mandatory or optional:\n" \
	" - If the Automaton was created without argument (the default) as Automaton() or\n" \
	"   with Automaton(ahocorasik.STORE_ANY) then the value is required and can be any\n" \
	"   Python object.\n\n" \
	" - If the Automaton was created with Automaton(ahocorasik.STORE_LENGTH) then\n" \
	"   associating a value is not allowed --- len(word) is saved automatically as a\n" \
	"   value instead.\n\n" \
	" - If the Automaton was created with Automaton(ahocorasik.STORE_INTS) then the\n" \
	"   value is optional. If provided it must be an integer, otherwise it defaults to\n" \
	"   len(automaton) which is therefore the order index in which keys are added to the\n" \
	"   trie.\n\n" \
	"Calling add_word() invalidates all iterators only if the new key did not exist\n" \
	"in the trie so far (i.e. the method returned True)."

static PyObject*
automaton_add_word(PyObject* self, PyObject* args) {
#define automaton ((Automaton*)self)
	// argument
	PyObject* py_value = NULL;
	struct Input input;

	Py_uintptr_t integer = 0;
	TrieNode* node;
	bool new_word;

	if (!prepare_input_from_tuple(self, args, 0, &input)) {
		return NULL;
	}

	switch (automaton->store) {
		case STORE_ANY:
			py_value = PyTuple_GetItem(args, 1);
			if (not py_value) {
				PyErr_SetString(PyExc_ValueError, "A value object is required as second argument.");
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
					PyErr_SetString(PyExc_TypeError, "An integer value is required as second argument.");
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
			integer = input.wordlen;
			break;

		default:
			PyErr_SetString(PyExc_SystemError, "Invalid value for this key: see documentation for supported values.");
			return NULL;
	}

	node = NULL;
	new_word = false;

	if (input.wordlen > 0) {
		node = trie_add_word(automaton, input.word, input.wordlen, &new_word);
		destroy_input(&input);

		if (node == NULL) {
			return NULL;
		}
	} else {
		destroy_input(&input);
	}

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
			if (input.wordlen > automaton->longest_word)
				automaton->longest_word = (int)input.wordlen;

			Py_RETURN_TRUE;
		}
		else {
			Py_RETURN_FALSE;
		}
	}

	Py_RETURN_FALSE;
}


static void
clear_aux(TrieNode* node, KeysStore store) {

	int i;

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

		for (i=0; i < node->n; i++) {
			TrieNode* child = node->next[i];
			if (child != node) // avoid self-loops!
				clear_aux(child, store);
		}

		xfree(node->next);
		memory_free(node);
	}
#undef automaton
}


#define automaton_clear_doc\
	"Remove all keys from the trie. This method invalidates all iterators."

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
	TrieNode* node;
	struct Input input;

	if (!prepare_input(self, args, &input)) {
		return -1;
	}

	node = trie_find(automaton->root, input.word, input.wordlen);

	destroy_input(&input);

	return (node and node->eow);
#undef automaton
}


#define automaton_exists_doc \
	"exists(key)\n\n" \
	"Return True if the key is present in the trie. Same as using the 'in' keyword."

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
	"match(key)\n\n" \
	"Return True if there is a prefix (or key) equal to key present in the trie.\n\n" \
	"For example if the key 'example' has been added to the trie, then calls to\n" \
	"match('e'), match('ex'), ..., match('exampl') or match('example') all return\n" \
	"True. But exists() is True only when calling exists('example')"

static PyObject*
automaton_match(PyObject* self, PyObject* args) {
#define automaton ((Automaton*)self)
	TrieNode* node;
	struct Input input;

	if (!prepare_input_from_tuple(self, args, 0, &input)) {
		return NULL;
	}

	node = trie_find(automaton->root, input.word, input.wordlen);;

	destroy_input(&input);

	if (node)
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;
#undef automaton
}


#define automaton_longest_prefix_doc \
	"longest_prefix(string)\n\n" \
	"Return the length of the longest prefix of string that exists in the trie."

static PyObject*
automaton_longest_prefix(PyObject* self, PyObject* args) {
#define automaton ((Automaton*)self)
	int len;
	struct Input input;

	if (!prepare_input_from_tuple(self, args, 0, &input)) {
		return NULL;
	}

	len = trie_longest(automaton->root, input.word, input.wordlen);

	destroy_input(&input);

	return Py_BuildValue("i", len);
#undef automaton
}


#define automaton_get_doc \
	"get(key[, default])\n\n" \
	"Return the value associated with the key string.\n" \
	"Raise a KeyError exception if the key is not in the trie and no default is provided.\n" \
	"Return the optional default value if provided and the key is not in the trie."

static PyObject*
automaton_get(PyObject* self, PyObject* args) {
#define automaton ((Automaton*)self)
	struct Input input;
	PyObject* py_def;
	TrieNode* node;

	if (!prepare_input_from_tuple(self, args, 0, &input)) {
		return NULL;
	}

	node = trie_find(automaton->root, input.word, input.wordlen);

	destroy_input(&input);

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
	"Finalize and create the Aho-Corasick automaton based on the keys already added\n" \
	"to the trie. This does not require additional memory. After successful creation\n" \
	"the Automaton.kind attribute is set to ahocorasick.AHOCORASICK."

static PyObject*
automaton_make_automaton(PyObject* self, PyObject* args) {
#define automaton ((Automaton*)self)

	AutomatonQueueItem* item;
	List queue;
	int i;

	TrieNode* node;
	TrieNode* child;
	TrieNode* state;


	if (automaton->kind != TRIE)
		Py_RETURN_FALSE;

	list_init(&queue);

	// 1. setup nodes at first level: they fail back to the root
	ASSERT(automaton->root);

	for (i=0; i < automaton->root->n; i++) {
		TrieNode* child = trienode_get_ith_unsafe(automaton->root, i);
		ASSERT(child);
		// fail edges go to the root
		// every other letters loop on root - implicit (see automaton_next)
		child->fail = automaton->root;

		item = (AutomatonQueueItem*)list_item_new(sizeof(AutomatonQueueItem));
		if (item) {
			item->node = child;
			list_append(&queue, (ListItem*)item);
		}
		else
			goto no_mem;
	}

	// 2. make links
	while (true) {
		AutomatonQueueItem* item = (AutomatonQueueItem*)list_pop_first(&queue);
		if (item == NULL)
			break;
		else {
			node = item->node;
			memory_free(item);
		}

		for (i=0; i < node->n; i++) {
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
	"find_all(string, callback, [start, [end]])\n\n" \
	"Perform the Aho-Corasick search procedure using the provided input string and\n" \
	"iterate over the matching tuples (end_index, value) for keys found in string.\n" \
	"Invoke the callback callable for each matching tuple.\n\n" \
	"The callback callable must accept two positional arguments:\n" \
	" - end_index is the end index in the input string where a trie key string was found.\n" \
	" - value is the value associated with the found key string.\n\n" \
	"The start and end optional arguments can be used to limit the search to an\n" \
	"input string slice as in string[start:end].\n\n" \
	"Equivalent to a loop on iter() calling a callable at each iteration."

static PyObject*
automaton_find_all(PyObject* self, PyObject* args) {
#define automaton ((Automaton*)self)

	struct Input input;
	ssize_t start;
	ssize_t end;
	PyObject* callback;
	PyObject* callback_ret;

	ssize_t i;
	TrieNode* state;
	TrieNode* tmp;

	if (automaton->kind != AHOCORASICK)
		Py_RETURN_NONE;

	// arg 1
	if (!prepare_input_from_tuple(self, args, 0, &input)) {
		return NULL;
	}

	// arg 2
	callback = PyTuple_GetItem(args, 1);
	if (callback == NULL) {
		destroy_input(&input);
		return NULL;
	}
	else
	if (not PyCallable_Check(callback)) {
		PyErr_SetString(PyExc_TypeError, "The callback argument must be a callable such as a function.");
		destroy_input(&input);
		return NULL;
	}

	// parse start/end
	if (pymod_parse_start_end(args, 2, 3, 0, input.wordlen, &start, &end)) {
		destroy_input(&input);
		return NULL;
	}

	state = automaton->root;
	for (i=start; i < end; i++) {
		state = tmp = ahocorasick_next(state, automaton->root, input.word[i]);

		// return output
		while (tmp) {
			if (tmp->eow) {
				if (automaton->store == STORE_ANY)
					callback_ret = PyObject_CallFunction(callback, "iO", i, tmp->output.object);
				else
					callback_ret = PyObject_CallFunction(callback, "ii", i, tmp->output.integer);

				if (callback_ret == NULL) {
					destroy_input(&input);
					return NULL;
				} else
					Py_DECREF(callback_ret);
			}

			tmp = tmp->fail;
		}
	}
#undef automaton

	destroy_input(&input);
	Py_RETURN_NONE;
}


static PyObject*
automaton_items_create(PyObject* self, PyObject* args, const ItemsType type) {
#define automaton ((Automaton*)self)
	PyObject* arg1 = NULL;
	PyObject* arg2 = NULL;
	PyObject* arg3 = NULL;
	TRIE_LETTER_TYPE* word = NULL;
	ssize_t wordlen = 0;

	TRIE_LETTER_TYPE wildcard;
	bool use_wildcard = false;
	PatternMatchType matchtype = MATCH_AT_LEAST_PREFIX;

	AutomatonItemsIter* iter;

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
		ssize_t len = 0;

		arg2 = pymod_get_string(arg2, &tmp, &len);
		if (arg2 == NULL)
			goto error;
		else {
			if (len == 1) {
				wildcard = tmp[0];
				use_wildcard = true;
			}
			else {
				PyErr_SetString(PyExc_ValueError, "Wildcard must be a single character.");
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
						"The optional how third argument must beone of:\n"
						"MATCH_EXACT_LENGTH, MATCH_AT_LEAST_PREFIX or MATCH_AT_LEAST_PREFIX"
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
	iter = (AutomatonItemsIter*)automaton_items_iter_new(
					automaton,
					word,
					wordlen,
					use_wildcard,
					wildcard,
					matchtype);

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
	"keys([prefix, [wildcard, [how]]])\n\n" \
	"Return an iterator on keys.\n" \
	"If the optional prefix string is provided, only yield keys starting with this prefix.\n" \
	"If the optional wildcard is provided as a single character string, then the\n" \
	"prefix is treated as a simple pattern using this character as a wildcard.\n\n" \
	"The optional how argument is used to control how strings are matched using one\n" \
	"of these possible values:\n\n" \
	" - ahocorasick.MATCH_EXACT_LENGTH (default)\n" \
	"   Yield matches that have the same exact length as the prefix length.\n" \
	" - ahocorasick.MATCH_AT_LEAST_PREFIX\n" \
	"   Yield matches that have a length greater or equal to the prefix length.\n" \
	" - ahocorasick.MATCH_AT_MOST_PREFIX\n" \
	"   Yield matches that have a length lesser or equal to the prefix length."

static PyObject*
automaton_keys(PyObject* self, PyObject* args) {
	return automaton_items_create(self, args, ITER_KEYS);
}


static PyObject*
automaton_iterate(PyObject* self) {
	return automaton_items_create(self, NULL, ITER_KEYS);
}


#define automaton_values_doc \
	"values([prefix, [wildcard, [how]]])\n\n" \
	"Return an iterator on values associated with each keys.\n" \
	"Keys are matched optionally to the prefix using the same logic and\n" \
	"arguments as in the keys() method."

static PyObject*
automaton_values(PyObject* self, PyObject* args) {
	return automaton_items_create(self, args, ITER_VALUES);
}


#define automaton_items_doc \
	"items([prefix, [wildcard, [how]]])\n\n" \
	"Return an iterator on tuples of (key, value).\n" \
	"Keys are matched optionally to the prefix using the same logic and\n" \
	"arguments as in the keys() method."

static PyObject*
automaton_items(PyObject* self, PyObject* args) {
	return automaton_items_create(self, args, ITER_ITEMS);
}


#define automaton_iter_doc \
	"iter(string, [start, [end]])\n\n" \
	"Perform the Aho-Corasick search procedure using the provided input string.\n" \
	"Return an iterator of tuples (end_index, value) for keys found in string where:\n\n" \
	"- end_index is the end index in the input string where a trie key string was found.\n" \
	"- value is the value associated with the found key string.\n\n" \
	"The start and end optional arguments can be used to limit the search to an\n" \
	"input string slice as in string[start:end]."

static PyObject*
automaton_iter(PyObject* self, PyObject* args) {
#define automaton ((Automaton*)self)

	PyObject* object;
	ssize_t start;
	ssize_t end;

	if (automaton->kind != AHOCORASICK) {
		PyErr_SetString(PyExc_AttributeError,"Not an Aho-Corasick automaton yet: "
			"call add_word to add some keys and call make_automaton to "
			"convert the trie to an automaton.");
		return NULL;
	}

	object = PyTuple_GetItem(args, 0);
	if (object) {
#ifdef PY3K
    #ifdef AHOCORASICK_UNICODE
		if (PyUnicode_Check(object)) {
			start	= 0;
			#if PY_MINOR_VERSION >= 3
				end = PyUnicode_GET_LENGTH(object);
			#else
				end = PyUnicode_GET_SIZE(object);
			#endif
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
#else
        if (PyString_Check(object)) {
			start	= 0;
			end		= PyString_GET_SIZE(object);
        } else {
			PyErr_SetString(PyExc_TypeError, "string required");
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
		(int)start,
		(int)end
	);
#undef automaton
}


static void
get_stats_aux(TrieNode* node, AutomatonStatistics* stats, int depth) {

	int i;

	stats->nodes_count	+= 1;
	stats->words_count	+= (int)(node->eow);
	stats->links_count	+= node->n;
	stats->total_size	+= trienode_get_size(node);

	if (depth > stats->longest_word)
		stats->longest_word = depth;

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
	"get_stats()\n\n" \
	"Return a dictionary containing Automaton statistics.\n" \
	" - nodes_count   --- total number of nodes\n" \
	" - words_count   --- same as len(automaton)\n" \
	" - longest_word  --- length of the longest word\n" \
	" - links_count   --- number of edges\n" \
	" - sizeof_node   --- size of single node in bytes\n" \
	" - total_size    --- total size of trie in bytes (about\n" \
	"   nodes_count * size_of node + links_count * size of pointer)."

static PyObject*
automaton_get_stats(PyObject* self, PyObject* args) {
#define automaton ((Automaton*)self)

	PyObject* dict;

	if (automaton->stats.version != automaton->version)
		get_stats(automaton);

	dict = Py_BuildValue(
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
	"dump()\n\n" \
	"Return a three-tuple of lists describing the Automaton as a graph of " \
	"(nodes, edges, failure links).\n" \
	" - nodes: each item is a pair (node id, end of word marker)\n" \
	" - edges: each item is a triple (node id, label char, child node id)\n" \
	" - failure links: each item is a pair (source node id, node if connected by fail node)\n" \
	"\n" \
	"For each of these, the node id is a unique number and a label is a single byte."

static PyObject*
automaton_dump(PyObject* self, PyObject* args) {
#define automaton ((Automaton*)self)
	DumpAux dump;

	if (automaton->kind == EMPTY)
		Py_RETURN_NONE;

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


#define automaton___sizeof___doc \
	"Return the approximate size in bytes occupied by the Automaton instance in\n" \
	"memory excluding the size of associated objects when the Automaton is created\n" \
	"with Automaton() or Automaton(ahocorasick.STORE_ANY)."

static PyObject*
automaton___sizeof__(PyObject* self, PyObject* args) {
#define automaton ((Automaton*)self)
    Py_ssize_t size = sizeof(Automaton);

	if (automaton->kind != EMPTY) {
        if (automaton->stats.version != automaton->version) {
            get_stats(automaton);
        }

        size += automaton->stats.total_size;
    }

    return Py_BuildValue("i", size);
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
	method(__sizeof__,		METH_VARARGS),

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
		"Read-only attribute maintained automatically.\nKind for this Automaton instance.\nOne of ahocorasick.EMPTY, TRIE or AHOCORASICK."
	},

	{
		"store",
		T_INT,
		offsetof(Automaton, store),
		READONLY,
		"Read-only attribute set when creating an Automaton().\nType of values accepted by this Automaton.\nOne of ahocorasick.STORE_ANY, STORE_INTS or STORE_LEN."
	},

	{NULL}
};

static PyTypeObject automaton_type = {
	PY_OBJECT_HEAD_INIT
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
	automaton_doc,                              /* tp_doc */
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

// vim: noet
