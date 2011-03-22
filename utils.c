static PyObject*
pymod_get_string(PyObject* args, int index, char** word, ssize_t* wordlen) {
	PyObject* obj;

	obj = PyTuple_GetItem(args, index);
	if (obj == NULL)
		return NULL;

	if (PyBytes_Check(obj)) {
		if (PyBytes_AsStringAndSize(obj, word, wordlen))
			return NULL;
		else {
			Py_INCREF(obj);
			return obj;
		}
	}
	else {
		PyErr_SetString(PyExc_ValueError, "bytes expected");
		return NULL;
	}
}

/**
	[start, [end]]
*/
static int
pymod_parse_start_end(
	PyObject* args,
	int idx_start, int idx_end,
	const ssize_t min, const ssize_t max,
	ssize_t* Start, ssize_t* End
) {
	PyObject* tmp;
#define start (*Start)
#define end (*End)

	start	= min;
	end		= max;

	// first argument
	tmp = PyTuple_GetItem(args, idx_start);
	if (tmp == NULL) {
		PyErr_Clear();
		return 0;
	}

	tmp = PyNumber_Index(tmp);
	if (tmp == NULL)
		return -1;

	start = PyNumber_AsSsize_t(tmp, PyExc_IndexError);
	if (start == -1 and PyErr_Occurred())
		return -1;

	if (start < 0)
		start = max + start;

	if (start < min or start >= max) {
		PyErr_Format(PyExc_IndexError, "start index not in range %d..%d", min, max);
		return -1;
	}

	// second argument
	tmp = PyTuple_GetItem(args, idx_end);
	if (tmp == NULL) {
		PyErr_Clear();
		return 0;
	}

	tmp = PyNumber_Index(tmp);
	if (tmp == NULL)
		return -1;

	end = PyNumber_AsSsize_t(tmp, PyExc_IndexError);
	if (end == -1 and PyErr_Occurred())
		return -1;

	if (end < 0)
		end = max - 1 + end;

	if (end < min or end >= max) {
		PyErr_Format(PyExc_IndexError, "end index not in range %d..%d", min, max);
		return -1;
	}

	return 0;

#undef start
#undef end
}

