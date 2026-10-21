/*
 * Capture the trie without calling Python code while its lock is held.
 * Serialization and dump() can then allocate containers or call user code
 * without retaining pointers into a concurrently mutable automaton.
 */

typedef struct SnapshotNode {
    const TrieNode* original;
    TrieNode node;
} SnapshotNode;

typedef struct SnapshotIndex {
    const TrieNode* original;
    size_t id;
} SnapshotIndex;

typedef struct AutomatonSnapshot {
    AutomatonKind kind;
    KeysStore store;
    KeyType key_type;
    int count;
    int longest_word;
    SnapshotNode* nodes;
    size_t size;
    size_t capacity;
    Pair* edges;
    size_t edges_size;
    SnapshotIndex* index;
} AutomatonSnapshot;

static void
automaton_snapshot_clear(AutomatonSnapshot* snapshot) {
    size_t i;
    for (i = 0; i < snapshot->size; i++) {
        TrieNode* node = &snapshot->nodes[i].node;
        if (snapshot->store == STORE_ANY && node->eow)
            Py_XDECREF(node->output.object);
    }
    memory_safefree(snapshot->edges);
    memory_safefree(snapshot->nodes);
    memory_safefree(snapshot->index);
}

static int
automaton_snapshot_node(TrieNode* node, const int depth, void* extra) {
    AutomatonSnapshot* snapshot = (AutomatonSnapshot*)extra;
    SnapshotNode* copy;

    if (snapshot->size == snapshot->capacity) {
        size_t capacity = snapshot->capacity ? snapshot->capacity * 2 : 64;
        SnapshotNode* nodes = memory_realloc(snapshot->nodes, capacity * sizeof(SnapshotNode));
        if (nodes == NULL)
            return 0;
        snapshot->nodes = nodes;
        snapshot->capacity = capacity;
    }
    copy = &snapshot->nodes[snapshot->size++];
    copy->original = node;
    copy->node = *node;
    /* Mutating a compiled trie invalidates its failure links. */
    if (snapshot->kind != AHOCORASICK)
        copy->node.fail = NULL;
    if (snapshot->store == STORE_ANY && node->eow)
        Py_XINCREF(node->output.object);
    snapshot->edges_size += node->n * sizeof(Pair);
    return 1;
}

static bool
automaton_snapshot(Automaton* automaton, AutomatonSnapshot* snapshot) {
    bool success = true;
    size_t i;
    Pair* next;
    memset(snapshot, 0, sizeof(*snapshot));
    Py_BEGIN_CRITICAL_SECTION(automaton);
    snapshot->kind = automaton->kind;
    snapshot->store = automaton->store;
    snapshot->key_type = automaton->key_type;
    snapshot->count = automaton->count;
    snapshot->longest_word = automaton->longest_word;
    if (automaton->root)
        success = trie_traverse(automaton->root, automaton_snapshot_node, snapshot);
    /* Copy edges in one allocation while the borrowed arrays are still locked. */
    if (success && snapshot->edges_size) {
        snapshot->edges = memory_alloc(snapshot->edges_size);
        success = snapshot->edges != NULL;
        if (success) {
            next = snapshot->edges;
            for (i = 0; i < snapshot->size; i++) {
                TrieNode* node = &snapshot->nodes[i].node;
                if (node->n) {
                    memcpy(next, node->next, node->n * sizeof(Pair));
                    node->next = next;
                    next += node->n;
                }
            }
        }
    }
    Py_END_CRITICAL_SECTION();
    if (!success) {
        automaton_snapshot_clear(snapshot);
        PyErr_NoMemory();
        return false;
    }
    return true;
}

static int
snapshot_index_compare(const void* a, const void* b) {
    uintptr_t left = (uintptr_t)((const SnapshotIndex*)a)->original;
    uintptr_t right = (uintptr_t)((const SnapshotIndex*)b)->original;
    return (left > right) - (left < right);
}

static bool
automaton_snapshot_index(AutomatonSnapshot* snapshot) {
    size_t i;
    snapshot->index = memory_alloc(snapshot->size * sizeof(SnapshotIndex));
    if (snapshot->index == NULL) {
        PyErr_NoMemory();
        return false;
    }
    for (i = 0; i < snapshot->size; i++) {
        snapshot->index[i].original = snapshot->nodes[i].original;
        snapshot->index[i].id = i + 1;
    }
    qsort(snapshot->index, snapshot->size, sizeof(SnapshotIndex), snapshot_index_compare);
    return true;
}

static size_t
automaton_snapshot_id(const AutomatonSnapshot* snapshot, const TrieNode* original) {
    SnapshotIndex key;
    SnapshotIndex* found;
    if (original == NULL)
        return 0;
    key.original = original;
    found = bsearch(&key, snapshot->index, snapshot->size,
                    sizeof(SnapshotIndex), snapshot_index_compare);
    ASSERT(found != NULL);
    return found->id;
}
