#pragma once

#include "../../common.h"
#include "../../Automaton.h"

typedef struct AddressPair {
    TrieNode* unpickled;    ///< unpickled pointer
    TrieNode* current;      ///< currently used pointer
} AddressPair;


typedef struct AddressLookup {
    AddressPair*    table;  ///< map unpickled ptr -> current ptr
    size_t          size;   ///< physical size of map
    size_t          index;  ///< first free entry
} AddressLookup;


typedef struct PickleProxy {
    PyObject_HEAD

    Automaton*      automaton;  ///< set during pickling
    TrieNode*       root;       ///< set during unpickling
    AddressLookup   lookup;
} PickleProxy;

