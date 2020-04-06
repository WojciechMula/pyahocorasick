# -*- coding: utf-8 -*-

import ahocorasick
import unittest
import struct
import sys


class TreeNodeBuilderBase(object):
    def __init__(self):
        self.integer = 0
        self.fail    = 0
        self.n       = 0
        self.eow     = 0
        self.next    = []


    def dump(self):

        assert self.n == len(self.next)

        next = b''
        for letter, node in self.next:
            next += self.dump_edge(letter, node)

        return self.dump_node() + next

if sys.version_info.major == 3:

    class TreeNodeBuilderPy3(TreeNodeBuilderBase):
        def dump_node(self):
            """
            On Debian 64-bit, GCC 7.3

            python3:

                integer   : size 8, offset 0
                fail      : size 8, offset 8
                n         : size 4, offset 16
                eow       : size 1, offset 20
                padding   : size 3
                next      : size 8, offset 24 -- omitted in dump

            python2:

                integer   : size 8, offset 0
                fail      : size 8, offset 8
                n         : size 4, offset 16
                eow       : size 1, offset 20
                padding   : size 1
                next      : size 8, offset 22 -- omitted in dump
            """
            node = struct.pack('=QQIBxxx',
                self.integer,
                self.fail,
                self.n,
                self.eow)

            assert len(node) == 24

            return node


        def dump_edge(self, letter, node):
            assert ord(letter) < 256

            b = struct.pack('=IQ', ord(letter), node)
            assert len(b) == 12

            return b


    TreeNodeBuilder = TreeNodeBuilderPy3

elif sys.version_info.major == 2:

    class TreeNodeBuilderPy2(TreeNodeBuilderBase):
        def dump_node(self):
            """
            On Debian 64-bit, GCC 7.3

            python2:

                integer   : size 8, offset 0
                fail      : size 8, offset 8
                n         : size 4, offset 16
                eow       : size 1, offset 20
                padding   : size 3
                next      : size 8, offset 24 -- omitted in dump
            """
            node = struct.pack('QQIBxxx',
                self.integer,
                self.fail,
                self.n,
                self.eow)

            assert len(node) == 24

            return node


        def dump_edge(self, letter, node):
            assert ord(letter) < 256

            b = struct.pack('=HQ', ord(letter), node)
            assert len(b) == 10

            return b


    TreeNodeBuilder = TreeNodeBuilderPy2


USE_EXACT_RAW = True


class TestUnpickleRaw(unittest.TestCase):

    def __init__(self, *args):
        super(TestUnpickleRaw, self).__init__(*args)

        if not hasattr(self, 'assertRaisesRegex'):
            # fixup for Py2
            self.assertRaisesRegex = self.assertRaisesRegexp


    # raw constructor get 7-tuple (see Automaton.c):
    # 1. serialized nodes (as list of bytes or strings)
    # 2. kind
    # 3. store
    # 4. key type
    # 5. word count
    # 6. length of the longest word
    # 7. python values saved in a trie (if store == ahocorasick.STORE_ANY)

    def setUp(self):
        self.count       = 0
        self.raw         = b''
        self.kind        = ahocorasick.EMPTY
        self.store       = ahocorasick.STORE_ANY
        self.key_type    = ahocorasick.KEY_STRING
        self.word_count  = 0
        self.longest     = 0
        self.values      = []


    def create_automaton(self, use_exact_raw=False):
        # alter values that were set in setUp
        if use_exact_raw:
            raw = self.raw
        else:
            raw = [self.create_raw_count(self.count) + self.raw]

        args = (raw, self.kind, self.store, self.key_type,
                self.word_count, self.longest, self.values);

        return ahocorasick.Automaton(*args)


    def create_node_builder(self, eow, children):
        builder = TreeNodeBuilder()
        builder.next    = [(letter, i + 1) for letter, i in children] # starts from 1
        builder.n       = len(children)
        builder.eow     = eow

        return builder


    def create_raw_count(self, n):
        return struct.pack('Q', n)


    def create_raw_node(self, eow, children):
        return self.create_node_builder(eow, children).dump()


    # --------------------------------------------------


    def test__construct_empty(self):

        A = self.create_automaton()

        self.assertTrue(A.kind == ahocorasick.EMPTY)
        self.assertTrue(len(A) == 0)


    def test__construct_simple_trie(self):

        r"""
        trie for set {he, her, his, him, it}

        #0 -> [h #1 ] -> [e #2*] -> [r #3*]
         |           \-> [i #4 ] -> [s #5*]
         |                      \-> [m #6*]
         |
         +--> [i #7 ] -> [t #8 ]
        """
        values = ["HE", "HER", "HIS", "HIM", "IT"]

        node0 = self.create_raw_node(0, [('h', 1), ('i', 7)])
        node1 = self.create_raw_node(0, [('e', 2), ('i', 4)])
        node2 = self.create_raw_node(1, [('r', 3)])             # HE
        node3 = self.create_raw_node(1, [])                     # HER
        node4 = self.create_raw_node(0, [('s', 5), ('m', 6)])
        node5 = self.create_raw_node(1, [])                     # HIS
        node6 = self.create_raw_node(1, [])                     # HIM
        node7 = self.create_raw_node(0, [('t', 8)])
        node8 = self.create_raw_node(1, [])                     # IT

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


    def test__construct_simple_trie__split_across_a_few_chunks(self):

        r"""
        trie for set {he, her, his, him, it}

        #0 -> [h #1 ] -> [e #2*] -> [r #3*]
         |           \-> [i #4 ] -> [s #5*]
         |                      \-> [m #6*]
         |
         +--> [i #7 ] -> [t #8 ]
        """
        values = ["HE", "HER", "HIS", "HIM", "IT"]

        node0 = self.create_raw_node(0, [('h', 1), ('i', 7)])
        node1 = self.create_raw_node(0, [('e', 2), ('i', 4)])
        node2 = self.create_raw_node(1, [('r', 3)])             # HE
        node3 = self.create_raw_node(1, [])                     # HER
        node4 = self.create_raw_node(0, [('s', 5), ('m', 6)])
        node5 = self.create_raw_node(1, [])                     # HIS
        node6 = self.create_raw_node(1, [])                     # HIM
        node7 = self.create_raw_node(0, [('t', 8)])
        node8 = self.create_raw_node(1, [])                     # IT

        self.count = 9
        self.raw   = [
            self.create_raw_count(2) + node0 + node1,
            self.create_raw_count(3) + node2 + node3 + node4,
            self.create_raw_count(1) + node5,
            self.create_raw_count(3) + node6 + node7 + node8
        ]
        self.kind  = ahocorasick.TRIE
        self.values = values
        self.word_count = 5

        A = self.create_automaton(USE_EXACT_RAW)
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

        node0 = self.create_raw_node(0, [('h', 1)])
        node1 = self.create_raw_node(1, [('e', 2)])    # expect python value
        node2 = self.create_raw_node(1, [])            # also python value

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

        node0 = self.create_raw_node(0, [('h', 1)])
        node1 = self.create_raw_node(0, [('e', 2)])
        node2 = self.create_raw_node(1, [])
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

        node0 = self.create_raw_node(0, [('?', 1)])
        node1 = self.create_raw_node(0, [('x', 16)]) # the second node point to non-existent node

        self.count = 2
        self.raw   = node0 + node1
        self.kind  = ahocorasick.TRIE

        with self.assertRaisesRegex(ValueError, "Node #1 malformed: next link #0 points to.*"):
            self.create_automaton()


    def test__malicious_fail_pointer(self):
        """
        trie with just one node
        """

        builder = self.create_node_builder(0, [])
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
            raw += self.create_raw_node(1, [])
            values.append(tuple("node %d" % i))

        # create the last node that will cause error -- malformed next pointer
        raw += self.create_raw_node(1, [('_', 10000)])
        values.append(tuple("never reached"))

        self.count  = good_nodes + 1
        self.raw    = raw
        self.kind   = ahocorasick.TRIE
        self.values = values

        with self.assertRaises(ValueError):
            self.create_automaton()


    def test__wrong_type_of_chunk_container(self):

        self.count = 9
        self.raw   = () # this shuld be a list
        self.kind  = ahocorasick.TRIE
        self.values = None
        self.word_count = 5

        with self.assertRaisesRegex(TypeError, "Expected list"):
            A = self.create_automaton(USE_EXACT_RAW)


    def test__wrong_type_of_chunk(self):

        self.count = 9
        self.raw   = [42] # list items must be strings/bytes
        self.kind  = ahocorasick.TRIE
        self.values = None
        self.word_count = 5

        with self.assertRaisesRegex(ValueError, "Item #0 on the bytes list is not a bytes object"):
            A = self.create_automaton(USE_EXACT_RAW)


    def test__wrong_count_of_nodes_in_chunk__case1(self):

        self.count = 9
        self.raw   = [
            self.create_raw_count(0) # count must be greater than 0
        ]
        self.kind  = ahocorasick.TRIE
        self.values = None
        self.word_count = 5

        with self.assertRaisesRegex(ValueError, r"Nodes count for item #0 on the bytes list is not positive \(0\)"):
            A = self.create_automaton(USE_EXACT_RAW)


    def test__wrong_count_of_nodes_in_chunk__case2(self):

        self.count = 9
        self.raw   = [
            self.create_raw_count(-12 & 0xffffffffffffffff) # count must be greater than 0
        ]
        self.kind  = ahocorasick.TRIE
        self.values = None
        self.word_count = 5

        with self.assertRaisesRegex(ValueError, r"Nodes count for item #0 on the bytes list is not positive \(-12\)"):
            A = self.create_automaton(USE_EXACT_RAW)



if __name__ == '__main__':
    print("WARNING: these tests deal with in-memory representation (see TreeNodeBuilder),")
    print("         they were meant to test low-level implementation of pickling.")
    print("         Might segfault on your machine which is not necessary a bug in pyahocorasick.")
    unittest.main()
