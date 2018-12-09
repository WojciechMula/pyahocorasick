#ifndef PYCALLFAULT_H_
#define PYCALLFAULT_H_

#include <Python.h>

#define F(name) name##_custom

void initialize_pycallfault(void);

// --- python function wrappers ----------------------------------------- 

int check(void);
int check_and_set_error(void);

#define PyObject_New_custom(type, typeobj) (check_and_set_error() ? NULL : PyObject_New(type, typeobj))

#define PyArg_ParseTuple_custom(arg1, arg2, ...) (check() ? 0 : PyArg_ParseTuple(arg1, arg2, __VA_ARGS__))

#define PyTuple_GetItem_custom(arg1, arg2) (check_and_set_error() ? NULL : PyTuple_GetItem(arg1, arg2))

#define PyList_GetItem_custom(arg1, arg2) (check_and_set_error() ? NULL : PyList_GetItem(arg1, arg2))

#define PyNumber_AsSsize_t_custom(arg1, arg2) (check_and_set_error() ? -1 : PyNumber_AsSsize_t(arg1, arg2))

#define PyList_New_custom(arg) (check_and_set_error() ? NULL : PyList_New(arg))

#define Py_BuildValue_custom(arg, ...) (check_and_set_error() ? NULL : Py_BuildValue(arg, __VA_ARGS__))

#define PyCallable_Check_custom(arg) (check() ? 0 : PyCallable_Check(arg))

#define PyUnicode_Check_custom(arg) (check() ? 0 : PyUnicode_Check(arg))

#define PyBytes_Check_custom(arg) (check() ? 0 : PyBytes_Check(arg))

#define PyCallable_Check_custom(arg) (check() ? 0 : PyCallable_Check(arg))

#define PyNumber_Check_custom(arg) (check() ? 0 : PyNumber_Check(arg))

#define PyTuple_Check_custom(arg) (check() ? 0 : PyTuple_Check(arg))

#define PyObject_CallFunction_custom(arg1, arg2, ...) (check_and_set_error() ? NULL : PyObject_CallFunction(arg1, arg2, __VA_ARGS__))

#endif // PYCALLFAULT_H_
