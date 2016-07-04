/*
	This is part of pyahocorasick Python module.
	
	Automaton class methods

	Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
	License   : 3-clauses BSD (see LICENSE)
*/
#ifndef ahocorasick_Automaton_h_included
#define ahocorasick_Automaton_h_included

#include "common.h"
#include "trie.h"

typedef enum {
	EMPTY,
	TRIE,
	AHOCORASICK
} AutomatonKind;


typedef enum {
	STORE_INTS,
	STORE_LENGTH,
	STORE_ANY
} KeysStore;


typedef struct AutomatonStatistics {
	int			version;

	ssize_t		nodes_count;		///< total number of nodes
	ssize_t		words_count;		///< len(automaton)
	ssize_t		longest_word;		///< longest word
	ssize_t		links_count;		///< links count
	ssize_t		sizeof_node;		///< size of single node (a C structure)
	ssize_t		total_size;			///< total size in bytes
} AutomatonStatistics;


typedef struct Automaton {
	PyObject_HEAD

	AutomatonKind	kind;	///< current kind of automaton
	KeysStore		store;	///< type of values: copy of string, bare integer, python  object
	int				count;	///< number of distinct words
	int				longest_word;	///< length of the longest word
	TrieNode*		root;	///< root of a trie

	int				version;	///< current version of automaton, incremented by add_word, clean and make_automaton; used to lazy invalidate iterators

	AutomatonStatistics	stats;	///< statistics
} Automaton;

/*------------------------------------------------------------------------*/

static bool
automaton_unpickle(
	Automaton* automaton,
	const size_t count,
	uint8_t* data,
	const size_t size,
	PyObject* values
);

/* __init__ */
static PyObject*
automaton_new(PyTypeObject* self, PyObject* args, PyObject* kwargs);

/* clear() */
static PyObject*
automaton_clear(PyObject* self, PyObject* args);

/* len() */
static ssize_t
automaton_len(PyObject* self);

/* add_word */
static PyObject*
automaton_add_word(PyObject* self, PyObject* args);

/* clear() */
static PyObject*
automaton_clear(PyObject* self, PyObject* args);

/* __contains__ */
static int
automaton_contains(PyObject* self, PyObject* args);

/* exists() */
static PyObject*
automaton_exists(PyObject* self, PyObject* args);

/* match() */
static PyObject*
automaton_match(PyObject* self, PyObject* args);

/* get() */
static PyObject*
automaton_get(PyObject* self, PyObject* args);

/* make_automaton() */
static PyObject*
automaton_make_automaton(PyObject* self, PyObject* args);

/* find_all() */
static PyObject*
automaton_find_all(PyObject* self, PyObject* args);

/* keys() */
static PyObject*
automaton_keys(PyObject* self, PyObject* args);

/* values() */
static PyObject*
automaton_values(PyObject* self, PyObject* args);

/* items() */
static PyObject*
automaton_items(PyObject* self, PyObject* args);

/* iter() */
static PyObject*
automaton_iter(PyObject* self, PyObject* args);

/* get_stats() */
static PyObject*
automaton_get_stats(PyObject* self, PyObject* args);

#endif
