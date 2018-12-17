/*

Introduction
-----------------------------------------------------------

	To perform pickle and unpickle an additional object PickleProxy is used.
	It is responsible for serializing and deserializing underlaying automaton
	structure.

	Automaton itself pickles:
	* kind
	* key type,
	* store type,
	* number of words,
	* length of the longest word)
	* PickleProxy instance


Pickling
-----------------------------------------------------------

	PickleProxy serializes each trie node separately, using implicitely
	(non-public) StreamNodesIter. The iterator yields one of:

		1. pair: binary data describing a node + associated object value
		   (if automaton store == STORE_ANY and node is a terminating one)
		2. just binary data (if node is not terminating)
		3. None, as the end-of-stream marker

	Binary data has two or three parts:

		1. address of node at the moment of pickling
		2. binary image of node data
		3. array of pointers to children (not present when node's a leaf)

	Pickling has no memory footprint.


Unpickling
-----------------------------------------------------------

	Due to __reduce__ protocol, when we use iterator to emit portions
	of data, the class has to implement 'append' and 'extend' methods.
	Because I didn't want to create these public methods just for
	pickling, I decided to introduce PickleProxy.

	Unpickling happens in append method of PickleProxy.

	- For each binary image of node, the new node is allocates; it's
	  new and old (pickled) addreess are stored in an auxilary table.
	- After all nodes are recreated, fail and next pointers are replaced
	  from old ones to actual addresses.
	
	Unpickling requires extra memory: 2 * pointer size * number of nodes.
*/

#pragma once

#include "../../trienode.h"

// We save all TrieNode's fields except the last one, which is a pointer to array,
// as we're store that array just after the node
#define PICKLE_TRIENODE_SIZE (sizeof(TrieNode) - sizeof(TrieNode**))
#define PICKLE_POINTER_SIZE (sizeof(TrieNode*))
