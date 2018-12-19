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

static void
trienode_free(TrieNode* node) {

	ASSERT(node);

	if (node->n > 0) {
		memory_free(node->next);
	}

	memory_free(node);
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


static TristateResult
trienode_unset_next_pointer(TrieNode* node, TrieNode* child) {

	unsigned i;
	unsigned index;
	TrieNode** next;

	ASSERT(node);
	for (i=0; i < node->n; i++) {
		if (node->next[i] == child) {
			index = i;
			goto found;
		}
	}

	return FALSE;

found:
	if (node->n == 1) {
		// there is just one node
		node->n = 0;
		memory_free(node->next);
		node->next = NULL;
		return TRUE;
	}

	// there are more nodes, reallocation is needed

	next = (TrieNode**)memory_alloc((node->n - 1) * sizeof(TrieNode*));
	if (UNLIKELY(next == NULL)) {
		return MEMORY_ERROR;
	}

	for (i=0; i < index; i++) {
		next[i] = node->next[i];
	}

	for (i=index + 1; i < node->n; i++) {
		next[i - 1] = node->next[i];
	}

	memory_free(node->next);
	node->next = next;
	node->n -= 1;
	return TRUE;
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


#ifdef DEBUG_LAYOUT
void trienode_dump_layout() {
#define field_size(name) sizeof(((TrieNode*)NULL)->name)
#define field_ofs(name) offsetof(TrieNode, name)
#define field_dump(name) printf("- %-12s: %d %d\n", #name, field_size(name), field_ofs(name));

    puts("TrieNode:");
    field_dump(output);
    field_dump(fail);
    field_dump(n);
    field_dump(eow);
    field_dump(letter);
    field_dump(next);

#undef field_dump
#undef field_size
#undef field_ofs
}
#endif


UNUSED static void
trienode_dump_to_file(TrieNode* node, FILE* f) {

    unsigned i;

    ASSERT(node != NULL);
    ASSERT(f != NULL);

    if (node->n == 0)
        fprintf(f, "leaf ");

    fprintf(f, "node %p\n", node);
    fprintf(f, "- letter %d [%c]\n", node->letter, node->letter);
    if (node->eow)
        fprintf(f, "- eow [%p]\n", node->output.object);

    fprintf(f, "- fail: %p\n", node->fail);
    if (node->n > 0) {
        if (node->next == NULL) {
            fprintf(f, "- %d next: %p\n", node->n, node->next);
        } else {
            fprintf(f, "- %d next: [%p", node->n, node->next[0]);
            for (i=1; i < node->n; i++)
                fprintf(f, ", %p", node->next[i]);
            fprintf(f, "]\n");
        }
    }
}

