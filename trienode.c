/*
	This is part of pyahocorasick Python module.

	Trie implementation

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
    License   : BSD-3-Clause (see LICENSE)
*/

#include "trienode.h"

static TrieNode*
trienode_new(const TRIE_LETTER_TYPE letter, const char eow) {
	TrieNode* node = (TrieNode*)memory_alloc(sizeof(TrieNode));
	if (node) {
		node->output.integer = 0;
		node->output.object = NULL;
		node->fail		= NULL;

		node->n		= 0;
		node->letter	= letter;
		node->eow	    = eow;
		node->next	= NULL;
	}

	return node;
}


static TrieNode* PURE
trienode_get_next(TrieNode* node, const TRIE_LETTER_TYPE letter) {

	unsigned i;

	ASSERT(node);
	for (i=0; i < node->n; i++)
		if ((node)->next[i]->letter == letter)
			return node->next[i];

	return NULL;
}


static TrieNode* PURE
trienode_get_ith_unsafe(TrieNode* node, size_t index) {
    ASSERT(node);

    return node->next[index];
}


static TrieNode*
trienode_set_next(TrieNode* node, const TRIE_LETTER_TYPE letter, TrieNode* child) {

	int n;
	TrieNode** next;

	ASSERT(node);
	ASSERT(child);
	ASSERT(trienode_get_next(node, letter) == NULL);

	n = node->n;
	next = (TrieNode**)memrealloc(node->next, (n + 1) * sizeof(TrieNode*));
	if (next) {
		node->next = next;
		node->next[n] = child;
		node->n += 1;

		return child;
	}
	else
		return NULL;
}


