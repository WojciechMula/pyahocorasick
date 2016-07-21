/*
	This is part of pyahocorasick Python module.

	Trie implementation

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
    License   : BSD-3-Clause (see LICENSE)
*/

#include "trie.h"


static TrieNode*
trie_add_word(Automaton* automaton, const TRIE_LETTER_TYPE* word, const size_t wordlen, bool* new_word) {

	TrieNode* node;
	TrieNode* child;
	int i;

	if (automaton->kind == EMPTY) {
		ASSERT(automaton->root == NULL);
		automaton->root = trienode_new('\0', false);
		if (automaton->root == NULL)
			return NULL;
	}

	node = automaton->root;

	for (i=0; i < wordlen; i++) {
		const TRIE_LETTER_TYPE letter = word[i];

		child = trienode_get_next(node, letter);
		if (child == NULL) {
			child = trienode_new(letter, false);
			if (child)
				trienode_set_next(node, letter, child);
			else
				return NULL;
		}

		node = child;
	}

	if (node->eow == false) {
		node->eow = true;
		*new_word = true;
		automaton->count += 1;
	}
	else
		*new_word = false;

	automaton->kind = TRIE;

	return node;
}


static TrieNode* PURE
trie_find(TrieNode* root, const TRIE_LETTER_TYPE* word, const size_t wordlen) {
	TrieNode* node;
	size_t i;

	node = root;

	if (node != NULL) {
		for (i=0; i < wordlen; i++) {
			node = trienode_get_next(node, word[i]);
			if (node == NULL)
				return NULL;
		}
	}

	return node;
}


static int PURE
trie_longest(TrieNode* root, const TRIE_LETTER_TYPE* word, const size_t wordlen) {
	TrieNode* node;
	int len = 0;
	size_t i;

	node = root;
	for (i=0; i < wordlen; i++) {
		node = trienode_get_next(node, word[i]);
		if (node == NULL)
			break;
		else
			len += 1;
	}

	return len;
}


static TrieNode* PURE
ahocorasick_next(TrieNode* node, TrieNode* root, const TRIE_LETTER_TYPE letter) {
	TrieNode* next = node;
	TrieNode* tmp;

	while (next) {
		tmp = trienode_get_next(next, letter);
		if (tmp)
			// found link
			return tmp;
		else
			// or go back through fail edges
			next = next->fail;
	}

	// or return root node
	return root;
}

static int
trie_traverse_aux(
	TrieNode* node,
	const int depth,
	trie_traverse_callback callback,
	void *extra
) {
	int i;

	if (callback(node, depth, extra) == 0)
		return 0;

	for (i=0; i < node->n; i++) {
		TrieNode* child = node->next[i];
		ASSERT(child);
		if (trie_traverse_aux(child, depth + 1, callback, extra) == 0)
			return 0;
	}

	return 1;
}


static void
trie_traverse(
	TrieNode* root,
	trie_traverse_callback callback,
	void *extra
) {
	ASSERT(root);
	ASSERT(callback);
	trie_traverse_aux(root, 0, callback, extra);
}


size_t PURE
trienode_get_size(const TrieNode* node) {
	return sizeof(TrieNode) + node->n * sizeof(TrieNode*);
}
