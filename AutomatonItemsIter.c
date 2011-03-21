typedef struct AutomatonItemsStackItem {
	LISTITEM_data

	struct TrieNode*	node;
	size_t depth;
} AutomatonItemsStackItem;

#define StackItem AutomatonItemsStackItem

static PyObject*
automaton_items_iter_new(Automaton* automaton) {
	AutomatonItemsIter* iter;

	iter = (AutomatonItemsIter*)PyObject_New(AutomatonItemsIter, &automaton_items_iter_type);
	if (iter == NULL)
		return NULL;

	iter->automaton = automaton;
	iter->state	= NULL;
	iter->type = ITER_KEYS;
	list_init(&iter->stack);

	StackItem* new_item = (StackItem*)list_item_new(sizeof(StackItem));
	if (not new_item) {
		PyErr_SetNone(PyExc_MemoryError);
		return NULL;
	}
	else {
		new_item->node = automaton->root;
		new_item->depth = 0;
		list_push_front(&iter->stack, (ListItem*)new_item);
	}


	iter->n = 256;
	iter->buffer = memalloc(iter->n);
	if (iter->buffer == NULL) {
		PyObject_Del((PyObject*)iter);
		return NULL;
	}
	
	Py_INCREF((PyObject*)iter->automaton);
	return (PyObject*)iter;
}


#define iter ((AutomatonItemsIter*)self)

static void
automaton_items_iter_del(PyObject* self) {
	if (iter->buffer)
		memfree(iter->buffer);
	
	list_delete(&iter->stack);
	Py_DECREF(iter->automaton);

	PyObject_Del(self);
}


static PyObject*
automaton_items_iter_iter(PyObject* self) {
	Py_INCREF(self);
	return self;
}



static PyObject*
automaton_items_iter_next(PyObject* self) {
	while (true) {
		StackItem* item = (StackItem*)list_pop_first(&iter->stack);
		if (item == NULL or item->node == NULL)
			return NULL; /* Stop iteration */

		iter->state = item->node;
		const int n = iter->state->n;
		int i;
		for (i=0; i < n; i++) {
			StackItem* new_item = (StackItem*)list_item_new(sizeof(StackItem));
			if (not new_item) {
				PyErr_SetNone(PyExc_MemoryError);
				return NULL;
			}

			new_item->node  = iter->state->next[i];
			new_item->depth = item->depth + 1;
			list_push_front(&iter->stack, (ListItem*)new_item);
		}

		iter->buffer[item->depth] = iter->state->byte;
		if (iter->state->eow) {
			PyObject* key;
			PyObject* val;
			PyObject* it;
			switch (iter->type) {
				case ITER_KEYS:
					key = PyBytes_FromStringAndSize(iter->buffer + 1, item->depth);
					return key;

				case ITER_VALUES:
					val = (PyObject*)iter->state->output;
					Py_INCREF(val);
					return val;

				case ITER_ITEMS:
					it = PyTuple_New(2);
					if (it) {
						key = PyBytes_FromStringAndSize(iter->buffer + 1, item->depth);
						val = (PyObject*)iter->state->output;
						PyTuple_SET_ITEM(it, 0, key);
						PyTuple_SET_ITEM(it, 1, val);

						Py_XDECREF(key);

						return it;
					}

					// in case of error
					return NULL;
			}
		}
	}
}

#undef StackItem
#undef iter

static PyTypeObject automaton_items_iter_type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	"AutomatonItemsIter",						/* tp_name */
	sizeof(AutomatonItemsIter),					/* tp_size */
	0,											/* tp_itemsize? */
	(destructor)automaton_items_iter_del,		/* tp_dealloc */
	0,                                      	/* tp_print */
	0,                                         	/* tp_getattr */
	0,                                          /* tp_setattr */
	0,                                          /* tp_reserved */
	0,											/* tp_repr */
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
	automaton_items_iter_iter,					/* tp_iter */
	automaton_items_iter_next,					/* tp_iternext */
	0,											/* tp_methods */
	0,						                	/* tp_members */
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
