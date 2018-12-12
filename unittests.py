# -*- coding: utf-8 -*-
"""
    This is part of pyahocorasick Python module.

    Unit tests for the C-based ahocorasick module.

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl/proj/pyahocorasick/
    License   : public domain
"""

import sys
import os
import unittest
import ahocorasick

try:
    import _pickle
except ImportError:
    _pickle = None


if ahocorasick.unicode:
    conv = lambda x: x
else:
    if sys.version_info.major >= 3:
        conv = lambda x: bytes(x, 'ascii')
    else:
        conv = lambda x: x


class TestCase(unittest.TestCase):
    def __init__(self, *args):
        super(TestCase, self).__init__(*args)

        if not hasattr(self, 'assertRaisesRegex'):
            # fixup for Py2
            self.assertRaisesRegex = self.assertRaisesRegexp


    def assertEmpty(self, collection):
        self.assertEqual(0, len(collection))


    def assertNotEmpty(self, collection):
        self.assertGreater(len(collection), 0)


class TestConstructor(TestCase):
    def test_constructor_wrong_key_type(self):
        with self.assertRaisesRegex(ValueError, "key_type must have value.*"):
            ahocorasick.Automaton(-42)


class TestTrieStorePyObjectsBase(TestCase):
    def setUp(self):
        self.A = ahocorasick.Automaton();
        self.words = "word python aho corasick \x00\x00\x00".split()
        self.inexisting = "test foo bar dword".split()


class TestTrieMethods(TestTrieStorePyObjectsBase):
    "Test basic methods related to trie structure"

    def test_empty(self):
        A = self.A
        self.assertTrue(A.kind == ahocorasick.EMPTY)
        self.assertTrue(len(A) == 0)


    def test_add_word(self):
        A = self.A
        self.assertTrue(A.kind == ahocorasick.EMPTY)

        n = 0
        for word in self.words:
            n += 1
            A.add_word(conv(word), None)
            self.assertEqual(A.kind, ahocorasick.TRIE)
            self.assertEqual(len(A), n)

        # dupliacted entry
        A.add_word(conv(self.words[0]), None)
        self.assertTrue(A.kind == ahocorasick.TRIE)
        self.assertTrue(len(A) == n)


    def test_add_empty_word(self):
        if ahocorasick.unicode:
            self.assertFalse(self.A.add_word("", None))
        else:
            self.assertFalse(self.A.add_word(b"", None))

        self.assertEqual(len(self.A), 0)
        self.assertEqual(self.A.kind, ahocorasick.EMPTY)


    def test_clear(self):
        A = self.A
        self.assertTrue(A.kind == ahocorasick.EMPTY)

        for w in self.words:
            A.add_word(conv(w), w)

        self.assertEqual(len(A), len(self.words))

        A.clear()
        self.assertEqual(A.kind, ahocorasick.EMPTY)
        self.assertEqual(len(A), 0)


    def test_exists(self):
        A = self.A

        for w in self.words:
            A.add_word(conv(w), w)

        for w in self.words:
            self.assertTrue(A.exists(conv(w)))

        for w in self.inexisting:
            self.assertFalse(A.exists(conv(w)))


    def test_contains(self):
        A = self.A
        for w in self.words:
            A.add_word(conv(w), w)

        for w in self.words:
            self.assertTrue(conv(w) in A)

        for w in self.inexisting:
            self.assertTrue(conv(w) not in A)


    def test_match(self):
        A = self.A
        for word in self.words:
            A.add_word(conv(word), word)

        prefixes = "w wo wor word p py pyt pyth pytho python \x00 \x00\x00 \x00\x00\x00".split()

        for word in prefixes:
            self.assertTrue(A.match(conv(word)))

        inexisting = "wa apple pyTon \x00\x00\x00\x00".split()
        for word in inexisting:
            self.assertFalse(A.match(conv(word)))


    def test_get1(self):
        A = self.A
        for i, w in enumerate(self.words):
            A.add_word(conv(w), i + 1)

        for i, w in enumerate(self.words):
            self.assertEqual(A.get(conv(w)), i + 1)


    def test_get2(self):
        A = self.A
        for i, w in enumerate(self.words):
            A.add_word(conv(w), i + 1)

        for w in self.inexisting:
            self.assertEqual(A.get(conv(w), None), None)


    def test_get3(self):
        A = self.A
        for i, w in enumerate(self.words):
            A.add_word(conv(w), i + 1)

        for w in self.inexisting:
            with self.assertRaises(KeyError):
                A.get(conv(w))


    def test_get_from_an_empty_automaton(self):
        A = ahocorasick.Automaton()

        r = A.get('foo', None)
        self.assertEqual(r, None)


    def test_longest_prefix(self):
        A = self.A
        for i, w in enumerate(self.words):
            A.add_word(conv(w), i + 1)

        # there is "word"
        self.assertEqual(A.longest_prefix(conv("wo")), 2)
        self.assertEqual(A.longest_prefix(conv("working")), 3)
        self.assertEqual(A.longest_prefix(conv("word")), 4)
        self.assertEqual(A.longest_prefix(conv("wordbook")), 4)
        self.assertEqual(A.longest_prefix(conv("void")), 0)
        self.assertEqual(A.longest_prefix(conv("")), 0)


    def test_stats_have_valid_structure(self):
        A = self.A
        for i, w in enumerate(self.words):
            A.add_word(conv(w), i + 1)

        platform_dependent = None
        reference = {
            'longest_word': 8,
            'total_size': platform_dependent,
            'sizeof_node': platform_dependent,
            'nodes_count': 25,
            'words_count': 5,
            'links_count': 24
        }

        s = A.get_stats()

        self.assertEqual(len(s), len(reference))

        for key in reference:
            self.assertIn(key, s)

        for key in (key for key in reference if reference[key] != platform_dependent):
            self.assertEqual(reference[key], s[key])


    def test_stats_for_empty_tire_are_empty(self):
        s = self.A.get_stats()
        self.assertTrue(len(s) > 0)
        for key in s:
            if key != "sizeof_node":
                self.assertEqual(s[key], 0)


class TestTrieIterators(TestTrieStorePyObjectsBase):
    "Test iterators walking over trie"


    def test_iter(self):
        A = self.A
        for i, w in enumerate(self.words):
            A.add_word(conv(w), i + 1)

        L = [word for word in A]
        K = list(map(conv, self.words))
        self.assertEqual(len(L), len(K))
        self.assertEqual(set(L), set(K))


    def test_keys(self):
        A = self.A
        for i, w in enumerate(self.words):
            A.add_word(conv(w), i + 1)

        L = [word for word in A.keys()]
        K = [conv(word) for word in self.words]
        self.assertEqual(len(L), len(K))
        self.assertEqual(set(L), set(K))


    def test_values(self):
        A = self.A
        for i, w in enumerate(self.words):
            A.add_word(conv(w), i + 1)

        L = [x for x in A.values()]
        V = list(range(1, len(self.words) + 1))
        self.assertEqual(len(L), len(V))
        self.assertEqual(set(L), set(V))


    def test_items(self):
        A = self.A
        I = []
        for i, w in enumerate(self.words):
            A.add_word(conv(w), i + 1)
            I.append((conv(w), i + 1))

        L = [x for x in A.items()]
        self.assertEqual(len(L), len(I))
        self.assertEqual(set(L), set(I))


    def test_items_with_prefix_valid(self):
        A = self.A
        words = "he she her hers star ham".split()
        for word in words:
            A.add_word(conv(word), word)

        I = list(map(conv, "he her hers".split()))
        L = [x for x in A.keys(conv("he"))]
        self.assertEqual(len(L), len(I))
        self.assertEqual(set(L), set(I))


    def test_items_with_prefix_invalid(self):
        A = self.A
        words = "he she her hers star ham".split()
        for word in words:
            A.add_word(conv(word), word)

        I = []
        L = [x for x in A.keys(conv("cat"))]
        self.assertEqual(len(L), len(I))
        self.assertEqual(set(L), set(I))


    def test_items_with_valid_pattern(self):
        A = self.A
        words = "abcde aXcd aZcdef aYc Xbcdefgh".split()
        for word in words:
            A.add_word(conv(word), word)

        I = ["aXcd"]
        L = [x for x in A.keys(conv("a?cd"), conv("?"))]
        self.assertEqual(set(I), set(L))


    def test_items_with_valid_pattern2(self):
        A = self.A
        words = "abcde aXcde aZcdef aYc Xbcdefgh".split()
        for word in words:
            A.add_word(conv(word), word)

        L = [x for x in A.keys(conv("a?c??"), conv("?"), ahocorasick.MATCH_EXACT_LENGTH)]
        I = ["abcde", "aXcde"]
        self.assertEqual(set(I), set(L))

        L = [x for x in A.keys(conv("a?c??"), conv("?"), ahocorasick.MATCH_AT_MOST_PREFIX)]
        I = ["aYc", "abcde", "aXcde"]
        self.assertEqual(set(I), set(L))

        L = [x for x in A.keys(conv("a?c??"), conv("?"), ahocorasick.MATCH_AT_LEAST_PREFIX)]
        I = ["abcde", "aXcde", "aZcdef"]
        self.assertEqual(set(I), set(L))


    def test_items_wrong_wildcrard(self):
        with self.assertRaisesRegex(ValueError, "Wildcard must be a single character.*"):
            self.A.keys(conv("anything"), conv("??"))


    def test_items_wrong_match_enum(self):
        with self.assertRaisesRegex(ValueError, "The optional how third argument must be one of"):
            self.A.keys(conv("anything"), conv("?"), -42)


class TestTrieIteratorsInvalidate(TestTrieStorePyObjectsBase):
    "Test invalidating iterator when trie is changed"

    def helper(self, method):
        A = self.A
        for i, w in enumerate(self.words):
            A.add_word(conv(w), i + 1)

        it = method()
        w = next(it)
        # word already exists, just change associated value
        # iterator is still valid
        A.add_word(conv(self.words[0]), 2)
        w = next(it)

        # new word, iterator is invalidated
        A.add_word(conv("should fail"), 1)
        with self.assertRaises(ValueError):
            w = next(it)


    def test_keys(self):
        self.helper(self.A.keys)


    def test_values(self):
        self.helper(self.A.values)


    def test_items(self):
        self.helper(self.A.items)


class TestAutomatonBase(TestCase):
    def setUp(self):
        self.A = ahocorasick.Automaton();
        self.words = "he her hers she".split()
        self.string = "_sherhershe_"
        self.correct_positons = [
            (3, "she"),
            (3, "he"),
            (4, "her"),
            (6, "he"),
            (7, "her"),
            (8, "hers"),
            (10, "she"),
            (10, "he")
        ]


    def add_words(self):
        for word in self.words:
            self.A.add_word(conv(word), word)


    def add_words_and_make_automaton(self):
        self.add_words()
        self.A.make_automaton()
        return self.A


class TestAutomatonConstruction(TestAutomatonBase):
    "Test converting trie to Aho-Corasick automaton"

    def test_make_automaton1(self):
        A = self.A
        self.assertEqual(A.kind, ahocorasick.EMPTY)
        A.make_automaton()
        # empty trie is never converted to automaton
        self.assertEqual(A.kind, ahocorasick.EMPTY)


    def test_make_automaton2(self):
        A = self.A
        self.assertEqual(A.kind, ahocorasick.EMPTY)

        self.add_words()
        self.assertEqual(A.kind, ahocorasick.TRIE)

        A.make_automaton()
        self.assertEqual(A.kind, ahocorasick.AHOCORASICK)


    def test_make_automaton3(self):
        A = self.A
        self.assertEqual(A.kind, ahocorasick.EMPTY)

        self.add_words()
        self.assertEqual(A.kind, ahocorasick.TRIE)

        A.make_automaton()
        self.assertEqual(A.kind, ahocorasick.AHOCORASICK)

        A.add_word(conv("rollback?"), True)
        self.assertEqual(A.kind, ahocorasick.TRIE)


class TestAutomatonSearch(TestAutomatonBase):
    "Test searching using constructed automaton (method find_all)"

    def test_find_all1(self):
        "no action is performed until automaton is constructed"
        A = self.A
        self.assertEqual(A.kind, ahocorasick.EMPTY)

        self.assertEqual(A.find_all(self.string, conv("any arg")), None)

        A.add_word(conv("word"), None)
        self.assertEqual(A.kind, ahocorasick.TRIE)
        self.assertEqual(A.find_all(self.string, conv("any arg")), None)


    def test_find_all2(self):
        A = self.add_words_and_make_automaton()

        L = []
        def callback(index, word):
            L.append((index, word))

        A.find_all(conv(self.string), callback)

        C = self.correct_positons
        self.assertEqual(L, C)


    def test_find_all3(self):
        A = self.add_words_and_make_automaton()

        L = []
        def callback(index, word):
            L.append((index, word))

        start = 4
        end = 9

        L = []
        A.find_all(conv(self.string[start:end]), callback)
        C = [(pos + start, word) for pos, word in L]

        L = []
        A.find_all(conv(self.string), callback, start, end)

        self.assertEqual(L, C)


    def test_find_all__not_a_callable_object(self):
        A = self.add_words_and_make_automaton()

        with self.assertRaisesRegex(TypeError, "The callback argument must be a callable such as a function."):
            A.find_all(conv(self.string), None)


    def test_find_all__wrong_range__case_1(self):
        A = self.add_words_and_make_automaton()

        L = []
        def callback(index, word):
            L.append((index, word))

        with self.assertRaisesRegex(IndexError, "end index not in range 0..12"):
            A.find_all(conv(self.string), callback, 0, len(self.string) + 5)


    def test_find_all__wrong_range__case_2(self):
        A = self.add_words_and_make_automaton()

        L = []
        def callback(index, word):
            L.append((index, word))

        with self.assertRaisesRegex(IndexError, "start index not in range 0..12"):
            A.find_all(conv(self.string), callback, -len(self.string) - 1, 3)


    def test_find_all__end_index_not_given(self):
        A = self.add_words_and_make_automaton()

        L = []
        def callback(index, word):
            L.append((index, word))

        A.find_all(conv(self.string), callback, 0)


    def test_find_all__start_is_negative(self):
        A = self.add_words_and_make_automaton()

        L = []
        def callback(index, word):
            L.append((index, word))

        A.find_all(conv(self.string), callback, -3, 4)


    def test_find_all__end_is_negative(self):
        A = self.add_words_and_make_automaton()

        L = []
        def callback(index, word):
            L.append((index, word))

        A.find_all(conv(self.string), callback, 0, -1)


class TestAutomatonIterSearch(TestAutomatonBase):
    "Test searching using constructed automaton (iterator)"

    def test_iter1(self):
        A = self.A
        self.assertEqual(A.kind, ahocorasick.EMPTY)
        with self.assertRaises(AttributeError):
            A.iter(conv(self.string))

        A.add_word(conv("word"), None)
        self.assertEqual(A.kind, ahocorasick.TRIE)
        with self.assertRaises(AttributeError):
            A.iter(conv(self.string))


    def test_iter2(self):
        A = self.add_words_and_make_automaton()

        L = []
        for index, word in A.iter(conv(self.string)):
            L.append((index, word))

        C = self.correct_positons
        self.assertEqual(L, C)


    def test_iter3(self):
        A = self.add_words_and_make_automaton()

        start = 4
        end = 9

        C = []
        for index, word in A.iter(conv(self.string[start:end])):
            C.append((index + start, word))

        L = []
        for index, word in A.iter(conv(self.string), start, end):
            L.append((index, word))

        self.assertEqual(L, C)


    def test_iter_set(self):
        A = self.add_words_and_make_automaton()
        parts = "_sh erhe rshe _".split()

        expected = {
            '_sh'   : [],
            'erhe'  : [(3, 'she'),
                       (3, 'he'),
                       (4, 'her'),
                       (6, 'he')],
            'rshe'  : [(7, 'her'),
                       (8, 'hers'),
                       (10, 'she'),
                       (10, 'he')],
             '_'    : []
        }

        it = A.iter(conv(""))
        result = {}
        for part in parts:
            it.set(conv(part))
            result[part] = []
            for item in it:
                result[part].append(item)

        self.assertEqual(expected, result)

    def test_iter_set__with_reset(self):
        A = self.add_words_and_make_automaton()

        expected = {
            'he'    : [(1, 'he')],
            'she'   : [(2, 'she'), (2, 'he')],
        }

        it = A.iter(conv(""))
        result = {}
        for part in ["he", "she"]:
            it.set(conv(part), True)
            result[part] = []
            for item in it:
                result[part].append(item)

        self.assertEqual(expected, result)


    def test_iter_compare_with_find_all(self):
        A = self.add_words_and_make_automaton()

        # results from find_all
        L = []
        def callback(index, word):
            L.append((index, word))

        A.find_all(conv(self.string), callback)

        # results from iterator
        C = []
        for index, word in A.iter(conv(self.string)):
            C.append((index, word))

        self.assertEqual(L, C)


    def test_iter_wrong_argument_type(self):
        A = self.add_words_and_make_automaton()

        with self.assertRaisesRegex(TypeError, "string required"):
            A.iter(None)


class TestAutomatonIterSearchWithIgnoreWhiteSpace(TestAutomatonBase):
    "Test searching using constructed automaton (iterator)"

    def setUp(self):
        self.A = ahocorasick.Automaton()
        self.words = "he her hers she".split()
        self.string = "_sh e rher she_"
        self.correct_positons = [
            (4, "she"),
            (4, "he"),
            (6, "her"),
            (8, "he"),
            (9, "her"),
            (11, "hers"),
            (13, "she"),
            (13, "he")
        ]
        self.correct_positons_start_12 = [
            (13, "he")
        ]


    def test_iter1(self):
        self.add_words_and_make_automaton()
        A = self.A
        self.assertEqual(A.kind, ahocorasick.AHOCORASICK)

        L = []
        for index, word in A.iter(conv(self.string), ignore_white_space=True):
            L.append((index, word))
        self.assertEqual(L, self.correct_positons)


    def test_iter2(self):
        self.add_words_and_make_automaton()
        A = self.A
        self.assertEqual(A.kind, ahocorasick.AHOCORASICK)

        L = []
        for index, word in A.iter(conv(self.string), ignore_white_space=True, start=12):
            L.append((index, word))
        self.assertEqual(L, self.correct_positons_start_12)


    def test_wrong_keyword(self):
        self.add_words_and_make_automaton()
        A = self.A
        self.assertEqual(A.kind, ahocorasick.AHOCORASICK)

        with self.assertRaises(TypeError):
            A.iter(conv(self.string), ignore_white_space2=True)


class TestAutomatonIterInvalidate(TestAutomatonBase):
    "Test if searching iterator is invalidated when trie/automaton change"

    def test_iter1(self):
        A = self.add_words_and_make_automaton()

        it = A.iter(conv(self.string))
        w = next(it)
        A.add_word(conv("should fail"), 1)
        with self.assertRaises(ValueError):
            w = next(it)


    def test_iter2(self):
        A = self.add_words_and_make_automaton()

        it = A.iter(conv(self.string))
        w = next(it)
        A.clear()
        with self.assertRaises(ValueError):
            w = next(it)


print_dumps = False

class TestPickle(TestAutomatonBase):
    "Test pickling/unpickling"

    def test_pickle(self):
        import pickle

        A = self.add_words_and_make_automaton();
        reduced = A.__reduce__()
        self.assertEqual(len(reduced), 2)
        if print_dumps:
            print(pickle.dumps(A))


    def test_unpickle(self):
        import pickle
        A = self.add_words_and_make_automaton();
        dump = pickle.dumps(A)
        B = pickle.loads(dump)

        self.compare_automatons(A, B)


    def test_unicode(self):
        # sample Russian words from issue #8
        import pickle

        test_sentences_rus = ["!ASM Print",
        "!ASM Print, tyre компания er",
        "!ASM Print, рекламно-производственная компания rr",
        "!Action Pact!",
        "!T.O.O.H.!",
        "!YES, лингвистический центр",
        "!ts, магазин",
        "!ФЕСТ",
        '"100-th" department store',
        '"1000 мелочей"',
        '"1001 мелочь"',
        '"19 отряд Федеральной противопожарной службы по Ленинградской области"',
        '"У Друзей"',
        '"ШТОРЫ и не только..."']

        A = ahocorasick.Automaton()
        for sentences in test_sentences_rus[-7:]:
            for index, word in enumerate(sentences.split(' ')):
                A.add_word(word, (index, word))

        dump = pickle.dumps(A)
        B = pickle.loads(dump)

        self.compare_automatons(A, B)


    def test_empty(self):
        import pickle

        A = ahocorasick.Automaton()
        dump = pickle.dumps(A)
        B = pickle.loads(dump)

        self.compare_automatons(A, B)


    def compare_automatons(self, A, B):
        if print_dumps:
            print([x for x in B.items()])
            print([x for x in A.items()])

        self.assertEqual(len(A), len(B))

        for item in zip(A.items(), B.items()):
            (AK, AV), (BK, BV) = item

            self.assertEqual(AK, BK)
            self.assertEqual(AV, BV)


class TestSizeOf(TestCase):
    def setUp(self):
        self.A = ahocorasick.Automaton();
        words = "word python aho corasick tree bark branch root".split()
        for word in words:
            self.A.add_word(conv(word), 1)


    def test_sizeof(self):
        size1 = sys.getsizeof(self.A)

        # grow memory
        self.A.add_word("kitten", "fluffy")

        size2 = sys.getsizeof(self.A)

        # just change the assigned value, no changes to the trie structure
        self.A.add_word("word", "other value")

        size3 = sys.getsizeof(self.A)

        self.assertTrue(size2 > size1)
        self.assertTrue(size3 == size2)


class TestBugAutomatonSearch(TestAutomatonBase):
    """Bug in search"""

    def setUp(self):
        self.A = ahocorasick.Automaton()
        self.words = ['GT-C3303', 'SAMSUNG-GT-C3303K/']


    def test_bug(self):
        self.add_words_and_make_automaton()
        text = 'SAMSUNG-GT-C3303i/1.0 NetFront/3.5 Profile/MIDP-2.0 Configuration/CLDC-1.1'

        res = list(self.A.iter(conv(text)))

        self.assertEqual([(15, 'GT-C3303')], res)


class TestIntSequenceBase(TestCase):
    def setUp(self):
        self.A = ahocorasick.Automaton(ahocorasick.KEY_SEQUENCE);


class TestIntSequence__TrieMethods(TestIntSequenceBase):

    def test_add__case_1(self):
        A = self.A

        ret = A.add_word((1, 2, 3), "foo")
        self.assertTrue(ret)
        self.assertTrue(A.kind == ahocorasick.TRIE)

        self.assertEqual(len(A), 1)
        self.assertTrue((1, 2, 3) in A)


    def test_add__case_2(self):
        A = self.A

        A.add_word((1, 2, 3), "foo")
        ret = A.add_word((1, 2, 3), "bar")
        self.assertFalse(ret)


    def test_add__case_3(self):
        A = self.A

        A.add_word((1, 2, 3), "foo")
        A.add_word((1, 2, 3, 4, 5), "bar")
        A.add_word((1, 3, 4, 5), "baz")

        self.assertEqual(len(A), 3);
        self.assertEqual(A.get((1, 2, 3)),          "foo");
        self.assertEqual(A.get((1, 2, 3, 4, 5)),    "bar");
        self.assertEqual(A.get((1, 3, 4, 5)),       "baz");


    def test_add__case_4(self):
        A = self.A

        ret = A.add_word((), "foo")
        self.assertFalse(ret)


    def test_add__case_5__wrong_argument_type(self):
        A = self.A

        with self.assertRaises(TypeError) as e:
            A.add_word("hello!", "foo")

        self.assertEqual(str(e.exception), "argument is not a supported sequence type")


    def test_add__case_6__wrong_item_type(self):
        A = self.A

        with self.assertRaises(ValueError) as e:
            A.add_word((1, 2, "hello!"), "foo")

        self.assertEqual(str(e.exception), "item #2 is not a number")


    def test_add__case_7__wrong_value(self):
        A = self.A

        with self.assertRaises(ValueError) as e:
            A.add_word((1, -1, 12), "foo")

        errmsg = str(e.exception)
        msgs = [
            "item #1: value -1 outside range [0..65535]",
            "item #1: value -1 outside range [0..4294967295]",
        ]

        self.assertIn(errmsg, msgs)


    def test_add__case_8__wrong_value(self):
        A = self.A

        with self.assertRaises(ValueError) as e:
            A.add_word((2**42, 0, 12), "foo")

        # Depending on python's version the message might be different,
        # but the type remains the same.

        errmsg = str(e.exception)
        msgs = [
            "item #0: value 4398046511104 outside range [0..65535]",
            "item #0: value 4398046511104 outside range [0..4294967295]",
            "item #0 is not a number",
        ]

        self.assertIn(errmsg, msgs)


    def test_match(self):
        A = self.A

        ret = A.add_word((1, 2, 3), "foo")
        self.assertTrue(A.match((1,)))
        self.assertTrue(A.match((1, 2)))
        self.assertTrue(A.match((1, 2, 3)))


    def test_longest_prefix(self):
        A = self.A

        ret = A.add_word((1, 2, 3, 4, 5, 6), "foo")
        self.assertEqual(A.longest_prefix((1, 2, 3, 111, 1111, 11111)), 3);
        self.assertEqual(A.longest_prefix((111, 1111, 11111)), 0);

    def test_iter1(self):
        A = self.A

        A.add_word((1, 2, 3), "foo")
        A.add_word((2, 3, 4, 5), "bar")
        A.add_word((2, 3, 5), "baz")
        A.make_automaton()

        L = [(index, value) for index, value in A.iter((1, 2, 3, 5))]

        self.assertEqual(L, [
            (2, "foo"),
            (3, "baz"),
        ])

    def test_iter2(self):
        A = self.A

        A.add_word((43, 89), (43, 89))
        A.add_word((43, 89, 64), (43, 89, 64))
        A.add_word((89, 64), (89, 64))
        A.add_word((89, 100), (89, 100))
        A.make_automaton()

        L = [
            (index, value)
            for index, value in
            A.iter((80, 80, 43, 89, 90, 89, 64, 100, 43, 89, 100))
        ]

        self.assertEqual(L, [
            (3, (43, 89)),
            (6, (89, 64)),
            (9, (43, 89)),
            (10, (89, 100)),
        ])


    def test_iter_wrong_argument_type(self):
        A = self.A
        A.add_word((89, 100), (89, 100))
        A.make_automaton()

        with self.assertRaisesRegex(TypeError, "tuple required"):
            self.A.iter(None)


class TestDump(TestAutomatonBase):
    def test_dump_empty(self):
        self.assertIsNone(self.A.dump())


    def test_dump_trie(self):
        self.add_words()
        ret = self.A.dump()

        self.assertEqual(3, len(ret))
        self.assertNotEmpty(ret[0])     # list of nodes
        self.assertNotEmpty(ret[1])     # list of edges
        self.assertEmpty(ret[2])        # list of fail links -- empty, if not an automaton


    def test_dump_automaton(self):
        self.add_words_and_make_automaton()
        ret = self.A.dump()

        self.assertEqual(3, len(ret))
        self.assertNotEmpty(ret[0])     # list of nodes
        self.assertNotEmpty(ret[1])     # list of edges
        self.assertNotEmpty(ret[2])     # list of fail links


class TestIssue53(TestCase):
    """
    Problems with handling of UCS-2 encoding
    """

    def test_case1(self):
        # test contributed by @woakesd (David Woakes)

        a = ahocorasick.Automaton()
        a.add_word('test', 'test')

        a.make_automaton()

        test_string = 'test 🙈 test?!'

        # wrongly calculated matching position
        for item in a.iter(test_string):
            start = item[0] - len(item[1]) + 1
            match = test_string[start:item[0] + 1]
            self.assertEqual(match, "test")


    def test_case2(self):
        a = ahocorasick.Automaton()
        a.add_word('test', 'test')

        a.make_automaton()

        test_string = '🙈' * 1000

        # wrongly calculated the input's length
        for item in a.iter(test_string):
            pass


class TestIssue68(TestCase):
    """
    Test problems with pickling
    """

    def test_case1(self):
        if _pickle is None:
            print("module _pickle not available")
            return

        A = ahocorasick.Automaton()
        for i in range(0, 65):
            A.add_word(str(i), (i, i))

        path = 'TestIssue68.test_case1'
        with open(path, 'wb') as f:
            _pickle.dump(A, f)

        with open(path, 'rb') as f:
            _pickle.load(f)

        try:
            os.unlink(path)
        except:
            pass


if __name__ == '__main__':
    unittest.main()

