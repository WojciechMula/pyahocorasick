/*
	This is part of pyahocorasick Python module.
	
	AutomatonSearchIter implementation

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
    License   : BSD-3-Clause (see LICENSE)
*/

#include "AutomatonSearchIter.h"

static PyTypeObject automaton_search_iter_type;

#define automaton_search_iter_doc \
	"This class is not available directly but instances of AutomatonSearchIter\n" \
	"are returned by the iter() method of an Automaton. This iterator can be\n" \
	"manipulated through its set() method."


#ifdef VARIABLE_LEN_CHARCODES
static int
automaton_search_iter_substring_index(struct Input* input, int position) {

	TRIE_LETTER_TYPE letter;

	int index = 0;
	int i;

	for (i=0; i < position; i++) {

		letter = input->word[index];
		if (UNLIKELY(Py_UNICODE_IS_SURROGATE(letter))) {
			if (UNLIKELY(!Py_UNICODE_IS_HIGH_SURROGATE(letter))) {
				PyErr_Format(PyExc_ValueError,
					"Malformed UCS-2 string: expected a high surrogate at %d, got %04x",
					index, letter);
				return -1;
			}

			index += 1;

			if (index >= input->wordlen) {
				PyErr_Format(PyExc_ValueError,
					"Malformed UCS-2 string: unexpected end of string");
				return -1;
			}

			letter = input->word[index];
			if (UNLIKELY(!Py_UNICODE_IS_LOW_SURROGATE(letter))) {
				PyErr_Format(PyExc_ValueError,
					"Malformed UCS-2 string: expected a low surrogate at %d, got %04x",
					index, letter);
				return -1;
			}

			index += 1;

		} else {
			index += 1;
		}
	}

	return index;
}
#endif // VARIABLE_LEN_CHARCODES


static PyObject*
automaton_search_iter_new(
	Automaton* automaton,
	PyObject* object,
	int start,
	int end
) {
	AutomatonSearchIter* iter;
#ifdef VARIABLE_LEN_CHARCODES
	int tmp;
#endif

	iter = (AutomatonSearchIter*)PyObject_New(AutomatonSearchIter, &automaton_search_iter_type);
	if (iter == NULL)
		return NULL;

	if (!prepare_input((PyObject*)automaton, object, &iter->input)) {
		return NULL;
	}

	iter->automaton = automaton;
	iter->version	= automaton->version;

	iter->state	= automaton->root;
	iter->output= NULL;
	iter->shift	= 0;
#ifdef VARIABLE_LEN_CHARCODES
	if (automaton->key_type == KEY_STRING) {
		tmp = automaton_search_iter_substring_index(&iter->input, start);
		if (tmp >= 0) {
			iter->index	   = tmp - 1;
			iter->position = start - 1;
		} else {
			return NULL;
		}

		tmp = automaton_search_iter_substring_index(&iter->input, end);
		if (tmp >= 0) {
			iter->end = end;
		} else {
			return NULL;
		}

		iter->expected	= pyaho_UCS2_Any;
	} else {
		iter->index	= start - 1;
		iter->end	= end;
	}
#else
	// -1 because the first instruction in next() increments index
	iter->index	= start - 1;
	iter->end	= end;
#endif

	Py_INCREF(iter->automaton);

	return (PyObject*)iter;
}

#define iter ((AutomatonSearchIter*)self)

static void
automaton_search_iter_del(PyObject* self) {
	Py_DECREF(iter->automaton);
	destroy_input(&iter->input);
	PyObject_Del(self);
}


static PyObject*
automaton_search_iter_iter(PyObject* self) {
	Py_INCREF(self);
	return self;
}


enum {
	OutputValue,
	OutputNone,
	OutputError
};


static int
automaton_build_output(PyObject* self, PyObject** result) {
	TrieNode* node;
	int idx = 0;

	while (iter->output && !iter->output->eow) {
		iter->output = iter->output->fail;
	}

	if (iter->output) {
		node = iter->output;
		iter->output = iter->output->fail;

#ifdef VARIABLE_LEN_CHARCODES
		idx = iter->shift;
		if (iter->automaton->key_type == KEY_STRING) {
			idx += iter->position;
		} else {
			idx += iter->index;
		}
#else
		idx = iter->index + iter->shift;
#endif
		switch (iter->automaton->store) {
			case STORE_LENGTH:
			case STORE_INTS:
				*result = Py_BuildValue("ii", idx, node->output.integer);
				return OutputValue;

			case STORE_ANY:
				*result = Py_BuildValue("iO", idx, node->output.object);
				return OutputValue;

			default:
				PyErr_SetString(PyExc_ValueError, "inconsistent internal state!");
				return OutputError;
		}
	}

	return OutputNone;
}



#ifdef VARIABLE_LEN_CHARCODES
static bool
automaton_search_iter_advance_index(PyObject* self) {

	TRIE_LETTER_TYPE letter;

	iter->index += 1;
	if (iter->automaton->key_type == KEY_SEQUENCE) {
		return true;
	}

	letter = iter->input.word[iter->index];
	if (iter->expected == pyaho_UCS2_Any) {
		if (UNLIKELY(Py_UNICODE_IS_SURROGATE(letter))) {
			if (LIKELY(Py_UNICODE_IS_HIGH_SURROGATE(letter))) {
				iter->expected = pyaho_UCS2_LowSurrogate;
			} else {
				PyErr_Format(PyExc_ValueError,
					"Malformed UCS-2 string: expected a high surrogate at %d, got %04x",
					iter->index, letter);
				return false;
			}
		} else {
			iter->position += 1;
		}
	} else {
		assert(iter->expected == pyaho_UCS2_LowSurrogate);
		if (LIKELY(Py_UNICODE_IS_LOW_SURROGATE(letter))) {
			iter->expected  = pyaho_UCS2_Any;
			iter->position += 1;
		} else {
			PyErr_SetString(PyExc_ValueError,
				"Malformed UCS-2 string: expected a low surrogate at %d, got %04x",
				iter->index, letter);
			return false;
		}
	}

	return true;
}
#endif

static PyObject*
automaton_search_iter_next(PyObject* self) {
	PyObject* output;

	if (iter->version != iter->automaton->version) {
		PyErr_SetString(PyExc_ValueError, "underlaying automaton has changed, iterator is not valid anymore");
		return NULL;
	}

return_output:
	switch (automaton_build_output(self, &output)) {
		case OutputValue:
			return output;

		case OutputNone:
			break;

		case OutputError:
			return NULL;
	}

#ifdef VARIABLE_LEN_CHARCODES
	if (!automaton_search_iter_advance_index(self)) {
		return NULL;
	}
#else
	iter->index += 1;
#endif
	while (iter->index < iter->end) {
		// process single char
		iter->state = ahocorasick_next(
						iter->state,
						iter->automaton->root,
						iter->input.word[iter->index]
						);

		ASSERT(iter->state);

		iter->output = iter->state;
		goto return_output;

#ifdef VARIABLE_LEN_CHARCODES
		if (!automaton_search_iter_advance_index(self)) {
			return NULL;
		}
#else
		iter->index += 1;
#endif

	} // while 
	
	return NULL;	// StopIteration
}


#define automaton_search_iter_set_doc \
	"set(string, reset=False)\n\n" \
	"Set a new string to search. When the reset argument is False (default) \n" \
	"then the Aho-Corasick procedure is continued and the internal state of the \n" \
	"Automaton and end index of the string being searched are not reset. This allow \n" \
	"to search for large strings in multiple smaller chunks." 

static PyObject*
automaton_search_iter_set(PyObject* self, PyObject* args) {
	PyObject* object;
	PyObject* flag;
	int position;
	bool reset;
	struct Input new_input;

	// first argument - required string or buffer
	object = PyTuple_GetItem(args, 0);
	if (object) {
		if (!prepare_input((PyObject*)iter->automaton, object, &new_input)) {
			return NULL;
		}
	}
	else
		return NULL;

	// second argument - optional bool
	flag = PyTuple_GetItem(args, 1);
	if (flag) {
		switch (PyObject_IsTrue(flag)) {
			case 0:
				reset = false;
				break;
			case 1:
				reset = true;
				break;
			default:
				return NULL;
		}
	}
	else {
		PyErr_Clear();
		reset = false;
	}

	destroy_input(&iter->input);
	assign_input(&iter->input, &new_input);

	if (!reset) {
		position = iter->index;
#ifdef VARIABLE_LEN_CHARCODES
		if (iter->automaton->key_type == KEY_STRING) {
			position = iter->position;
		}
#endif
		iter->shift += (position >= 0) ? position : 0;
	}

	iter->index		= -1;
	iter->end		= new_input.wordlen;

	if (reset) {
		iter->state  = iter->automaton->root;
		iter->shift  = 0;
		iter->output = NULL;
#ifdef VARIABLE_LEN_CHARCODES
		iter->position = -1;
		iter->expected = pyaho_UCS2_Any;
#endif
	}

	Py_RETURN_NONE;
}


#undef iter

#define method(name, kind) {#name, automaton_search_iter_##name, kind, automaton_search_iter_##name##_doc}

static
PyMethodDef automaton_search_iter_methods[] = {
	method(set, METH_VARARGS),

	{NULL, NULL, 0, NULL}
};
#undef method


static PyTypeObject automaton_search_iter_type = {
	PY_OBJECT_HEAD_INIT
	"ahocorasick.AutomatonSearchIter",			/* tp_name */
	sizeof(AutomatonSearchIter),				/* tp_size */
	0,											/* tp_itemsize? */
	(destructor)automaton_search_iter_del,		/* tp_dealloc */
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
	automaton_search_iter_doc,                  /* tp_doc */
	0,                                          /* tp_traverse */
	0,                                          /* tp_clear */
	0,                                          /* tp_richcompare */
	0,                                          /* tp_weaklistoffset */
	automaton_search_iter_iter,					/* tp_iter */
	automaton_search_iter_next,					/* tp_iternext */
	automaton_search_iter_methods,				/* tp_methods */
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

// vim: noet ts=4
