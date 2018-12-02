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
	unsigned i;

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
			if (LIKELY(child != NULL))
				trienode_set_next(node, letter, child);
			else {
				// Note: in case of memory error, the already allocate nodes
				//       are still reachable from the root and will be free
				//       upon automaton destruction.
				return NULL;
            }
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


static PyObject*
trie_remove_word(Automaton* automaton, const TRIE_LETTER_TYPE* word, const size_t wordlen) {

	PyObject* object;
	TrieNode* node;
	TrieNode** path;
	unsigned i;
	unsigned last_multiway;

	if (automaton->root == NULL) {
		return NULL;
	}

	node = automaton->root;

	for (i=0; i < wordlen; i++) {
		const TRIE_LETTER_TYPE letter = word[i];

		node = trienode_get_next(node, letter);
		if (node == NULL) {
			return NULL;
		}
	}

	if (node->eow != true) {
		return NULL;
	}

	object = node->output.object;

	if (trienode_is_leaf(node)) {
		// Remove dangling path

		// the path includes automaton's root
		path = (TrieNode**)memory_alloc((wordlen + 1) * sizeof(TrieNode*));
		if (UNLIKELY(path == NULL)) {
			return MEMORY_ERROR;
		}

		// 1. Collect all nodes along the path and find the last node which either
		//    has more than one children or it's a terminating node.
		node    = automaton->root;
		path[0] = automaton->root;
		last_multiway = 0;
		for (i=0; i < wordlen; i++) {
			node = trienode_get_next(node, word[i]);
			ASSERT(node);
			path[i + 1] = node;
			if (node->n > 1 || (node->n == 1 && node->eow)) {
				last_multiway = i + 1;
			}
		}

		// the last will node never be a multiway one
		ASSERT(last_multiway < wordlen);

		// Here we know that tail path[last_multiway + 1 .. ] is a linear list.

		// 1. Unlink the tail from the trie
		if (UNLIKELY(trienode_unset_next_pointer(path[last_multiway], path[last_multiway + 1]) == MEMORY_ERROR)) {
			return MEMORY_ERROR;
		}

		// 2. Free the tail (reference to value from the last element was already saved)
		for (i=last_multiway + 1; i < wordlen + 1; i++) {
			trienode_free(path[i]);
		}

		memory_free(path);
	} else {
		// just unmark the terminating node
		node->eow = false;
	}

	automaton->kind = TRIE;
	return object;
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
	unsigned i;

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
