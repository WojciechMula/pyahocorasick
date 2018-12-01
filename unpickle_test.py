# -*- coding: utf-8 -*-

import ahocorasick
import unittest
import struct


class TreeNodeBuilder(object):
    def __init__(self):
        self.integer = 0
        self.fail    = 0
        self.n       = 0
        self.eow     = 0
        self.letter  = 0
        self.next    = []


    def dump(self):
        """
        On Debian 64-bit, GCC 7.2 it is:

        integer   : size 8, offset 0
        fail      : size 8, offset 8
        n         : size 4, offset 16
        eow       : size 1, offset 20
        padding   : size 3
        letter    : size 4, offset 24
        padding   : size 4
        next      : size 8, offset 32 -- omitted in dump

        """

        assert self.n == len(self.next)

        next = b''
        for node in self.next:
            next += struct.pack('Q', node)

        node = struct.pack('QQIBBBBII',
            self.integer,
            self.fail,
            self.n,
            self.eow, 0, 0, 0,
            self.letter, 0)

        assert len(node) == 32

        return node + next


class TestUnpickleRaw(unittest.TestCase):

    # raw constructor get 9-tuple (see Automaton.c):
    # 1. count of nodes
    # 2. serialized nodes (as bytes or string in Py2)
    # 3. kind
    # 4. store
    # 5. key type
    # 6. version (internal counter)
    # 7. word count
    # 8. length of the longest word
    # 9. python values saved in a trie (if store == ahocorasick.STORE_ANY

    def setUp(self):
        self.count       = 0
        self.raw         = b''
        self.kind        = ahocorasick.EMPTY
        self.store       = ahocorasick.STORE_ANY
        self.key_type    = ahocorasick.KEY_STRING
        self.version     = 0
        self.word_count  = 0
        self.longest     = 0
        self.values      = []


    def create_automaton(self):
        # alter values set in setUp
        args = (self.count, self.raw, self.kind, self.store, self.key_type,
                self.version, self.word_count, self.longest, self.values);

        return ahocorasick.Automaton(*args)


    def create_node_builder(self, letter, eow, children):
        builder = TreeNodeBuilder()
        builder.letter  = ord(letter)
        builder.next    = [i + 1 for i in children] # starts from 1
        builder.n       = len(children)
        builder.eow     = eow

        return builder


    def create_raw_node(self, letter, eow, children):
        return self.create_node_builder(letter, eow, children).dump()


    # --------------------------------------------------


    def test__construct_empty(self):

        A = self.create_automaton()

        self.assertTrue(A.kind == ahocorasick.EMPTY)
        self.assertTrue(len(A) == 0)


    def test__construct_simple_trie(self):

        """
        trie for set {he, her, his, him, it}

        #0 -> [h #1 ] -> [e #2*] -> [r #3*]
         |           \-> [i #4 ] -> [s #5*]
         |                      \-> [m #6*]
         |
         +--> [i #7 ] -> [t #8 ]
        """
        values = ["HE", "HER", "HIS", "HIM", "IT"]

        node0 = self.create_raw_node('_', 0, [1, 7])
        node1 = self.create_raw_node('h', 0, [2, 4])
        node2 = self.create_raw_node('e', 1, [3])       # HE
        node3 = self.create_raw_node('r', 1, [])        # HER
        node4 = self.create_raw_node('i', 0, [5, 6])
        node5 = self.create_raw_node('s', 1, [])        # HIS
        node6 = self.create_raw_node('m', 1, [])        # HIM
        node7 = self.create_raw_node('i', 0, [8])
        node8 = self.create_raw_node('t', 1, [])        # IT

        self.count = 9
        self.raw   = node0 + node1 + node2 + node3 + node4 + node5 + node6 + node7 + node8
        self.kind  = ahocorasick.TRIE
        self.values = values
        self.word_count = 5

        A = self.create_automaton()
        self.assertEqual(len(A), 5)
        self.assertEqual(A.get("he"), "HE")
        self.assertEqual(A.get("her"), "HER")
        self.assertEqual(A.get("him"), "HIM")
        self.assertEqual(A.get("his"), "HIS")
        self.assertEqual(A.get("it"),  "IT")


    def test__construct_wrong_kind(self):

        self.kind = 10000

        with self.assertRaisesRegex(ValueError, "kind value.*"):
            self.create_automaton()


    def test__construct_wrong_store(self):

        self.store = 10000

        with self.assertRaisesRegex(ValueError, "store value.*"):
            self.create_automaton()


    def test__construct_wrong_key_type(self):

        self.key_type = 10000

        with self.assertRaisesRegex(ValueError, "key_type must have.*"):
            self.create_automaton()


    def test__construct_simple_trie__wrong_index(self):
        """
        trie for set {he}

        #0 -> [h #1*] -> [e #2*]
        """

        node0 = self.create_raw_node('_', 0, [1])
        node1 = self.create_raw_node('h', 1, [2])   # expect python value
        node2 = self.create_raw_node('e', 1, [])    # also python value

        self.count  = 3
        self.raw    = node0 + node1 + node2
        self.kind   = ahocorasick.TRIE
        self.values = ["HE"]                        # but we provide a too short collection
        self.word_count = 2

        with self.assertRaises(IndexError):
            self.create_automaton()


    def test__truncated_raw__case_1(self):

        self.count = 1 # we're saying this is a non-empty trie, but given empty data
        self.raw   = b''
        self.kind  = ahocorasick.TRIE

        with self.assertRaisesRegex(ValueError, r"Data truncated \[parsing header of node #0\].*"):
            self.create_automaton()


    def test__truncated_raw__case_2(self):
        """
        trie for set {he}

        #0 -> [h #1 ] -> [e #2*]
        """

        node0 = self.create_raw_node('_', 0, [1])
        node1 = self.create_raw_node('h', 0, [2])
        node2 = self.create_raw_node('e', 1, [])
        raw   = node0 + node1 + node2

        self.count  = 3
        self.kind   = ahocorasick.TRIE

        for length in range(len(raw)):
            self.raw = raw[:length] # truncate data and expect fail
            with self.assertRaisesRegex(ValueError, "Data truncated.*"):
                self.create_automaton()


    def test__malicious_next_pointer(self):
        """
        #0 -> [? #1 ]
        """

        node0 = self.create_raw_node('_', 0, [1])
        node1 = self.create_raw_node('?', 0, [16]) # the second node point to non-existent node

        self.count = 2
        self.raw   = node0 + node1
        self.kind  = ahocorasick.TRIE

        with self.assertRaisesRegex(ValueError, "Node #1 malformed: next link #0 points to.*"):
            self.create_automaton()


    def test__malicious_fail_pointer(self):
        """
        trie with just one node
        """

        builder = self.create_node_builder('_', 0, [])
        builder.fail = 42

        self.count = 1
        self.raw   = builder.dump()
        self.kind  = ahocorasick.TRIE

        with self.assertRaisesRegex(ValueError, "Node #0 malformed: the fail link points to.*"):
            self.create_automaton()


    def test__values_leaks(self):

        # create not connected nodes, but each hold a value
        good_nodes = 1000
        raw    = b''
        values = []
        for i in range(good_nodes):
            raw += self.create_raw_node('?', 1, [])
            values.append(tuple("node %d" % i))

        # create the last node that will cause error -- malformed next pointer
        raw += self.create_raw_node('?', 1, [10000])
        values.append(tuple("never reached"))

        self.count  = good_nodes + 1
        self.raw    = raw
        self.kind   = ahocorasick.TRIE
        self.values = values

        with self.assertRaises(ValueError):
            self.create_automaton()



if __name__ == '__main__':
    print("WARNING: these tests deal with in-memory representation (see TreeNodeBuilder),")
    print("         they were meant to test low-level implementation of pickling.")
    print("         Might segfault on your machine which is not necessary a bug in pyahocorasick.")
    unittest.main()
