/*
	Procedures that work with trie.

*/

/*
Note on trie implementation
===========================

Each nodes remembers edge label, thus making loops on self is
not possible. But such loops appear only at root node, so this
case is resolved manually in Automaton.c:make_automaton function.

*/

static TrieNode*
trienode_new(char byte, char eow) {
	TrieNode* node = (TrieNode*)memalloc(sizeof(TrieNode));
	if (node) {
		node->output	= NULL;
		node->fail		= NULL;

		node->n		= 0;
		node->byte	= byte;
		node->eow	= eow;
		node->next	= NULL;
	}

	return node;
}


static TrieNode* //ALWAYS_INLINE
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


static TrieNode*
trie_add_word(Automaton* automaton, char* word, size_t wordlen, bool* new_word) {
	if (automaton->kind == EMPTY) {
		ASSERT(automaton->root == NULL);
		automaton->root = trienode_new('\0', false);
	}

	TrieNode* node = automaton->root;
	TrieNode* child;

	int i;
	for (i=0; i < wordlen; i++) {
		child = trienode_get_next(node, word[i]);
		if (child == NULL) {
			child = trienode_new(word[i], false);
			if (child)
				trienode_set_next(node, word[i], child);
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


static TrieNode*
trie_find(TrieNode* root, const char* word, const size_t wordlen) {
	TrieNode* node;

	node = root;
	ssize_t i;
	for (i=0; i < wordlen; i++) {
		node = trienode_get_next(node, word[i]);
		if (node == NULL)
			return NULL;
	}
		
	return node;
}


static TrieNode*
ahocorasick_next(TrieNode* node, const uint8_t byte) {
	TrieNode* next = node;
	TrieNode* tmp;

	do {
		tmp = trienode_get_next(next, byte);
		if (tmp)
			return tmp;
		else
			next = next->fail;
	} while (true);
}


