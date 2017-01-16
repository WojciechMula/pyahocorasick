/*
	This is part of pyahocorasick Python module.
	
	Trie node declarations

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
    License   : BSD-3-Clause (see LICENSE)
*/

#ifndef ahocorasick_trienode_h_included
#define ahocorasick_trienode_h_included

#include "common.h"

/* links to children nodes are stored in dynamic table */
typedef struct TrieNode {
	union {
		PyObject*	object;		///< valid when kind = STORE_ANY
		Py_uintptr_t integer;	///< valid when kind in [STORE_LENGTH, STORE_INTS]
	} output; ///< output function, valid when eow is true
	struct TrieNode*	fail;	///< fail node

#if TRIE_LETTER_SIZE == 1
	uint16_t			n;		///< length of next
#else
	uint32_t			n;		///< length of next
#endif
	uint8_t  			eow:1;	    ///< end of word marker
	TRIE_LETTER_TYPE	letter;	///< incoming edge label

	struct TrieNode**	next;	///< table of pointers
} TrieNode;


/* allocate new node */
static TrieNode*
trienode_new(const TRIE_LETTER_TYPE letter, const char eow);

/* returns child node linked by edge labeled with letter */
static TrieNode* PURE
trienode_get_next(TrieNode* node, const TRIE_LETTER_TYPE letter);

/* link with child node by edge labeled with letter */
static TrieNode*
trienode_set_next(TrieNode* node, const TRIE_LETTER_TYPE letter, TrieNode* child);

static TrieNode* PURE
trienode_get_ith_unsafe(TrieNode* node, size_t letter);

#endif
