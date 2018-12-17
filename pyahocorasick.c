/*
	This is part of pyahocorasick Python module.
	
	Python module.

	This file include all code from *.c files.

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
    License   : BSD-3-Clause (see LICENSE)
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
#include "src/pickle/streamnodes_iter.c"
#include "src/pickle/pickle_proxy.c"
#include "Automaton.c"
#include "AutomatonItemsIter.c"
#include "AutomatonSearchIter.c"
#ifdef PYCALLS_INJECT_FAULTS
#include "src/pycallfault/pycallfault.c"
#endif

#define ahocorasick_doc \
    "pyahocorasick is a fast and memory efficient library for exact or approximate\n" \
	"multi-pattern string search meaning that you can find multiple key strings\n" \
	"occurrences at once in some input text."


static
PyMethodDef
ahocorasick_module_methods[] = {
	{NULL, NULL, 0, NULL}
};


#ifdef PY3K
static
PyModuleDef ahocorasick_module = {
	PyModuleDef_HEAD_INIT,
	"ahocorasick",
	"ahocorasick module",
	-1,
	ahocorasick_module_methods
};
#endif

#ifdef PY3K
#define init_function PyInit_ahocorasick
#define init_return(value) return (value)
#else
#define init_function initahocorasick
#define init_return(unused) return
#endif

PyMODINIT_FUNC
init_function(void) {
	PyObject* module;

#ifdef MEMORY_DEBUG
    PyErr_WarnEx(PyExc_RuntimeWarning,
                 "This is a developer version of pyahcorosick. "
                 "The module was compiled with flag MEMORY_DEBUG.", 1);
    initialize_memory_debug();
#endif

#ifdef PYCALLS_INJECT_FAULTS
    PyErr_WarnEx(PyExc_RuntimeWarning,
                 "This is a developer version of pyahcorosick. "
                 "The module was compiled with flag PYCALLS_INJECT_FAULTS.", 1);
    initialize_pycallfault();
#endif

#if DEBUG_LAYOUT
    PyErr_WarnEx(PyExc_RuntimeWarning,
                 "This is a developer version of pyahcorosick. "
                 "The module was compiled with flag DEBUG_LAYOUT.", 1);
    trienode_dump_layout();
#endif

	automaton_as_sequence.sq_length   = automaton_len;
	automaton_as_sequence.sq_contains = automaton_contains;

	automaton_type.tp_as_sequence = &automaton_as_sequence;

#ifdef PY3K
	module = PyModule_Create(&ahocorasick_module);
#else
    module = Py_InitModule3("ahocorasick", ahocorasick_module_methods, ahocorasick_doc);
#endif
	if (module == NULL)
		init_return(NULL);


	if (PyType_Ready(&automaton_type) < 0) {
		Py_DECREF(module);
		init_return(NULL);
	}
	else
		PyModule_AddObject(module, "Automaton", (PyObject*)&automaton_type);


	if (PyType_Ready(&pickle_proxy_type) < 0) {
		Py_DECREF(module);
		init_return(NULL);
	}
	else
		PyModule_AddObject(module, "_PickleProxy", (PyObject*)&pickle_proxy_type);

#define add_enum_const(name) PyModule_AddIntConstant(module, #name, name)
	add_enum_const(TRIE);
	add_enum_const(AHOCORASICK);
	add_enum_const(EMPTY);

	add_enum_const(STORE_LENGTH);
	add_enum_const(STORE_INTS);
	add_enum_const(STORE_ANY);

	add_enum_const(KEY_STRING);
	add_enum_const(KEY_SEQUENCE);

	add_enum_const(MATCH_EXACT_LENGTH);
	add_enum_const(MATCH_AT_MOST_PREFIX);
	add_enum_const(MATCH_AT_LEAST_PREFIX);
#undef add_enum_const

#ifdef AHOCORASICK_UNICODE
	PyModule_AddIntConstant(module, "unicode", 1);
#else
	PyModule_AddIntConstant(module, "unicode", 0);
#endif

	init_return(module);
}
