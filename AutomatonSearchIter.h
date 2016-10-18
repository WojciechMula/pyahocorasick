/*
	This is part of pyahocorasick Python module.
	
	AutomatonSearchIter const, struct & methods declarations.
	This class implements iterator walk over Aho-Corasick
	automaton. Object of this class is returnd by 'iter' method
	of Automaton class.

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
    License   : BSD-3-Clause (see LICENSE)
*/
#ifndef ahocorasick_AutomatonSearchIter_h_included
#define ahocorasick_AutomatonSearchIter_h_included

#include "common.h"
#include "Automaton.h"

typedef struct AutomatonSearchIter {
	PyObject_HEAD

	Automaton*	automaton;
	int			version;	///< automaton version
	struct Input input;		///< input string
	TrieNode*	state;		///< current state of automaton
	TrieNode*	output;		///< current node, i.e. yielded value
	
	int			index;		///< current index in data
	int			shift;		///< shift + index => output index
	int			end;		///< end index
} AutomatonSearchIter;


static PyObject*
automaton_search_iter_new(
	Automaton* automaton,
	PyObject* object,
	int start,
	int end
);

#endif
