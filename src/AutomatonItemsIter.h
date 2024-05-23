/*
    This is part of pyahocorasick Python module.

    AutomatonItemsIter const, struct & methods declarations.
    This class implements iterator walk over trie, that returns
    words and associated values. Object of this class is
    returned by 'keys'/'values'/'items' methods of Automaton class.

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
    License   : BSD-3-Clause (see LICENSE)
*/
#ifndef ahocorasick_AutomatonItemsIter_h_included
#define ahocorasick_AutomatonItemsIter_h_included

#include "common.h"
#include "Automaton.h"

typedef enum {
    ITER_KEYS,
    ITER_VALUES,
    ITER_ITEMS
} ItemsType;

typedef enum {
    MATCH_PREFIX,
    MATCH_WHOLE,
} PatternMatchType;


typedef struct AutomatonItemsIter {
    PyObject_HEAD

    Automaton*  automaton;
    int         version;        ///< automaton version
    TrieNode*   state;          ///< current automaton node
    TRIE_LETTER_TYPE letter;    ///< current letter
    List        stack;          ///< stack
    ItemsType   type;           ///< type of iterator (KEYS/VALUES/ITEMS)

    size_t pattern_length;
    TRIE_LETTER_TYPE* pattern;  ///< pattern
    PatternMatchType matchtype; ///< how pattern have to be handled
} AutomatonItemsIter;


/* new() */
static PyObject*
automaton_items_iter_new(
    Automaton* automaton,
    const TRIE_LETTER_TYPE* word,
    const Py_ssize_t wordlen,
    const PatternMatchType  matchtype
);

#endif
