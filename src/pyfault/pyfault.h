#ifndef PYFAULT_H_
#define PYFAULT_H_

#include <Python.h>

#define F(name) name##_custom

void initialize_pyfault(void);

// --- python function wrappers ----------------------------------------- 

int check(void);

#define PyObject_New_custom(type, typeobj) ((type*)(_PyObject_New_custom(typeobj)))
PyObject* _PyObject_New_custom(PyTypeObject* typeobj);

#define PyArg_ParseTuple_custom(arg1, arg2, ...) (check() ? 0 : PyArg_ParseTuple(arg1, arg2, __VA_ARGS__))

#endif // PYFAULT_H_
