#ifndef PYFAULT_H_
#define PYFAULT_H_

#include <Python.h>

#define F(name) name##_custom

void initialize_pyfault(void);

// --- python function wrappers ----------------------------------------- 

#define PyObject_New_custom(type, typeobj) ((type*)(_PyObject_New_custom(typeobj)))
PyObject* _PyObject_New_custom(PyTypeObject* typeobj);

#endif // PYFAULT_H_
