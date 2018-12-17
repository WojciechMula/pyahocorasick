#include "pickle_proxy.h"

#include "pickle.h"
#include "streamnodes_iter.h"


#define DEFAULT_ADDRESS_LOOKUP_SIZE (1024u) // items


static PyTypeObject pickle_proxy_type;

#define PickleProxy_Check(ob) (Py_TYPE(ob) == &pickle_proxy_type)

static PyObject*
pickle_proxy_new(PyTypeObject* self, PyObject* args, PyObject* kwargs) {
    // PickleProxy(Automaton) -> called upon pickling
    // PickleProxy() -> called upon unpickling

    PickleProxy* proxy;

    proxy = F(PyObject_New)(PickleProxy, &pickle_proxy_type);
    if (UNLIKELY(proxy == NULL)) {
        return NULL;
    }

    proxy->automaton = NULL;
    proxy->lookup.table = NULL;
    proxy->lookup.size  = DEFAULT_ADDRESS_LOOKUP_SIZE;
    proxy->lookup.index = 0;

    if (!F(PyArg_ParseTuple)(args, "O", &proxy->automaton)) {
        PyErr_Clear();
    }

    if (proxy->automaton != NULL) {
        // pickling -- no address lookup is needed
    } else {
        // unpickling
        proxy->lookup.table = (AddressPair*)memory_alloc(sizeof(AddressPair) * proxy->lookup.size);
        if (UNLIKELY(proxy->lookup.table == NULL)) {
            PyErr_NoMemory();
            return NULL;
        }
    }

    return (PyObject*)proxy;
}

// --------------------------------------------------

static void
address_lookup__cleanup(AddressLookup* lookup, int release_objects);

static void
pickle_proxy_del(PyObject* object) {

    PickleProxy* proxy;
    
    proxy = (PickleProxy*)object;

    if (proxy->automaton) {
        address_lookup__cleanup(&proxy->lookup, proxy->automaton->store == STORE_ANY);
        Py_DECREF(proxy->automaton);
    }
}

// --------------------------------------------------

UNUSED static void
address_lookup_dump(AddressLookup* lookup, FILE* out) {

    AddressPair* result;
    size_t i;

    fprintf(out, "[\n");
    for (i=0; i < lookup->index; i++) {
        result = &lookup->table[i];
        fprintf(out, "%3lu: %p -> %p\n", i, result->unpickled, result->current);
    }
    fprintf(out, "]\n");
}

PURE ALWAYS_INLINE static int
pickle_proxy__cmp_addresses(const void* a, const void* b) {
    uintptr_t A_ptr;
    uintptr_t B_ptr;

    A_ptr = (uintptr_t)((const AddressPair*)a)->unpickled;
    B_ptr = (uintptr_t)((const AddressPair*)b)->unpickled;

    if (A_ptr < B_ptr)
        return -1;
    else if (A_ptr > B_ptr)
        return +1;
    else
        return 0;
}


PURE static TrieNode*
address_lookup_get(AddressLookup* lookup, TrieNode* unpickled) {

    AddressPair* result;

    result = (AddressPair*)bsearch(
                &unpickled,
                lookup->table,
                lookup->index,
                sizeof(AddressPair),
                pickle_proxy__cmp_addresses);


    if (LIKELY(result != NULL)) {
        return result->current;
    }

    return NULL;
}


static void
pickle_proxy__finalize_unpickle(PickleProxy* proxy) {

    TrieNode* node;
    size_t i;
    size_t k;

    // 0. get the root node -- it's always pickled first
    proxy->root = proxy->lookup.table[0].current;

    // 1. sort addresses to make them bsearch-able
    qsort(proxy->lookup.table,
          proxy->lookup.index,
          sizeof(AddressPair),
          pickle_proxy__cmp_addresses);

    // 2. update fail and next links to actual pointers
    for (i=0; i < proxy->lookup.index; i++) {
        node = proxy->lookup.table[i].current;

        if (node->fail != NULL) {
            node->fail = address_lookup_get(&proxy->lookup, node->fail);
            if (UNLIKELY(node->fail == NULL)) {
                PyErr_Format(PyExc_ValueError, "Unpickle node %lu: cannot locate fail node", i);
                return;
            }
        }

        for (k=0; k < node->n; k++) {
            node->next[k] = address_lookup_get(&proxy->lookup, node->next[k]);
            if (UNLIKELY(node->next[k] == NULL)) {
                PyErr_Format(PyExc_ValueError, "Unpickle node %lu: cannot locate next[%lu]", i, k);
                return;
            }
        }
    }

    // 3. free memory
    memory_free(proxy->lookup.table);
    proxy->lookup.table = NULL;
    proxy->lookup.size  = 0;
    proxy->lookup.index = 0;
}

static void
address_lookup__cleanup(AddressLookup* lookup, int release_objects) {

    size_t i;
    TrieNode* node;

    // - During pickling no memory is allocate.
    // - There are allocation during unpickling, but if the whole
    //   process succedded then the lookup table is free.
    //   If there were error, we need to free already allocated objects.
    if (LIKELY(lookup->table == NULL)) {
        return;
    }

    for (i=0; i < lookup->index; i++) {
        node = lookup->table[i].current;

        if (release_objects && node->eow) {
            Py_DECREF(node->output.object);
        }

        xfree(node->next);
        memory_free(node);
    }

    memory_free(lookup->table);
    lookup->table = NULL;
    lookup->size  = 0;
    lookup->index = 0;
}

static PyObject*
pickle_proxy_append__single(PickleProxy* proxy, PyObject* bytes, PyObject* value) {

    TrieNode** unpickled;
    TrieNode* node;
    TrieNode* dump;
    TrieNode** arr;
    AddressPair* pair;
    size_t i;

    AddressPair* new_lookup;
    size_t new_size;

    // 1. restore data
    unpickled = (TrieNode**)(PyBytes_AS_STRING(bytes));
    dump      = (TrieNode* )(PyBytes_AS_STRING(bytes) + PICKLE_POINTER_SIZE);
    arr       = (TrieNode**)(PyBytes_AS_STRING(bytes) + PICKLE_POINTER_SIZE + PICKLE_TRIENODE_SIZE);

    node = (TrieNode*)memory_alloc(sizeof(TrieNode));
    if (UNLIKELY(node == NULL)) {
        PyErr_NoMemory();
        return NULL;
    }

    if (value != NULL) {
        node->output.object = value;
        Py_INCREF(value);
    } else {
        node->output.integer = dump->output.integer;
    }

    node->n      = dump->n;
    node->eow    = dump->eow;
    node->letter = dump->letter;
    node->fail   = dump->fail;

    if (node->n > 0) {
        node->next = (TrieNode**)memory_alloc(sizeof(TrieNode*) * node->n);
        if (UNLIKELY(node->next == NULL)) {
            PyErr_NoMemory();
            goto exception;
        }

        for (i=0; i < node->n; i++) {
            node->next[i] = arr[i];
        }
    } else {
        node->next = NULL;
    }

    // 2. register node's pointer in lookup table
    if (UNLIKELY(proxy->lookup.index >= proxy->lookup.size)) {
        new_size   = proxy->lookup.size + proxy->lookup.size/2; // grow the size by 1.5
        new_lookup = (AddressPair*)memrealloc(proxy->lookup.table, sizeof(AddressPair) * new_size);
        if (UNLIKELY(new_lookup == NULL)) {
            PyErr_NoMemory();
            goto exception;
        }

        proxy->lookup.table = new_lookup;
        proxy->lookup.size  = new_size;
    }

    pair = &proxy->lookup.table[proxy->lookup.index];
    pair->unpickled = *unpickled;
    pair->current   = node;
    proxy->lookup.index += 1;

    Py_RETURN_NONE;

exception:
    Py_XDECREF(value);
    if (node) {
        xfree(node->next);
        memory_free(node);
    }

    return NULL;
}


static PyObject*
pickle_proxy_append(PyObject* self, PyObject* args) {

    PickleProxy* proxy;
    PyObject* bytes;
    PyObject* value;

    if (UNLIKELY(PyTuple_GET_SIZE(args) != 1)) {
        PyErr_SetString(PyExc_ValueError, "Wrong number of arguments");
        return NULL;
    }

    proxy = (PickleProxy*)self;
    args  = PyTuple_GET_ITEM(args, 0);

    if (F(PyBytes_Check)(args)) {
        return pickle_proxy_append__single(proxy, args, NULL);
    }

    if (PyTuple_Check(args)) {
        if (UNLIKELY(PyTuple_GET_SIZE(args) != 2)) {
            PyErr_SetString(PyExc_ValueError, "wrong number of arguments");
            return NULL;
        }

        bytes = PyTuple_GetItem(args, 0);
        value = PyTuple_GetItem(args, 1);

        return pickle_proxy_append__single(proxy, bytes, value);
    }

    if (UNLIKELY(args != Py_None)) {
        PyErr_SetString(PyExc_ValueError, "expected None");
        return NULL;
    }

    pickle_proxy__finalize_unpickle(proxy);
    if (LIKELY(F(PyErr_Occurred)() == NULL)) {
        Py_RETURN_NONE;
    } else {
        address_lookup__cleanup(&proxy->lookup, proxy->automaton->store == STORE_ANY);
        return NULL;
    }
}

// --------------------------------------------------

static PyObject*
pickle_proxy_extend(PyObject* self, PyObject* args) {

    return PyExc_NotImplementedError;
}

// --------------------------------------------------

static PyObject*
pickle_proxy___reduce__(PyObject* object) {

    PickleProxy* proxy = (PickleProxy*)object;

    PyObject* iter;
    PyObject* tuple;

    iter = (PyObject*)streamnodes_iter_new(proxy->automaton);
    if (UNLIKELY(iter == NULL)) {
        goto exception;
    }

    tuple = F(Py_BuildValue)("O()OO", Py_TYPE(proxy), Py_None, iter);
    if (UNLIKELY(tuple == NULL)) {
        goto exception;
    }

    Py_DECREF(iter);

    return tuple;

exception:
    Py_XDECREF(iter);
    Py_XDECREF(tuple);
    return NULL;
}


#define method(name, kind) {#name, (PyCFunction)pickle_proxy_##name, kind, ""}
static
PyMethodDef pickle_proxy_methods[] = {
    method(append,      METH_VARARGS),
    method(extend,      METH_VARARGS),
    method(__reduce__,  METH_VARARGS),

    {NULL, NULL, 0, NULL}
};
#undef method

// --------------------------------------------------

static PyTypeObject pickle_proxy_type = {
    PY_OBJECT_HEAD_INIT
    "ahocorasick._PickleProxy",                 /* tp_name */
    sizeof(PickleProxy),                        /* tp_size */
    0,                                          /* tp_itemsize? */
    (destructor)pickle_proxy_del,               /* tp_dealloc */
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
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    pickle_proxy_methods,                       /* tp_methods */
    0,                                          /* tp_members */
    0,                                          /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    0,                                          /* tp_init */
    0,                                          /* tp_alloc */
    pickle_proxy_new,                           /* tp_new */
};
