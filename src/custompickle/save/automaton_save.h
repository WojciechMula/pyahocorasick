#pragma once

#include "../../../common.h"

#define automaton_save_doc \
	"Save content of automaton in an on-disc file"

PyObject*
automaton_save(PyObject* self, PyObject* args);

