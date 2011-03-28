/*
	This is part of pyahocorasick Python module.
	
	Trie node declarations

	Author    : Wojciech Mu³a, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl/proj/pyahocorasick/
	License   : 3-clauses BSD (see LICENSE)
	Date      : $Date$

	$Id$
*/

#ifndef ahocorasick_trie_h_included
#define ahocorasick_trie_h_included

#include "common.h"
#include "trienode.h"

/* links to children nodes are stored in dynamic table */
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


/* allocate new node */
static TrieNode*
trienode_new(char byte, char eow);

/* returns child node linked by edge labeled with byte */
static TrieNode* PURE ALWAYS_INLINE
trienode_get_next(TrieNode* node, const uint8_t byte);

/* link with child node by edge labeled with byte */
static TrieNode*
trienode_set_next(TrieNode* node, const uint8_t byte, TrieNode* child);

#endif
