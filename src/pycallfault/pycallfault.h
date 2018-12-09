#ifndef PYCALLFAULT_H_
#define PYCALLFAULT_H_

#include <Python.h>

#define F(name) name##_custom

void initialize_pycallfault(void);

// --- python function wrappers ----------------------------------------- 

int check(void);
int check_and_set_error(void);

#define PyObject_New_custom(type, typeobj) ((type*)(_PyObject_New_custom(typeobj)))
PyObject* _PyObject_New_custom(PyTypeObject* typeobj);

#define PyArg_ParseTuple_custom(arg1, arg2, ...) (check() ? 0 : PyArg_ParseTuple(arg1, arg2, __VA_ARGS__))

#define PyTuple_GetItem_custom(arg1, arg2) (check_and_set_error() ? NULL : PyTuple_GetItem(arg1, arg2))

#define PyNumber_AsSsize_t_custom(arg1, arg2) (check_and_set_error() ? -1 : PyNumber_AsSsize_t(arg1, arg2))

#define PyList_New_custom(arg) (check_and_set_error() ? NULL : PyList_New(arg))

#endif // PYCALLFAULT_H_
