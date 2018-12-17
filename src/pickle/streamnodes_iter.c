#include "streamnodes_iter.h"
#include "pickle.h"

static PyTypeObject streamnodes_iter_type;

#define NEW_NODE (-1)
#define STOP_ITERATION (-1)

static PyObject*
streamnodes_iter_new(Automaton* automaton) {
    ASSERT(automaton != NULL);

    StreamNodesIter* iter;

    iter = F(PyObject_New)(StreamNodesIter, &streamnodes_iter_type);
    if (UNLIKELY(iter == NULL)) {
        return NULL;
    }

    iter->automaton = automaton;
    Py_INCREF(automaton);

    iter->stack = (StackItem*)memory_alloc(sizeof(StackItem) * (automaton->longest_word + 1));
    if (UNLIKELY(iter->stack == NULL)) {
        PyErr_NoMemory();
        return NULL;
    }

    iter->stack_top = 0;
    iter->stack[0].node  = automaton->root;
    iter->stack[0].index = NEW_NODE;

    return (PyObject*)iter;
}

// --------------------------------------------------

static void
streamnodes_iter_del(PyObject* object) {
    StreamNodesIter* iter;

    iter = (StreamNodesIter*)object;

    Py_DECREF(iter->automaton);
    xfree(iter->stack);
}

// --------------------------------------------------

static PyObject*
streamnodes_iter_iter(PyObject* iter) {
    Py_INCREF(iter);
    return iter;
}

// --------------------------------------------------

PURE ALWAYS_INLINE static size_t
picklednode_size(TrieNode* node) {
    ASSERT(node != NULL);

    return PICKLE_POINTER_SIZE + PICKLE_TRIENODE_SIZE + node->n * PICKLE_POINTER_SIZE;
}


static PyObject*
streamnodes_serialize_node(TrieNode* node) {
    ASSERT(node != NULL);

    PyObject*  bytes;
    TrieNode** node_ptr;
    TrieNode*  dump;
    TrieNode** arr;
    size_t i;

    bytes = F(PyBytes_FromStringAndSize)(NULL, picklednode_size(node));
    if (UNLIKELY(bytes == NULL)) {
        return NULL;
    }

    node_ptr  = (TrieNode**)(PyBytes_AS_STRING(bytes));
    *node_ptr = node;

    dump = (TrieNode*)(PyBytes_AS_STRING(bytes) + PICKLE_POINTER_SIZE);

    // we do not save the last pointer in array
    arr = (TrieNode**)(PyBytes_AS_STRING(bytes) + PICKLE_POINTER_SIZE + PICKLE_TRIENODE_SIZE);

    dump->output.integer = node->output.integer;
    dump->n         = node->n;
    dump->eow       = node->eow;
    dump->letter    = node->letter;
    dump->fail      = node->fail;

    // save array of pointers
    for (i=0; i < node->n; i++) {
        TrieNode* child = node->next[i];
        ASSERT(child);
        arr[i] = child;
    }

    return bytes;
}


static PyObject*
streamnodes_iter_next(PyObject* object) {
    StreamNodesIter* iter;
    PyObject* bytes;

    StackItem* top;
    StackItem* next;

    iter = (StreamNodesIter*)object;

    if (iter->stack_top == STOP_ITERATION) {
        return NULL; // stop iteration
    }

    while (true) {
        top = &iter->stack[iter->stack_top];

        if (top->index == NEW_NODE) {
            bytes = streamnodes_serialize_node(top->node);
            if (UNLIKELY(bytes == NULL)) {
                return NULL;
            }

            top->index += 1;

            if (iter->automaton->store == STORE_ANY && top->node->eow != 0) {
                return F(Py_BuildValue)("OO", bytes, top->node->output.object);
            } else {
                return bytes;
            }
        } else {

            if (top->index < top->node->n) {
                // go deeper
                next = &iter->stack[iter->stack_top + 1];

                next->node  = top->node->next[top->index];
                next->index = NEW_NODE;

                top->index += 1;
                iter->stack_top += 1;
                continue;

            } else {

                iter->stack_top -= 1;
                if (UNLIKELY(iter->stack_top == -1)) {
                    // we visited all nodes, emit an end-of-stream mark -- the None value
                    iter->stack_top = STOP_ITERATION;
                    Py_RETURN_NONE;
                } else {
                    continue;
                }
            }
        }
    } // while
}

// --------------------------------------------------

static PyTypeObject streamnodes_iter_type = {
    PY_OBJECT_HEAD_INIT
    "StreamNodesIter",                          /* tp_name */
    sizeof(StreamNodesIter),                    /* tp_size */
    0,                                          /* tp_itemsize? */
    (destructor)streamnodes_iter_del,           /* tp_dealloc */
    0,                                          /* tp_print */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_reserved */
    0,                                          /* tp_repr */
    0,                                          /* tp_as_number */
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash */
    0,                                          /* tp_call */
    0,                                          /* tp_str */
    PyObject_GenericGetAttr,                    /* tp_getattro */
    0,                                          /* tp_setattro */
    0,                                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                         /* tp_flags */
    0,                                          /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    streamnodes_iter_iter,                      /* tp_iter */
    streamnodes_iter_next,                      /* tp_iternext */
    0,                                          /* tp_methods */
    0,                                          /* tp_members */
    0,                                          /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    0,                                          /* tp_init */
    0,                                          /* tp_alloc */
    0,                                          /* tp_new */
};
