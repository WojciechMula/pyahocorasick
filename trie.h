#ifndef ahocorasick_trie_h_included
#define ahocorasick_trie_h_included

#include "common.h"

typedef struct TrieNode {
	union {
		PyObject*	object;		///< valid when kind = STORE_ANY
		int			integer;	///< valid when kind in [STORE_LENGTH, STORE_INTS]
	} output; ///< output function, valid when eow is true

	struct TrieNode*	fail;	///< fail node

	uint16_t			n;		///< length of next
	uint8_t				byte;	///< incoming edge label
	uint8_t				eow;	///< end of word marker
	struct TrieNode**	next;	///< table of pointers
} TrieNode;


#endif
