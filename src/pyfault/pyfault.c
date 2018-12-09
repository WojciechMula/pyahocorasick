#include "pyfault.h"

#include <stdlib.h>


static int pycall      = -1;
static int pycall_fail = -1;
static int pycall_trap = 0;

void initialize_pyfault(void) {
    const char* fail   = getenv("PYCALL_FAIL");
    const char* trap   = getenv("PYCALL_TRAP");

    if (fail != NULL) {
        pycall_fail = atoi(fail);
    }

    if (trap != NULL) {
        pycall_trap = 1;
    }
}


PyObject* _PyObject_New_custom(PyTypeObject* typeobj) {
    pycall += 1;

    if (pycall == pycall_fail) {
        if (pycall_trap) {
            __builtin_trap();
        }

        printf("Failed pycall #%d\n", pycall);
        PyErr_NoMemory();
        return NULL;
    }

    return PyObject_New(PyObject, typeobj);
}
