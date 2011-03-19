PyObject*
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
