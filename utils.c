/*
	This is part of pyahocorasick Python module.
	
	Helpers functions.
	This file is included directly.

	Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
	License   : public domain
*/


/* returns bytes or unicode internal buffer */
static PyObject*
pymod_get_string(PyObject* obj, TRIE_LETTER_TYPE** word, ssize_t* wordlen) {
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
            *word = (TRIE_LETTER_TYPE*)PyBytes_AS_STRING(obj);
            *wordlen = PyBytes_GET_SIZE(obj);
            Py_INCREF(obj);
            return obj;
        }
        else {
            PyErr_SetString(PyExc_TypeError, "bytes expected");
            return NULL;
        }
#   endif
#else // PY_MAJOR_VERSION == 3
	if (PyString_Check(obj)) {
        *word = PyString_AS_STRING(obj);
        *wordlen = PyString_GET_SIZE(obj);

        Py_INCREF(obj);
        return obj;
    } else {
		PyErr_SetString(PyExc_TypeError, "string1 required");
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

