#include "automaton_save.h"

#include "../custompickle.h"
#include "../pyhelpers.h"
#include "savebuffer.h"


// --- public -----------------------------------------------------------

static bool
automaton_save_impl(AutomatonSnapshot* snapshot, const char* path, PyObject* serializer);

PyObject*
automaton_save(PyObject* self, PyObject* args) {

    SaveLoadParameters params;
    Automaton* automaton;
    AutomatonSnapshot snapshot;
    int ret;

    automaton = (Automaton*)self;

    if (UNLIKELY(!automaton_save_load_parse_args(automaton->store, args, &params))) {
        return NULL;
    }

    if (!automaton_snapshot(automaton, &snapshot)) {
        Py_DECREF(params.path);
        return NULL;
    }
    ret = automaton_save_impl(&snapshot, PyBytes_AsString(params.path), params.callback);
    Py_DECREF(params.path);
    automaton_snapshot_clear(&snapshot);

    if (LIKELY(ret))
        Py_RETURN_NONE;
    else
        return NULL;
}

// --- private ----------------------------------------------------------

static int
automaton_save_node(SnapshotNode* entry, SaveBuffer* output);

static bool
automaton_save_impl(AutomatonSnapshot* snapshot, const char* path, PyObject* serializer) {

    CustompickleHeader header;
    CustompickleFooter footer;
    SaveBuffer         output;
    int                ret;
    size_t             i;

    ret = savebuffer_init(&output,
                          serializer,
                          snapshot->store,
                          path,
                          SAVEBUFFER_DEFAULT_SIZE);
    if (!ret)
        return false;

    memcpy(header.magick, CUSTOMPICKLE_MAGICK, sizeof(CUSTOMPICKLE_MAGICK));
    header.data.kind = snapshot->kind;
    header.data.store = snapshot->store;
    header.data.key_type = snapshot->key_type;
    header.data.words_count = snapshot->count;
    header.data.longest_word = snapshot->longest_word;

    // 1. save header
    savebuffer_store(&output, (const char*)&header, sizeof(header));

    // 2. save nodes
    for (i = 0; i < snapshot->size; i++) {
        if (!automaton_save_node(&snapshot->nodes[i], &output)) {
            goto done;
        }
    }

    // 3. save footer
    custompickle_initialize_footer(&footer, output.nodes_count);
    savebuffer_store(&output, (const char*)&footer, sizeof(footer));

done:
    savebuffer_finalize(&output);
    return PyErr_Occurred() == NULL;
}


static int
automaton_save_node(SnapshotNode* entry, SaveBuffer* output) {

    TrieNode* node = &entry->node;
    TrieNode* dump;
    PyObject* bytes;

    // 1. save actual address of node
    savebuffer_store_pointer(output, (void*)entry->original);

    // 2. obtain buffer
    dump = (TrieNode*)savebuffer_acquire(output, PICKLE_TRIENODE_SIZE);
    if (PyErr_Occurred())
        return 0;

    if (output->store != STORE_ANY)
        dump->output.integer = node->output.integer;

    dump->n         = node->n;
    dump->eow       = node->eow;
    dump->fail      = node->fail;

    // 3. pickle python value associated with word
    if (node->eow && output->store == STORE_ANY) {
        bytes = F(PyObject_CallFunctionObjArgs)(output->serializer, node->output.object, NULL);
        if (UNLIKELY(bytes == NULL)) {
            return 0;
        }

        if (UNLIKELY(!F(PyBytes_CheckExact)(bytes))) {
            Py_DECREF(bytes);
            PyErr_SetString(PyExc_TypeError, "serializer must return bytes object");
            return 0;
        }

        // store the size of buffer in trie node [which is not saved yet in the file]
        *(size_t*)(&dump->output.integer) = PyBytes_GET_SIZE(bytes);
    } else {
        bytes = NULL;
    }

    // 4. save array of pointers
    if (node->n > 0) {
        savebuffer_store(output, (const char*)node->next, node->n * sizeof(Pair));
    }

    // 5. save pickled data, if any
    if (bytes) {
        if (!PyErr_Occurred())
            savebuffer_store(output, PyBytes_AS_STRING(bytes), PyBytes_GET_SIZE(bytes));
        Py_DECREF(bytes);
    }

    output->nodes_count += 1;

    return PyErr_Occurred() == NULL;
}
