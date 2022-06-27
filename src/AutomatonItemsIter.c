/*
    This is part of pyahocorasick Python module.

    AutomatonItemsIter implementation

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
    License   : BSD-3-Clause (see LICENSE)
*/
#include "AutomatonItemsIter.h"

static PyTypeObject automaton_items_iter_type;


typedef struct AutomatonItemsStackItem {
    LISTITEM_data;

    struct TrieNode* node;
    TRIE_LETTER_TYPE letter;
    size_t depth;
    bool escaped;

#ifdef AHOCORASICK_UNICODE
    TRIE_LETTER_TYPE* prefix;   ///< buffer to construct key representation
#else
    char* prefix;
#endif
    size_t prefix_len;

} AutomatonItemsStackItem;

#define StackItem AutomatonItemsStackItem


static StackItem *
create_stack_item(Automaton* automaton) {
    StackItem* new_item = (StackItem*)list_item_new(sizeof(StackItem));
    if (UNLIKELY(new_item == NULL)) {
        goto no_memory;
    }

    new_item->node = automaton->root;
    new_item->depth = 0;
    new_item->escaped = false;
    new_item->prefix = NULL;
    new_item->prefix_len = 0;
    return new_item;

no_memory:
    PyErr_NoMemory();
    return NULL;
}


static void
memory_free_stack_item(StackItem *item) {
    memory_safefree(item->prefix);
    memory_free(item);
}


static void
stack_item_set_prefix(StackItem* item, const void* prefix, size_t prefix_len, const TRIE_LETTER_TYPE* matched_letter) {
    item->prefix = memory_realloc(item->prefix, (prefix_len + 1) * sizeof *(item->prefix));
    if (item->prefix == NULL) {
        goto no_memory;
    }
    memcpy(item->prefix, prefix, prefix_len * sizeof *(item->prefix));
    item->prefix_len = prefix_len;

    if (matched_letter != NULL) {
        item->prefix[item->prefix_len] = *matched_letter;
        item->prefix_len += 1;
    }
    return;

no_memory:
    PyErr_NoMemory();
}


static PyObject*
automaton_items_iter_new(
    Automaton* automaton,
    const TRIE_LETTER_TYPE* word,
    const Py_ssize_t wordlen,
    const PatternMatchType matchtype
) {
    AutomatonItemsIter* iter;
    StackItem* new_item;

    iter = (AutomatonItemsIter*)F(PyObject_New)(AutomatonItemsIter, &automaton_items_iter_type);
    if (iter == NULL)
        return NULL;

    iter->automaton = automaton;
    iter->version   = automaton->version;
    iter->state = NULL;
    iter->type = ITER_KEYS;
    iter->pattern = NULL;
    iter->matchtype = matchtype;
    list_init(&iter->stack);

    Py_INCREF((PyObject*)iter->automaton);

    if (word) {
        iter->pattern = (TRIE_LETTER_TYPE*)memory_alloc(wordlen * TRIE_LETTER_SIZE);
        if (UNLIKELY(iter->pattern == NULL)) {
            goto no_memory;
        }
        else {
            iter->pattern_length = wordlen;
            memcpy(iter->pattern, word, wordlen * TRIE_LETTER_SIZE);
        }
    }
    else
        iter->pattern_length = 0;

    new_item = create_stack_item(automaton);
    if (UNLIKELY(new_item == NULL)) {
        goto no_memory;
    }
    list_push_front(&iter->stack, (ListItem*)new_item);

    return (PyObject*)iter;

no_memory:
    Py_DECREF((PyObject*)iter);
    PyErr_NoMemory();
    return NULL;
}


#define iter ((AutomatonItemsIter*)self)

static void
automaton_items_iter_del(PyObject* self) {
    memory_safefree(iter->pattern);

    while (true) {
        StackItem* item = (StackItem*)list_pop_first(&iter->stack);
        if (item == NULL) {
            break;
        }
        memory_free_stack_item(item);
    }
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
    TrieNode* node;
    TRIE_LETTER_TYPE letter;
    size_t depth;
#ifdef AHOCORASICK_UNICODE
    TRIE_LETTER_TYPE *prefix;
#else
    char *prefix;
#endif
    size_t prefix_len;
    bool escaped;

    if (UNLIKELY(iter->version != iter->automaton->version)) {
        PyErr_SetString(PyExc_ValueError, "The underlying automaton has changed: this iterator is no longer valid.");
        return NULL;
    }

    while (true) {
        StackItem* top = (StackItem*)list_pop_first(&iter->stack);
        if (top == NULL)
            return NULL; /* Stop iteration */

        if (top->node == NULL) {
            memory_free_stack_item(top);
            return NULL; /* Stop iteration */
        }

        node = top->node;
        letter = top->letter;
        depth = top->depth;
        escaped = top->escaped;
        prefix = top->prefix;
        prefix_len = top->prefix_len;

        iter->state  = node;
        iter->letter = letter;

        if ((iter->pattern_length == 0)
            or (depth >= iter->pattern_length and iter->matchtype == MATCH_PREFIX)) {
            // match all
            const int n = iter->state->n;
            int i;
            for (i=0; i < n; i++) {
                StackItem *new_item = create_stack_item(iter->automaton);
                if (UNLIKELY(new_item == NULL)) {
                    memory_free_stack_item(top);
                    PyErr_NoMemory();
                    return NULL;
                }
                new_item->node = trienode_get_ith_unsafe(iter->state, i);
                new_item->letter = trieletter_get_ith_unsafe(iter->state, i);
                new_item->depth = depth + 1;
                new_item->escaped = false;
                if (iter->type != ITER_VALUES) {
                    stack_item_set_prefix(new_item, prefix, prefix_len, &new_item->letter);
                }
                list_push_front(&iter->stack, (ListItem*)new_item);
            }
        }
        else if (depth < iter->pattern_length and iter->pattern[depth] == '\\' and !escaped) {
            // Skip escape character and block next iteration from processing a wildcard
            StackItem *new_item = create_stack_item(iter->automaton);
            if (UNLIKELY(new_item == NULL)) {
                memory_free_stack_item(top);
                PyErr_NoMemory();
                return NULL;
            }

            new_item->node = node;
            new_item->letter = letter;
            new_item->depth = depth + 1;
            new_item->escaped = true;
            if (iter->type != ITER_VALUES) {
                stack_item_set_prefix(new_item, prefix, prefix_len, NULL);
            }
            list_push_front(&iter->stack, (ListItem*)new_item);

        }
        else if (depth < iter->pattern_length and iter->pattern[depth] == '?' and !escaped) {
            // match any single character
            const int n = iter->state->n;
            if (n < 1) {
                // no match
                memory_free_stack_item(top);
                continue;
            }

            int i;
            for (i=0; i < n; i++) {
                StackItem *new_item = create_stack_item(iter->automaton);
                if (UNLIKELY(new_item == NULL)) {
                    memory_free_stack_item(top);
                    PyErr_NoMemory();
                    return NULL;
                }

                new_item->node = trienode_get_ith_unsafe(iter->state, i);
                new_item->letter = trieletter_get_ith_unsafe(iter->state, i);
                new_item->depth = depth + 1;
                new_item->escaped = false;
                if (iter->type != ITER_VALUES) {
                    stack_item_set_prefix(new_item, prefix, prefix_len, &new_item->letter);
                }
                list_push_front(&iter->stack, (ListItem*)new_item);
            }
        }
        else if (depth < iter->pattern_length and iter->pattern[depth] == '*' and !escaped) {
            // match zero children and skip wildcard
            StackItem *new_item = create_stack_item(iter->automaton);
            if (UNLIKELY(new_item == NULL)) {
                memory_free_stack_item(top);
                PyErr_NoMemory();
                return NULL;
            }

            new_item->node = node;
            new_item->letter = letter;
            new_item->depth = depth + 1;
            new_item->escaped = false;
            if (iter->type != ITER_VALUES) {
                stack_item_set_prefix(new_item, prefix, prefix_len, NULL);
            }
            list_push_front(&iter->stack, (ListItem*)new_item);

            // match all children and retain wildcard
            const int n = iter->state->n;
            int i;
            for (i=0; i < n; i++) {
                StackItem *new_item = create_stack_item(iter->automaton);
                if (UNLIKELY(new_item == NULL)) {
                    memory_free_stack_item(top);
                    PyErr_NoMemory();
                    return NULL;
                }

                new_item->node = trienode_get_ith_unsafe(iter->state, i);
                new_item->letter = trieletter_get_ith_unsafe(iter->state, i);
                new_item->depth = depth;
                new_item->escaped = false;
                if (iter->type != ITER_VALUES) {
                    stack_item_set_prefix(new_item, prefix, prefix_len, &new_item->letter);
                }
                list_push_front(&iter->stack, (ListItem*)new_item);
            }
        }
        else if (depth < iter->pattern_length) {
            // match single letter
            TrieNode* node = trienode_get_next(iter->state, iter->pattern[depth]);

            if (node) {
                StackItem *new_item = create_stack_item(iter->automaton);
                if (UNLIKELY(new_item == NULL)) {
                    memory_free_stack_item(top);
                    PyErr_NoMemory();
                    return NULL;
                }

                new_item->node = node;
                new_item->letter = iter->pattern[depth];
                new_item->depth = depth + 1;
                new_item->escaped = false;
                if (iter->type != ITER_VALUES) {
                    stack_item_set_prefix(new_item, prefix, prefix_len, &new_item->letter);
                }
                list_push_front(&iter->stack, (ListItem*)new_item);
            }
        }

        if (depth >= iter->pattern_length and iter->state->eow) {
            PyObject* result;
            PyObject* val;

            switch (iter->type) {
                case ITER_KEYS:
#if defined PEP393_UNICODE
                    result = F(PyUnicode_FromKindAndData)(PyUnicode_4BYTE_KIND, (void*)(prefix), prefix_len);
                    memory_free_stack_item(top);
                    return result;
#elif defined AHOCORASICK_UNICODE
                    result = PyUnicode_FromUnicode((Py_UNICODE*)(prefix), prefix_len);
                    memory_free_stack_item(top);
                    return result;
#else
                    result = PyBytes_FromStringAndSize(prefix, prefix_len);
                    memory_free_stack_item(top);
                    return result;
#endif

                case ITER_VALUES:
                    memory_free_stack_item(top);
                    switch (iter->automaton->store) {
                        case STORE_ANY:
                            val = iter->state->output.object;
                            Py_INCREF(val);
                            break;

                        case STORE_LENGTH:
                        case STORE_INTS:
                            return F(Py_BuildValue)("i", iter->state->output.integer);

                        default:
                            PyErr_SetString(PyExc_SystemError, "Incorrect 'store' attribute.");
                            return NULL;
                    }

                    return val;

                case ITER_ITEMS:
                    switch (iter->automaton->store) {
                        case STORE_ANY:
                            result = F(Py_BuildValue)(
#ifdef PY3K
    #ifdef AHOCORASICK_UNICODE
                                "(u#O)", /*key*/ prefix, prefix_len,
    #else
                                "(y#O)", /*key*/ prefix, prefix_len,
    #endif
#else
                                "(s#O)", /*key*/ prefix, prefix_len,
#endif
                                /*val*/ iter->state->output.object
                            );
                            memory_free_stack_item(top);
                            return result;

                        case STORE_LENGTH:
                        case STORE_INTS:
                            result = F(Py_BuildValue)(
#ifdef PY3K
    #ifdef AHOCORASICK_UNICODE
                                "(u#i)", /*key*/ prefix, prefix_len,
    #else
                                "(y#i)", /*key*/ prefix, prefix_len,
    #endif
#else
                                "(s#i)", /*key*/ prefix, prefix_len,
#endif
                                /*val*/ iter->state->output.integer
                            );
                            memory_free_stack_item(top);
                            return result;

                        default:
                            PyErr_SetString(PyExc_SystemError, "Incorrect 'store' attribute.");
                            memory_free_stack_item(top);
                            return NULL;
                    } // switch
            }
        }

        // done with top for this iteration, free it
        memory_free_stack_item(top);
    }
}

#undef StackItem
#undef iter

static PyTypeObject automaton_items_iter_type = {
    PY_OBJECT_HEAD_INIT
    "AutomatonItemsIter",                       /* tp_name */
    sizeof(AutomatonItemsIter),                 /* tp_size */
    0,                                          /* tp_itemsize? */
    (destructor)automaton_items_iter_del,       /* tp_dealloc */
    0,                                          /* tp_print */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_reserved */
    0,                                          /* tp_repr */
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
    automaton_items_iter_iter,                  /* tp_iter */
    automaton_items_iter_next,                  /* tp_iternext */
    0,                                          /* tp_methods */
    0,                                          /* tp_members */
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
