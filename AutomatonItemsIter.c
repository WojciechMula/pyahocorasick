/*
	This is part of pyahocorasick Python module.
	
	AutomatonItemsIter implementation

	Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
	License   : 3-clauses BSD (see LICENSE)
*/
#include "AutomatonItemsIter.h"

static PyTypeObject automaton_items_iter_type;


typedef struct AutomatonItemsStackItem {
	LISTITEM_data;

	struct TrieNode*	node;
	size_t depth;
} AutomatonItemsStackItem;

#define StackItem AutomatonItemsStackItem

static PyObject*
automaton_items_iter_new(
	Automaton* automaton,
	const TRIE_LETTER_TYPE* word,
	const ssize_t wordlen,

	const bool use_wildcard,
	const TRIE_LETTER_TYPE wildcard,
	const PatternMatchType matchtype
) {
	AutomatonItemsIter* iter;
	StackItem* new_item;

	iter = (AutomatonItemsIter*)PyObject_New(AutomatonItemsIter, &automaton_items_iter_type);
	if (iter == NULL)
		return NULL;

	iter->automaton = automaton;
	iter->version	= automaton->version;
	iter->state	= NULL;
	iter->type = ITER_KEYS;
	iter->buffer = NULL;
	iter->pattern = NULL;
	iter->use_wildcard = use_wildcard;
	iter->wildcard = wildcard;
	iter->matchtype = matchtype;
	list_init(&iter->stack);

	iter->buffer = memalloc((automaton->longest_word + 1) * TRIE_LETTER_SIZE);
	if (iter->buffer == NULL) {
		PyObject_Del((PyObject*)iter);
		PyErr_NoMemory();
		return NULL;
	}
	
	if (word) {
		iter->pattern = (TRIE_LETTER_TYPE*)memalloc(wordlen * TRIE_LETTER_SIZE);
		if (UNLIKELY(iter->pattern == NULL)) {
			PyObject_Del((PyObject*)iter);
			PyErr_NoMemory();
			return NULL;
		}
		else {
			iter->pattern_length = wordlen;
			memcpy(iter->pattern, word, wordlen * TRIE_LETTER_SIZE);
		}
	}
	else
		iter->pattern_length = 0;
	
	new_item = (StackItem*)list_item_new(sizeof(StackItem));
	if (UNLIKELY(new_item == NULL)) {
		PyObject_Del((PyObject*)iter);
		PyErr_NoMemory();
		return NULL;
	}

	new_item->node = automaton->root;
	new_item->depth = 0;
	list_push_front(&iter->stack, (ListItem*)new_item);

	Py_INCREF((PyObject*)iter->automaton);
	return (PyObject*)iter;
}


#define iter ((AutomatonItemsIter*)self)

static void
automaton_items_iter_del(PyObject* self) {
	if (iter->buffer)
		memfree(iter->buffer);

	if (iter->pattern)
		memfree(iter->pattern);
	
	list_delete(&iter->stack);
	Py_DECREF(iter->automaton);

	PyObject_Del(self);
}


static PyObject*
automaton_items_iter_iter(PyObject* self) {
	Py_INCREF(self);
	return self;
}


static PyObject*
automaton_items_iter_next(PyObject* self) {

	bool output;

	if (UNLIKELY(iter->version != iter->automaton->version)) {
		PyErr_SetString(PyExc_ValueError, "underlaying automaton has changed, iterator is not valid anymore");
		return NULL;
	}

	while (true) {
		StackItem* item = (StackItem*)list_pop_first(&iter->stack);
		if (item == NULL or item->node == NULL)
			return NULL; /* Stop iteration */

		if (iter->matchtype != MATCH_AT_LEAST_PREFIX and item->depth > iter->pattern_length)
			continue;

		switch (iter->matchtype) {
			case MATCH_EXACT_LENGTH:
				output = (item->depth == iter->pattern_length);
				break;

			case MATCH_AT_MOST_PREFIX:
				output = (item->depth <= iter->pattern_length);
				break;
				
			case MATCH_AT_LEAST_PREFIX:
			default:
				output = (item->depth >= iter->pattern_length);
				break;

		}

		iter->state = item->node;
		if ((item->depth >= iter->pattern_length) or
		    (iter->use_wildcard and iter->pattern[item->depth] == iter->wildcard)) {

			// process all
			const int n = iter->state->n;
			int i;
			for (i=0; i < n; i++) {
				StackItem* new_item = (StackItem*)list_item_new(sizeof(StackItem));
				if (UNLIKELY(new_item == NULL)) {
					PyErr_NoMemory();
					return NULL;
				}

				new_item->node  = iter->state->next[i];
				new_item->depth = item->depth + 1;
				list_push_front(&iter->stack, (ListItem*)new_item);
			}
		}
		else {
			// process single letter
			TrieNode* node = trienode_get_next(iter->state, iter->pattern[item->depth]);

			if (node) {
				StackItem* new_item = (StackItem*)list_item_new(sizeof(StackItem));
				if (UNLIKELY(new_item == NULL)) {
					PyErr_NoMemory();
					return NULL;
				}

				new_item->node  = node;
				new_item->depth = item->depth + 1;
				list_push_front(&iter->stack, (ListItem*)new_item);
			}
		}

		if (iter->type != ITER_VALUES)
			// update keys when needed
			iter->buffer[item->depth] = iter->state->letter;

		if (output and iter->state->eow) {
			PyObject* val;

			switch (iter->type) {
				case ITER_KEYS:
#ifdef AHOCORASICK_UNICODE
					return PyUnicode_FromUnicode(iter->buffer + 1, item->depth);
#else
					return PyBytes_FromStringAndSize((char*)(iter->buffer + 1), item->depth);
#endif

				case ITER_VALUES:
					switch (iter->automaton->store) {
						case STORE_ANY:
							val = iter->state->output.object;
							Py_INCREF(val);
							break;

						case STORE_LENGTH:
						case STORE_INTS:
							return Py_BuildValue("i", iter->state->output.integer);

						default:
							PyErr_SetString(PyExc_SystemError, "wrong attribute 'store'");
							return NULL;
					}

					return val;

				case ITER_ITEMS:
					switch (iter->automaton->store) {
						case STORE_ANY:
							return Py_BuildValue(
#ifdef PY3K
    #ifdef AHOCORASICK_UNICODE
								"(u#O)",
    #else
								"(y#O)",
    #endif
#else
                                "(s#O)",
#endif
								/*key*/ iter->buffer + 1, item->depth,
								/*val*/ iter->state->output.object
							);

						case STORE_LENGTH:
						case STORE_INTS:
							return Py_BuildValue(
#ifdef PY3K
    #ifdef AHOCORASICK_UNICODE
								"(u#i)",
    #else
								"(y#i)",
    #endif
#else
                                "(s#i)",
#endif
								/*key*/ iter->buffer + 1, item->depth,
								/*val*/ iter->state->output.integer
							);
						
						default:
							PyErr_SetString(PyExc_SystemError, "wrong attribute 'store'");
							return NULL;
					} // switch
			}
		}
	}
}

#undef StackItem
#undef iter

static PyTypeObject automaton_items_iter_type = {
	PY_OBJECT_HEAD_INIT
	"AutomatonItemsIter",						/* tp_name */
	sizeof(AutomatonItemsIter),					/* tp_size */
	0,											/* tp_itemsize? */
	(destructor)automaton_items_iter_del,		/* tp_dealloc */
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
	automaton_items_iter_iter,					/* tp_iter */
	automaton_items_iter_next,					/* tp_iternext */
	0,											/* tp_methods */
	0,						                	/* tp_members */
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
