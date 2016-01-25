/*
	This is part of pyahocorasick Python module.

	Python module.

	This file include all code from *.c files.

	Author    : Wojciech Mu�a, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl/proj/pyahocorasick/
	License   : 3-clauses BSD (see LICENSE)
	Date      : $Date$

	$Id$
*/

#include "common.h"
#include "slist.h"
#include "trienode.h"
#include "trie.h"
#include "Automaton.h"
#include "AutomatonSearchIter.h"
#include "AutomatonItemsIter.h"

/* code */
#include "utils.c"
#include "trienode.c"
#include "trie.c"
#include "slist.c"
#include "Automaton.c"
#include "AutomatonItemsIter.c"
#include "AutomatonSearchIter.c"


struct module_state {
    PyObject *error;
};

#ifdef PY3K
#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))
#else
#define GETSTATE(m) (&_state)
static struct module_state _state;
#endif


static PyObject *
error_out(PyObject *m) {
    struct module_state *st = GETSTATE(m);
    PyErr_SetString(st->error, "something bad happened");
    return NULL;
}


static PyMethodDef ahocorasick_module_methods[] = {
    {"error_out", (PyCFunction)error_out, METH_NOARGS, NULL},
    {NULL, NULL}
};


#ifdef PY3K
static int ahocorasick_traverse(PyObject *m, visitproc visit, void *arg) {
    Py_VISIT(GETSTATE(m)->error);
    return 0;
}

static int ahocorasick_clear(PyObject *m) {
    Py_CLEAR(GETSTATE(m)->error);
    return 0;
}


static struct PyModuleDef ahocorasick_module = {
    PyModuleDef_HEAD_INIT,
    "ahocorasick",
    "ahocorasick module",
	-1,
	ahocorasick_module_methods
    NULL,
    ahocorasick_traverse,
    ahocorasick_clear,
    NULL
};

#define INITERROR return NULL

PyObject *
PyInit_ahocorasick(void)

#else
#define INITERROR return

void
initahocorasick(void)
#endif
{
    PyObject* module;

    automaton_as_sequence.sq_length   = automaton_len;
    automaton_as_sequence.sq_contains = automaton_contains;

    automaton_type.tp_as_sequence = &automaton_as_sequence;
#ifdef PY3K
    module = PyModule_Create(&ahocorasick_module);
#else
    module = Py_InitModule("ahocorasick", ahocorasick_module_methods);
#endif

    if (module == NULL)
        INITERROR;
    struct module_state *st = GETSTATE(module);

    st->error = PyErr_NewException("ahocorasick.Error", NULL, NULL);
    if (st->error == NULL) {
        Py_DECREF(module);
        INITERROR;
    }

    PyModule_AddObject(module, "Automaton", (PyObject*)&automaton_type);

#define add_enum_const(name) PyModule_AddIntConstant(module, #name, name)
    add_enum_const(TRIE);
    add_enum_const(AHOCORASICK);
    add_enum_const(EMPTY);

    add_enum_const(STORE_LENGTH);
    add_enum_const(STORE_INTS);
    add_enum_const(STORE_ANY);

    add_enum_const(MATCH_EXACT_LENGTH);
    add_enum_const(MATCH_AT_MOST_PREFIX);
    add_enum_const(MATCH_AT_LEAST_PREFIX);
#undef add_enum_const

#ifdef AHOCORASICK_UNICODE
    PyModule_AddIntConstant(module, "unicode", 1);
#else
    PyModule_AddIntConstant(module, "unicode", 0);
#endif

#ifdef PY3K
    return module;
#endif
}
