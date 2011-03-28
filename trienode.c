/*
	This is part of pyahocorasick Python module.
	
	Trie implementation

	Author    : Wojciech Mu³a, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl/proj/pyahocorasick/
	License   : 3-clauses BSD (see LICENSE)
	Date      : $Date$

	$Id$
*/

#include "trie.h"

static TrieNode*
trienode_new(char byte, char eow) {
	TrieNode* node = (TrieNode*)memalloc(sizeof(TrieNode));
	if (node) {
		node->output.integer = 0;
		node->fail		= NULL;

		node->n		= 0;
		node->byte	= byte;
		node->eow	= eow;
		node->next	= NULL;
	}

	return node;
}


static TrieNode* PURE ALWAYS_INLINE
trienode_get_next(TrieNode* node, const uint8_t byte) {
	ASSERT(node);
	if (node->n == 256)
		return node->next[byte];
	else {
		int i;
		for (i=0; i < node->n; i++)
			if ((node)->next[i]->byte == byte)
				return node->next[i];

		return NULL;
	}
}

int
trienode_sort_cmp(const void* a, const void* b) {
#define A ((TrieNode*)a)
#define B ((TrieNode*)b)
	return (A->byte > B->byte) - (A->byte < B->byte);
#undef A
#undef B
}


static TrieNode*
trienode_set_next(TrieNode* node, const uint8_t byte, TrieNode* child) {
	ASSERT(node);
	ASSERT(child);
	ASSERT(trienode_get_next(node, byte) == NULL);

	const int n = node->n;
	TrieNode** next = (TrieNode**)memrealloc(node->next, (n + 1) * sizeof(TrieNode*));
	if (next) {
		node->next = next;
		node->next[n] = child;
		node->n += 1;

		if (node->n == 256) {
			qsort(node->next, 256, sizeof(TrieNode*), trienode_sort_cmp);
		}
		
		return child;
	}
	else
		return NULL;
}


