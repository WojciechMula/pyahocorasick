#pragma once

#include "../../common.h"
#include "../../Automaton.h"

typedef struct StackItem {
    TrieNode*   node;
    ssize_t     index;
} StackItem;

typedef struct StreamNodesIter {
    PyObject_HEAD

    Automaton*  automaton;
    StackItem*  stack;
    ssize_t     stack_top;
} StreamNodesIter;

/* new() */
static PyObject*
streamnodes_iter_new(Automaton* automaton);

