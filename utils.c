/*
	This is part of pyahocorasick Python module.
	
	Helpers functions.
	This file is included directly.

	Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
	License   : public domain
*/


/* returns bytes or unicode internal buffer */
static PyObject*
pymod_get_string(PyObject* obj, TRIE_LETTER_TYPE** word, ssize_t* wordlen) {

	ssize_t i;
	char* bytes;

#ifdef PY3K
#   ifdef AHOCORASICK_UNICODE
        if (PyUnicode_Check(obj)) {
            *word = PyUnicode_AS_UNICODE(obj);
            *wordlen = PyUnicode_GET_SIZE(obj);
            Py_INCREF(obj);
            return obj;
        }
        else {
            PyErr_SetString(PyExc_TypeError, "string expected");
            return NULL;
        }
#   else
        if (PyBytes_Check(obj)) {
            *wordlen = PyBytes_GET_SIZE(obj);
			*word    = (TRIE_LETTER_TYPE*)malloc(*wordlen * TRIE_LETTER_SIZE);

			bytes = PyBytes_AS_STRING(obj);
			for (i=0; i < *wordlen; i++) {
				(*word)[i] = bytes[i];
			}
			// Note: there is no INCREF
            return obj;
        }
        else {
            PyErr_SetString(PyExc_TypeError, "bytes expected");
            return NULL;
        }
#   endif
#else // PY_MAJOR_VERSION == 3
	if (PyString_Check(obj)) {
        *wordlen = PyString_GET_SIZE(obj);
		*word    = (TRIE_LETTER_TYPE*)malloc(*wordlen * TRIE_LETTER_SIZE);

		bytes = PyString_AS_STRING(obj);
		for (i=0; i < *wordlen; i++) {
			(*word)[i] = bytes[i];
		}
		// Note: there is no INCREF
        Py_INCREF(obj);
		return obj;
    } else {
		PyErr_SetString(PyExc_TypeError, "string required");
		return NULL;
	}
#endif
}


static PyObject*
pymod_get_string_from_tuple(PyObject* tuple, int index, TRIE_LETTER_TYPE** word, ssize_t* wordlen) {
	PyObject* obj;

	obj = PyTuple_GetItem(tuple, index);
	if (obj)
		return pymod_get_string(obj, word, wordlen);
	else
		return NULL;
}


static bool
__read_sequence__from_tuple(PyObject* obj, TRIE_LETTER_TYPE** word, ssize_t* wordlen) {
	Py_ssize_t i;
	Py_ssize_t size = PyTuple_Size(obj);

	*wordlen = size;
	*word = (TRIE_LETTER_TYPE*)malloc(size * TRIE_LETTER_SIZE);
	if (*word == NULL) {
		PyErr_NoMemory();
		return false;
	}

	for (i=0; i < size; i++) {
		Py_ssize_t value = PyNumber_AsSsize_t(PyTuple_GetItem(obj, i), PyExc_ValueError);
		if (value == -1 && PyErr_Occurred()) {
			PyErr_Format(PyExc_ValueError, "item #%zd is not a number", i);
			free(*word);
			return NULL;
		}

		const Py_ssize_t min = 0;		// XXX: both min and max values will be configured
		const Py_ssize_t max = 65535;

		if (value < min || value > max) {
			PyErr_Format(PyExc_ValueError, "item #%zd: value %zd ouside range [%zd..%zd]", i, value, min, max);
			free(*word);
			return NULL;
		}

		(*word)[i] = (TRIE_LETTER_TYPE)value;
	}

	return true;
}


static bool
pymod_get_sequence(PyObject* obj, TRIE_LETTER_TYPE** word, ssize_t* wordlen) {
	if (PyTuple_Check(obj)) {
		return __read_sequence__from_tuple(obj, word, wordlen);
	} else {
		PyErr_Format(PyExc_TypeError, "argument is not a supported sequence type");
		return false;
	}
}


static bool
pymod_get_sequence_from_tuple(PyObject* tuple, int index, TRIE_LETTER_TYPE** word, ssize_t* wordlen) {
	PyObject* obj;

	obj = PyTuple_GetItem(tuple, index);
	if (obj)
		return pymod_get_sequence(obj, word, wordlen);
	else
		return false;
}


/* parse optional indexes used in few functions [start, [end]] */
static int
pymod_parse_start_end(
	PyObject* args,
	int idx_start, int idx_end,
	const ssize_t min, const ssize_t max,
	ssize_t* Start, ssize_t* End
) {
	PyObject* obj;
#define start (*Start)
#define end (*End)

	start	= min;
	end		= max;

	// first argument
	obj = PyTuple_GetItem(args, idx_start);
	if (obj == NULL) {
		PyErr_Clear();
		return 0;
	}

	obj = PyNumber_Index(obj);
	if (obj == NULL)
		return -1;

	start = PyNumber_AsSsize_t(obj, PyExc_IndexError);
    Py_DECREF(obj);
	if (start == -1 and PyErr_Occurred())
		return -1;

	if (start < 0)
		start = max + start;

	if (start < min or start >= max) {
		PyErr_Format(PyExc_IndexError, "start index not in range %zd..%zd", min, max);
		return -1;
	}

	// second argument
	obj = PyTuple_GetItem(args, idx_end);
	if (obj == NULL) {
		PyErr_Clear();
		return 0;
	}

	obj = PyNumber_Index(obj);
	if (obj == NULL)
		return -1;

	end = PyNumber_AsSsize_t(obj, PyExc_IndexError);
    Py_DECREF(obj);
	if (end == -1 and PyErr_Occurred())
		return -1;

	if (end < 0)
		end = max - 1 + end;

	if (end < min or end > max) {
		PyErr_Format(PyExc_IndexError, "end index not in range %zd..%zd", min, max);
		return -1;
	}

	return 0;

#undef start
#undef end
}


bool prepare_input(PyObject* self, PyObject* tuple, struct Input* input) {
#define automaton ((Automaton*)self)
	if (automaton->key_type == KEY_STRING) {
		input->py_word = pymod_get_string(tuple, &input->word, &input->wordlen);
		if (not input->py_word)
			return false;
	} else {
		if (not pymod_get_sequence(tuple, &input->word, &input->wordlen)) {
			return false;
		}

		input->py_word = NULL;
	}
#undef automaton

	return true;
}


bool prepare_input_from_tuple(PyObject* self, PyObject* args, int index, struct Input* input) {
	PyObject* tuple;

	tuple = PyTuple_GetItem(args, index);
	if (tuple)
		return prepare_input(self, tuple, input);
	else
		return false;
}


void destroy_input(struct Input* input) {
	if (input->py_word) {
		Py_DECREF(input->py_word);
	} else {
		free(input->word);
	}
}


void assign_input(struct Input* dst, struct Input* src) {

	dst->wordlen	= src->wordlen;
	dst->word		= src->word;
	dst->py_word	= src->py_word; // Note: there is no INCREF
}


void xfree(void* ptr) {
	if (ptr != NULL) {
		free(ptr);
	}
}
