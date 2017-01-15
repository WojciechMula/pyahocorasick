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


static PyObject*
automaton_search_iter_new(
	Automaton* automaton,
	PyObject* object,
	int start,
	int end
) {
	AutomatonSearchIter* iter;

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
	iter->index	= start - 1;	// -1 because the first instruction in next() increments index
	iter->end	= end;

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

	while (iter->output && !iter->output->eow) {
		iter->output = iter->output->fail;
	}

	if (iter->output) {
		node = iter->output;
		iter->output = iter->output->fail;
		switch (iter->automaton->store) {
			case STORE_LENGTH:
			case STORE_INTS:
				*result = Py_BuildValue("ii", iter->index + iter->shift, node->output.integer);
				return OutputValue;

			case STORE_ANY:
				*result = Py_BuildValue("iO", iter->index + iter->shift, node->output.object);
				return OutputValue;

			default:
				PyErr_SetString(PyExc_ValueError, "inconsistent internal state!");
				return OutputError;
		}
	}

	return OutputNone;
}


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

	iter->index += 1;
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

		iter->index += 1;

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
	ssize_t len;
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

	if (!reset)
		iter->shift += (iter->index >= 0) ? iter->index : 0;

	iter->index		= -1;
	iter->end		= (int)len;

	if (reset) {
		iter->state  = iter->automaton->root;
		iter->shift  = 0;
		iter->output = NULL;
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
