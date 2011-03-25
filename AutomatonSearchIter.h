#ifndef ahocorasick_AutomatonSearchIter_h_included
#define ahocorasick_AutomatonSearchIter_h_included

#include "common.h"
#include "Automaton.h"

typedef struct AutomatonSearchIter {
	PyObject_HEAD

	Automaton*	automaton;
	int			version;	///< automaton version
	PyObject*	object;		///< unicode or buffer
	void*		data;		///< Py_UNICODE or char*
	TrieNode*	state;		///< current state of automaton
	TrieNode*	output;		///< current node, i.e. yielded value
	
	int			index;		///< current index
	int			end;		///< end index
	bool		is_unicode;	///< is data unicode or bytes
} AutomatonSearchIter;


static PyObject*
automaton_search_iter_new(
	Automaton* automaton,
	PyObject* object,
	int start,
	int end,
	bool is_unicode
);

#endif
