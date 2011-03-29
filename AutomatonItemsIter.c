/*
	This is part of pyahocorasick Python module.
	
	AutomatonItemsIter implementation

	Author    : Wojciech Mu³a, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl/proj/pyahocorasick/
	License   : 3-clauses BSD (see LICENSE)
	Date      : $Date$

	$Id$

*/
#include "AutomatonItemsIter.h"

static PyTypeObject automaton_items_iter_type;


typedef struct AutomatonItemsStackItem {
	LISTITEM_data

	struct TrieNode*	node;
	size_t depth;
} AutomatonItemsStackItem;

#define StackItem AutomatonItemsStackItem

static TrieNode*
trie_find_and_store_path(TrieNode* root, uint8_t* word, const ssize_t wordlen, char* buffer, size_t* n) {
	TrieNode* node = root;
	ssize_t i;
	for (i=0; i < wordlen; i++) {
		node = trienode_get_next(node, word[i]);
		if (node)
			buffer[i] = word[i];
		else
			return NULL;
	}

	*n = wordlen;
	return node;
}


static TrieNode*
trie_find_and_store_path_UCS2(TrieNode* root, uint16_t* word, const ssize_t wordlen, char* buffer, size_t* n) {
	TrieNode* node = root;
	ssize_t i, j;
	for (i=0, j=0; i < wordlen; i++) {
		uint8_t b;
		const uint16_t w = word[i];

		b = w & 0xff;
		node = trienode_get_next(node, b);
		if (node)
			buffer[j++] = b;
		else
			return NULL;

		if (w > 0x00ff) {
			b = (w >> 8) & 0xff;
			node = trienode_get_next(node, b);
			if (node)
				buffer[j++] = b;
			else
				return NULL;
		}
	}

	*n = j;
	return node;
}


static TrieNode*
trie_find_and_store_path_UCS4(TrieNode* root, uint32_t* word, const ssize_t wordlen, char* buffer, size_t* n) {
	TrieNode* node = root;
	ssize_t i, j;
	for (i=0, j=0; i < wordlen; i++) {
		uint8_t b;
		const uint32_t w = word[i];

		b = w & 0xff;
		node = trienode_get_next(node, b);
		if (node)
			buffer[j++] = b;
		else
			return NULL;

		if (w > 0x000000ff) {
			b = (w >> 8) & 0xff;
			node = trienode_get_next(node, b);
			if (node)
				buffer[j++] = b;
			else
				return NULL;

			if (w > 0x0000ffff) {
				b = (w >> 16) & 0xff;
				node = trienode_get_next(node, b);
				if (node)
					buffer[j++] = b;
				else
					return NULL;

				if (w > 0x00ffffff) {
					b = (w >> 24) & 0xff;
					node = trienode_get_next(node, b);
					if (node)
						buffer[j++] = b;
					else
						return NULL;
				}
			}
		}
	}

	*n = j;
	return node;
}


static PyObject*
automaton_items_iter_new(Automaton* automaton, uint8_t* word, const ssize_t wordlen, const bool unicode) {
	AutomatonItemsIter* iter;

	iter = (AutomatonItemsIter*)PyObject_New(AutomatonItemsIter, &automaton_items_iter_type);
	if (iter == NULL)
		return NULL;

	iter->automaton = automaton;
	iter->version	= automaton->version;
	iter->state	= NULL;
	iter->type = ITER_KEYS;
	list_init(&iter->stack);

	StackItem* new_item = (StackItem*)list_item_new(sizeof(StackItem));
	if (not new_item) {
		PyErr_SetNone(PyExc_MemoryError);
		return NULL;
	}

	size_t n_bytes;
	if (word) {
		if (unicode)
#ifdef Py_UNICODE_WIDE
			n_bytes = 4*wordlen;
#else
			n_bytes = 2*wordlen;
#endif
		else
			n_bytes = wordlen;
	}


	iter->n = (wordlen > 256) ? wordlen : 256;
	iter->buffer = memalloc(iter->n);
	if (iter->buffer == NULL) {
		PyObject_Del((PyObject*)iter);
		return NULL;
	}

	TrieNode* node = automaton->root;
	if (word != NULL) {
		if (unicode)
#ifdef Py_UNICODE_WIDE
			node = trie_find_and_store_path_UCS4(
					node,
					(uint32_t*)word,
					wordlen,
					iter->buffer + 1,
					&n_bytes);
#else
			node = trie_find_and_store_path_UCS2(
					node,
					(uint16_t*)word,
					wordlen,
					iter->buffer + 1,
					&n_bytes);
#endif
		else
			node = trie_find_and_store_path(
					node,
					word,
					wordlen,
					iter->buffer + 1,
					&n_bytes);
	}
	else
		n_bytes = 0;

	new_item->node = node;
	new_item->depth = n_bytes;
	list_push_front(&iter->stack, (ListItem*)new_item);

	Py_INCREF((PyObject*)iter->automaton);
	return (PyObject*)iter;
}


#define iter ((AutomatonItemsIter*)self)

static void
automaton_items_iter_del(PyObject* self) {
	if (iter->buffer)
		memfree(iter->buffer);
	
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
	if (UNLIKELY(iter->version != iter->automaton->version)) {
		PyErr_SetString(PyExc_ValueError, "underlaying automaton has changed, iterator is not valid anymore");
		return NULL;
	}

	while (true) {
		StackItem* item = (StackItem*)list_pop_first(&iter->stack);
		if (item == NULL or item->node == NULL)
			return NULL; /* Stop iteration */

		iter->state = item->node;
		const int n = iter->state->n;
		int i;
		for (i=0; i < n; i++) {
			StackItem* new_item = (StackItem*)list_item_new(sizeof(StackItem));
			if (not new_item) {
				PyErr_NoMemory();
				return NULL;
			}

			new_item->node  = iter->state->next[i];
			new_item->depth = item->depth + 1;
			list_push_front(&iter->stack, (ListItem*)new_item);
		}

		if (iter->type != ITER_VALUES) {
			// update keys when needed
			if (UNLIKELY(iter->n < item->depth)) {
				size_t new_size = 256*((item->depth + 255)/256);
				char* new_buf = (char*)memrealloc(iter->buffer, new_size);
				if (new_buf) {
					iter->n = new_size;
					iter->buffer = new_buf;
				}
				else {
					PyErr_NoMemory();
					return NULL;
				}
			}

			iter->buffer[item->depth] = iter->state->byte;
		} // if

		if (iter->state->eow) {
			PyObject* val;

			switch (iter->type) {
				case ITER_KEYS:
					return PyBytes_FromStringAndSize(iter->buffer + 1, item->depth);

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
							return Py_BuildValue("(y#O)",
								/*key*/ iter->buffer + 1, item->depth,
								/*val*/ iter->state->output.object
							);

						case STORE_LENGTH:
						case STORE_INTS:
							return Py_BuildValue("(y#i)",
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
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
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
