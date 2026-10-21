/*
    This is part of pyahocorasick Python module.

    Implementation of pickling/unpickling routines for Automaton class

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
    License   : BSD-3-Clause (see LICENSE)
*/

/* Serialize a private snapshot; never replace live trie failure links. */

#include <string.h>
#include "pickle/pickle_data.c"

static int
pickle_dump_save(const AutomatonSnapshot* snapshot, TrieNode* node, PickleData* data) {

    TrieNode* dump;
    Pair* arr;
    unsigned i;
    size_t size;

    size = PICKLE_TRIENODE_SIZE + node->n * sizeof(Pair);
    if (UNLIKELY(data->top + size > data->size)) {
        if (UNLIKELY(!pickle_data__add_next_buffer(data))) {
            return 0;
        }
    }

    dump = (TrieNode*)(data->data + data->top);

    // we do not save the last pointer in array
    arr = (Pair*)(data->data + data->top + PICKLE_TRIENODE_SIZE);

    // append the python object to the list
    if (node->eow and data->values) {
        if (PyList_Append(data->values, node->output.object) == -1) {
            return 0;
        }
    }

    // save node data
    if (data->values)
        dump->output.integer = 0;
    else
        dump->output.integer = node->output.integer;

    dump->n         = node->n;
    dump->eow       = node->eow;

    dump->fail = (TrieNode*)automaton_snapshot_id(snapshot, node->fail);

    // save array of pointers
    for (i=0; i < node->n; i++) {
        TrieNode* child = trienode_get_ith_unsafe(node, i);
        ASSERT(child);
        arr[i].child  = (TrieNode*)automaton_snapshot_id(snapshot, child);    // save the id of child node
        arr[i].letter = trieletter_get_ith_unsafe(node, i);
    }

    data->top       += size;
    (*data->count)  += 1;
    return 1;
}


static PyObject*
automaton___reduce__(PyObject* self, PyObject* args) {
    const size_t array_size = 16 * 1024 * 1024;
    AutomatonSnapshot snapshot;
    PickleData data;
    PyObject* tuple = NULL;
    size_t i;

    if (!automaton_snapshot((Automaton*)self, &snapshot))
        return NULL;
    pickle_data__init_default(&data);

    if (snapshot.count == 0) {
        tuple = F(Py_BuildValue)("O()", Py_TYPE(self));
        goto done;
    }
    if (!automaton_snapshot_index(&snapshot) ||
        !pickle_data__init(&data, snapshot.store,
                           snapshot.size * PICKLE_TRIENODE_SIZE + snapshot.edges_size,
                           array_size))
        goto done;

    for (i = 0; i < snapshot.size; i++) {
        if (!pickle_dump_save(&snapshot, &snapshot.nodes[i].node, &data))
            goto done;
    }
    if (!pickle_data__shrink_last_buffer(&data))
        goto done;

    tuple = F(Py_BuildValue)(
        "O(OiiiiiO)", Py_TYPE(self), data.bytes_list,
        snapshot.kind, snapshot.store, snapshot.key_type,
        snapshot.count, snapshot.longest_word,
        data.values ? data.values : Py_None
    );

done:
    pickle_data__cleanup(&data);
    automaton_snapshot_clear(&snapshot);
    return tuple;
}


static bool
automaton_unpickle__validate_bytes_list(PyObject* bytes_list, size_t* result) {

    PyObject* bytes;
    Py_ssize_t k;
    Py_ssize_t nodes_count;
    const uint8_t* data;

    size_t count = 0;

    // calculate the total number of nodes (and do validate data at the same time)
    for (k=0; k < PyList_GET_SIZE(bytes_list); k++) {
        bytes = PyList_GET_ITEM(bytes_list, k);
        if (UNLIKELY(!F(PyBytes_CheckExact)(bytes))) {
            PyErr_Format(PyExc_ValueError,
                         "Item #%lu on the bytes list is not a bytes object",
                         k);
            return false;
        }
        if (PyBytes_GET_SIZE(bytes) < (Py_ssize_t)PICKLE_CHUNK_COUNTER_SIZE) {
            PyErr_SetString(PyExc_ValueError, "Data truncated: missing node count");
            return false;
        }

        data = (const uint8_t*)PyBytes_AS_STRING(bytes);

        nodes_count = *((Py_ssize_t*)data);
        if (UNLIKELY(nodes_count <= 0)) {
            PyErr_Format(PyExc_ValueError,
                         "Nodes count for item #%lu on the bytes list is not positive (%lu)",
                         k, nodes_count);
            return false;
        }

        count += nodes_count;
    }

    if (count == 0) {
        PyErr_SetString(PyExc_ValueError, "Expected at least one node");
        return false;
    }
    *result = count;
    return true;
}


static bool
automaton_unpickle(
    Automaton* automaton,
    PyObject* bytes_list,
    PyObject* values
) {
    TrieNode** id2node = NULL;

    TrieNode* node;
    TrieNode* dump;
    Pair* next;
    PyObject* bytes;
    PyObject* value;
    Py_ssize_t nodes_count;
    Py_ssize_t i;

    unsigned id;
    const uint8_t* data;
    const uint8_t* ptr;
    const uint8_t* end;
    unsigned k;
    size_t j;
    unsigned object_idx = 0;
    size_t index;
    size_t count;

    if (!automaton_unpickle__validate_bytes_list(bytes_list, &count)) {
        goto exception;
    }

    id2node = (TrieNode**)memory_alloc((count+1) * sizeof(TrieNode*));
    if (UNLIKELY(id2node == NULL)) {
        goto no_mem;
    }

    // 1. make nodes
    id = 1;
    for (k=0; k < PyList_GET_SIZE(bytes_list); k++) {
        bytes = PyList_GET_ITEM(bytes_list, k);
        data = (const uint8_t*)PyBytes_AS_STRING(bytes);

        nodes_count = *((Py_ssize_t*)data);

        ptr  = data + PICKLE_CHUNK_COUNTER_SIZE;
        end  = ptr + PyBytes_GET_SIZE(bytes) - PICKLE_CHUNK_COUNTER_SIZE;
        for (i=0; i < nodes_count; i++) {
            if (UNLIKELY(ptr + PICKLE_TRIENODE_SIZE > end)) {
                PyErr_Format(PyExc_ValueError,
                             "Data truncated [parsing header of node #%li]: chunk #%di @ offset %ld, expected at least %lu bytes",
                             i, k, ptr - data, PICKLE_TRIENODE_SIZE);
                goto exception;
            }

            dump = (TrieNode*)(ptr);
            node = (TrieNode*)memory_alloc(sizeof(TrieNode));
            if (LIKELY(node != NULL)) {
                node->output    = dump->output;
                node->fail      = dump->fail;
                node->n         = dump->n;
                node->eow       = dump->eow;
                node->next      = NULL;
            }
            else
                goto no_mem;

            ptr += PICKLE_TRIENODE_SIZE;

            id2node[id++] = node;

            if (node->n > 0) {
                if (UNLIKELY(ptr + node->n * sizeof(Pair) > end)) {
                    PyErr_Format(PyExc_ValueError,
                                "Data truncated [parsing children of node #%lu]: "
                                "chunk #%d @ offset %lu, expected at least %ld bytes",
                                 i, k, ptr - data + i, node->n * sizeof(Pair));

                    goto exception;
                }

                node->next = (Pair*)memory_alloc(node->n * sizeof(Pair));
                if (UNLIKELY(node->next == NULL)) {
                    goto no_mem;
                }

                next = (Pair*)(ptr);
                for (j=0; j < node->n; j++) {
                    node->next[j] = next[j];
                }

                ptr += node->n * sizeof(Pair);
            }
        }
    }

    // 2. restore pointers and references to pyobjects
    for (i=1; i < id; i++) {
        node = id2node[i];

        // references
        if (values and node->eow) {
            value = F(PyList_GetItem)(values, object_idx);
            if (value) {
                Py_INCREF(value);
                node->output.object = value;
                object_idx += 1;
            }
            else
                goto exception;
        }

        // pointers
        if (node->fail) {
            index = (size_t)(node->fail);
            if (LIKELY(index < count + 1)) {
                node->fail = id2node[index];
            } else {
                PyErr_Format(PyExc_ValueError,
                             "Node #%lu malformed: the fail link points to node #%lu, while there are %lu nodes",
                             i - 1, index, count);
                goto exception;
            }
        }

        for (j=0; j < node->n; j++) {
            index = (size_t)(node->next[j].child);
            if (LIKELY(index < count + 1)) {
                node->next[j].child = id2node[index];
            } else {
                PyErr_Format(PyExc_ValueError,
                             "Node #%lu malformed: next link #%lu points to node #%lu, while there are %lu nodes",
                             i - 1, j, index, count);
                goto exception;
            }
        }
    }

    automaton->root = id2node[1];

    memory_free(id2node);
    return 1;

no_mem:
    PyErr_NoMemory();
exception:
    // free memory
    if (id2node) {
        for (i=1; i < id; i++) {
            trienode_free(id2node[i]);
        }

        memory_free(id2node);
    }

    // If there is value list and some of its items were already
    // referenced, release them
    if (values) {
        for (i=0; i < object_idx; i++) {
            Py_XDECREF(F(PyList_GetItem)(values, i));
        }
    }

    return 0;
}

