/*
	This is part of pyahocorasick Python module.
	
	Header for of automaton methods:

	Author    : Wojciech Mu³a, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl/proj/pyahocorasick/
	License   : 3-clauses BSD (see LICENSE)
	Date      : $Date$

	$Id$

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
	int		version;

	int		nodes_count;		///< total number of nodes
	int		words_count;		///< len(automaton)
	int		longest_word;		///< longest word
	int		links_count;		///< links count
	int		sizeof_node;		///< size of single node (a C structure)
	int		total_size;			///< total size in bytes
} AutomatonStatistics;


typedef struct Automaton {
	PyObject_HEAD

	AutomatonKind	kind;	///< current kind of automaton
	KeysStore		store;	///< type of values: copy of string, bare integer, python  object
	int				count;	///< number of distinct words
	TrieNode*		root;	///< root of a trie

	int				version;	///< current version of automaton, incremented by add_word, clean and make_automaton; used to lazy invalidate iterators

	AutomatonStatistics	stats;	///< statistics
} Automaton;

#endif
