#ifndef ahocorasick_AutomatonItemsIter_h_included
#define ahocorasick_AutomatonItemsIter_h_included

#include "common.h"
#include "Automaton.h"

typedef enum {
	ITER_KEYS,
	ITER_VALUES,
	ITER_ITEMS
} ItemsType;


typedef struct AutomatonItemsIter {
	PyObject_HEAD

	Automaton*	automaton;
	int			version;		///< automaton version
	TrieNode* 	state;			///< current automaton node
	List		stack;			///< stack
	ItemsType	type;			///< type of iterator (KEYS/VALUES/ITEMS)
	
	size_t		n;				///< length of buffer
	char*		buffer;			///< buffer to construct key representation
} AutomatonItemsIter;


static PyObject*
automaton_items_iter_new(Automaton* automaton);

#endif
