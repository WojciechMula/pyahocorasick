/*
	This is part of pyahocorasick Python module.
	
	Implementation of pickling/unpickling routines for Automaton class

	Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
	License   : 3-clauses BSD (see LICENSE)
*/

/*
Pickling (automaton___reduce__):

1. assign sequential numbers to nodes in order to replace
   address with these numbers
   (pickle_dump_replace_fail_with_id)
2. save in array all nodes data in the same order as numbers,
   also replace fail and next links with numbers; collect on
   a list all values (python objects) stored in trie
   (pickle_dump_save)
3. clean up
   (pickle_dump_undo_replace or pickle_dump_revert_replace)

Unpickling (automaton_unpickle, called in Automaton constructor)
1. load all nodes from array
2. make number->node lookup table
3. replace numbers stored in fail and next pointers with
   real pointers, reassign python objects as values
*/


#include <string.h>

typedef struct NodeID {
	TrieNode* fail;	///< original fail value
	Py_uintptr_t id;			///< id
} NodeID;

typedef struct DumpState {
	int id;					///< next id
	size_t total_size;		///< number of nodes
	TrieNode* failed_on;	///< if fail while numerating, save node in order
							///  to revert changes made in trie
} DumpState;


// replace fail with pairs (fail, id)
static int
pickle_dump_replace_fail_with_id(TrieNode* node, const int depth, void* extra) {

	NodeID* repl;

    ASSERT(sizeof(NodeID*) <= sizeof(TrieNode*));
#define state ((DumpState*)extra)
	repl = (NodeID*)memalloc(sizeof(NodeID));
	if (LIKELY(repl != NULL)) {
		state->id += 1;
		state->total_size += trienode_get_size(node)/* - sizeof(TrieNode*)*/;

		repl->id   = state->id;
		repl->fail = node->fail;

		node->fail = (TrieNode*)repl;
		return 1;
	}
	else {
		// error, revert is needed!
		state->failed_on = node;
		return 0;
	}
#undef state
}


// revert changes in trie (in case of error)
static int
pickle_dump_revert_replace(TrieNode* node, const int depth, void* extra) {
#define state ((DumpState*)extra)
	if (state->failed_on != node) {
		NodeID* repl = (NodeID*)(node->fail);
		node->fail = repl->fail;
		memfree(repl);

		return 1;
	}
	else
		return 0;
#undef state
}


// revert changes in trie
static int
pickle_dump_undo_replace(TrieNode* node, const int depth, void* extra) {
#define state ((DumpState*)extra)
	NodeID* repl = (NodeID*)(node->fail);
	node->fail = repl->fail;
	memfree(repl);

	return 1;
#undef state
}


typedef struct PickleData {
	size_t		size;	///< size of array
	size_t		top;	///< first free address
	uint8_t*	data;	///< array

	PyObject* 	values;	///< a list (if store == STORE_ANY)
	bool		error;	///< error occured during pickling
} PickleData;


static int
pickle_dump_save(TrieNode* node, const int depth, void* extra) {
#define self ((PickleData*)extra)
#define NODEID(object) ((NodeID*)((TrieNode*)object)->fail)

	TrieNode* dump;
	TrieNode* tmp;
	TrieNode** arr;
	int i;
	
	dump = (TrieNode*)(self->data + self->top);

	// we do not save last pointer in array
	arr = (TrieNode**)(self->data + self->top + sizeof(TrieNode) - sizeof(TrieNode*));

	// append python object to the list
	if (node->eow and self->values) {
		if (PyList_Append(self->values, node->output.object) == -1) {
			self->error = true;
			return 0;
		}
	}

	// save node data
	if (self->values)
		dump->output.integer = 0;
	else
		dump->output.integer = node->output.integer;

	dump->n		= node->n;
	dump->eow	= node->eow;
	dump->letter	= node->letter;

	tmp = NODEID(node)->fail;
	if (tmp)
		dump->fail	= (TrieNode*)(NODEID(tmp)->id);
	else
		dump->fail	= NULL;

	// save array of pointers
	for (i=0; i < node->n; i++) {
		TrieNode* child = node->next[i];
		ASSERT(child);
		arr[i] = (TrieNode*)(NODEID(child)->id);	// save id of child node
	}

	// advance pointer
	self->top += trienode_get_size(node) - sizeof(TrieNode*);

	return 1;
#undef NODEID
#undef self
}

#define automaton___reduce___doc \
	"reduce"

static PyObject*
automaton___reduce__(PyObject* self, PyObject* args) {
#define automaton ((Automaton*)self)

	DumpState 	state;
	PickleData	data;
	PyObject* 	tuple;
    
	// 0. for an empty automaton do nothing
    if (automaton->count == 0) {
        // the class constructor feeded with an empty argument build an empty automaton
        return Py_BuildValue("O()", Py_TYPE(self));
    }

	// 1. numerate nodes
	state.id		= 0;
	state.failed_on	= NULL;
	state.total_size = 0;

	trie_traverse(automaton->root, pickle_dump_replace_fail_with_id, &state);
	if (state.failed_on) {
		// revert changes (partial)
		trie_traverse(automaton->root, pickle_dump_revert_replace, &state);

		// and set error
		PyErr_NoMemory();
		return NULL;
	}

	// 2. gather data
	data.error	= false;
	data.size	= state.total_size;
	data.top	= 0;
	data.data	= memalloc(data.size);
	data.values = NULL;

	if (data.data == NULL) {
		PyErr_NoMemory();
		goto exception;
	}

	if (automaton->store == STORE_ANY) {
		data.values = PyList_New(0);
		if (not data.values)
			goto exception;
	}

	trie_traverse(automaton->root, pickle_dump_save, &data);
	if (data.error)
		goto exception;

	if (automaton->store != STORE_ANY) { // always pickle a Python object
		data.values = Py_None;
		Py_INCREF(data.values);
	}

	/* 3: save tuple:
		* count
		* binary data
		* automaton->kind
		* automaton->store
		* automaton->version
		* automaton->longest_word
		* list of values
	*/

	tuple = Py_BuildValue(
#ifdef PY3K
        "O(ky#iiiiiO)",
#else
        "O(ks#iiiiiO)",
#endif
		Py_TYPE(self),
		state.id,
		data.data, data.top,
		automaton->kind,
		automaton->store,
		automaton->version,
		automaton->count,
		automaton->longest_word,
		data.values
	);

    if (data.values == Py_None) {
        data.values = NULL;
    }

	if (tuple) {
		// revert all changes
		trie_traverse(automaton->root, pickle_dump_undo_replace, NULL);

		// and free memory
		if (data.data)
			memfree(data.data);
		
		Py_XDECREF(data.values);

		return tuple;
	}
	else
		goto exception;

exception:
	// revert all changes
	trie_traverse(automaton->root, pickle_dump_undo_replace, NULL);

	// and free memory
	if (data.data)
		memfree(data.data);
	
	Py_XDECREF(data.values);
	return NULL;
#undef automaton
}


static bool
automaton_unpickle(
	Automaton* automaton,
	const size_t count,
	uint8_t* data,
	const size_t size,
	PyObject* values
) {
	TrieNode** id2node;

	TrieNode* node;
	TrieNode* dump;
	TrieNode** next;
	PyObject* value;
	int id;
	uint8_t* ptr;
	size_t i, j;
	size_t object_idx;

    if (UNLIKELY(size < count*(sizeof(TrieNode) - sizeof(TrieNode*)))) {
        PyErr_SetString(PyExc_ValueError, "binary data truncated (1)");
        return false;
    }

	id2node = (TrieNode**)memalloc((count+1) * sizeof(TrieNode));
	if (id2node == NULL) {
		goto no_mem;
    }

	// 1. make nodes
	id = 1;
	ptr = data;
	for (i=0; i < count; i++) {
		dump = (TrieNode*)(ptr);
		node = (TrieNode*)memalloc(sizeof(TrieNode));
		if (LIKELY(node != NULL)) {
			node->output	= dump->output;
			node->fail		= dump->fail;
			node->letter	= dump->letter;
			node->n			= dump->n;
			node->eow		= dump->eow;
			node->next		= NULL;
		}
		else
			goto no_mem;

		if (node->n > 0) {
			node->next = (TrieNode**)memalloc(node->n * sizeof(TrieNode*));
			if (LIKELY(node->next != NULL)) {
                if (UNLIKELY(ptr + sizeof(TrieNode) - sizeof(TrieNode*) > data + size)) {
                    PyErr_SetString(PyExc_ValueError, "binary data truncated (2)");
                    goto exception;
                }

				next = (TrieNode**)(ptr + sizeof(TrieNode) - sizeof(TrieNode*));
				for (j=0; j < node->n; j++)
					node->next[j] = next[j];
			}
			else {
				memfree(node);
				goto no_mem;
			}
		}

		id2node[id++] = node;
		ptr += trienode_get_size(node) - sizeof(TrieNode*);
		if (UNLIKELY(ptr > data + size)) {
			PyErr_SetString(PyExc_ValueError, "binary data truncated (3)");
			goto exception;
		}
	}

	// 2. restore pointers and references to pyobjects
	object_idx = 0;
	for (i=1; i < id; i++) {
		node = id2node[i];

		// references
		if (values and node->eow) {
			value = PyList_GetItem(values, object_idx++);
			if (value) {
				Py_INCREF(value);
				node->output.object = value;
			}
			else
				goto exception;
		}

		// pointers
#define POINTER(object) id2node[(size_t)(object)]
		if (node->fail)
			node->fail = POINTER(node->fail);

		for (j=0; j < node->n; j++)
			node->next[j] = POINTER(node->next[j]);
#undef POINTER
	}

	automaton->root  = id2node[1];

	memfree(id2node);
	return 1;

no_mem:
	PyErr_NoMemory();
exception:
	if (id2node) {
		for (i=1; i < id; i++) {
			TrieNode* tmp = id2node[i];
			if (tmp->n > 0)
				memfree(tmp->next);

			memfree(tmp);
		}

		memfree(id2node);
	};

	return 0;
}

