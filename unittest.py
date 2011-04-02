# -*- coding: iso-8859-2 -*-
"""
	This is part of pyahocorasick Python module.
	
	Unit tests

	Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl/proj/pyahocorasick/
	License   : public domain
	Date      : $Date$

	$Id$
"""

import unittest
import ahocorasick

class TestTrieStorePyObjectsBase(unittest.TestCase):
	def setUp(self):
		self.A = ahocorasick.Automaton();
		self.words = b"word python aho corasick \x00\x00\x00".split()
		self.inexisting = b"test foo bar dword".split()


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
			A.add_word(word, None)
			self.assertEqual(A.kind, ahocorasick.TRIE)
			self.assertEqual(len(A), n)

		# dupliacted entry
		A.add_word(self.words[0], None)
		self.assertTrue(A.kind == ahocorasick.TRIE)
		self.assertTrue(len(A) == n)


	def test_add_empty_word(self):
		self.assertFalse(self.A.add_word("", None))
		self.assertEqual(len(self.A), 0)
		self.assertEqual(self.A.kind, ahocorasick.EMPTY)


	def test_clear(self):
		A = self.A
		self.assertTrue(A.kind == ahocorasick.EMPTY)

		for w in self.words:
			A.add_word(w, w)

		self.assertEqual(len(A), len(self.words))

		A.clear()
		self.assertEqual(A.kind, ahocorasick.EMPTY)
		self.assertEqual(len(A), 0)


	def test_exists(self):
		A = self.A
		words = b"word python aho corasick \x00\x00\x00".split()
		for w in self.words:
			A.add_word(w, w)

		for w in self.words:
			self.assertTrue(A.exists(w))

		for w in self.inexisting:
			self.assertFalse(A.exists(w))


	def test_contains(self):
		A = self.A
		for w in self.words:
			A.add_word(w, w)

		for w in self.words:
			self.assertTrue(w in A)

		for w in self.inexisting:
			self.assertTrue(w not in A)


	def test_match(self):
		A = self.A
		for w in self.words:
			A.add_word(w, w)

		prefixes = b"w wo wor word p py pyt pyth pytho python \x00 \x00\x00 \x00\x00\x00".split()
		for w in prefixes:
			self.assertTrue(A.match(w))

		inexisting = b"wa apple pyTon \x00\x00\x00\x00".split()
		for w in inexisting:
			self.assertFalse(A.match(w))


	def test_get1(self):
		A = self.A
		for i, w in enumerate(self.words):
			A.add_word(w, i+1)

		for i, w in enumerate(self.words):
			self.assertEqual(A.get(w), i+1)


	def test_get2(self):
		A = self.A
		for i, w in enumerate(self.words):
			A.add_word(w, i+1)

		for w in self.inexisting:
			self.assertEqual(A.get(w, None), None)


	def test_get3(self):
		A = self.A
		for i, w in enumerate(self.words):
			A.add_word(w, i+1)

		for w in self.inexisting:
			with self.assertRaises(KeyError):
				A.get(w)

	
	def test_longest_prefix(self):
		A = self.A
		for i, w in enumerate(self.words):
			A.add_word(w, i+1)

		# there is "word"
		self.assertEqual(A.longest_prefix(b"wo"), 2)
		self.assertEqual(A.longest_prefix(b"working"), 3)
		self.assertEqual(A.longest_prefix(b"word"), 4)
		self.assertEqual(A.longest_prefix(b"wordbook"), 4)
		self.assertEqual(A.longest_prefix(b"void"), 0)
		self.assertEqual(A.longest_prefix(b""), 0)


	def test_stats1(self):
		A = self.A
		for i, w in enumerate(self.words):
			A.add_word(w, i+1)

		s = A.get_stats()
		print(s)
		self.assertTrue(len(s) > 0)


	def test_stats2(self):
		s = self.A.get_stats()
		print(s)
		self.assertTrue(len(s) > 0)
		for key in s:
			if key != "sizeof_node":
				self.assertEqual(s[key], 0)


class TestTrieIterators(TestTrieStorePyObjectsBase):
	"Test iterators walking over trie"

	def test_keys(self):
		A = self.A
		for i, w in enumerate(self.words):
			A.add_word(w, i+1)

		L = [word for word in A.keys()]
		K = self.words
		self.assertEqual(len(L), len(K))
		self.assertEqual(set(L), set(K))


	def test_values(self):
		A = self.A
		for i, w in enumerate(self.words):
			A.add_word(w, i+1)

		L = [x for x in A.values()]
		V = list(range(1, len(self.words)+1))
		self.assertEqual(len(L), len(V))
		self.assertEqual(set(L), set(V))


	def test_items(self):
		A = self.A
		I = []
		for i, w in enumerate(self.words):
			A.add_word(w, i+1)
			I.append((w, i+1))

		L = [x for x in A.items()]
		self.assertEqual(len(L), len(I))
		self.assertEqual(set(L), set(I))


	def test_items_with_prefix_valid(self):
		A = self.A
		words = b"he she her hers star ham".split()
		for word in words:
			A.add_word(word, word)

		I = b"he her hers".split()
		L = [x for x in A.keys(b"he")]
		self.assertEqual(len(L), len(I))
		self.assertEqual(set(L), set(I))


	def test_items_with_prefix_invalid(self):
		A = self.A
		words = b"he she her hers star ham".split()
		for word in words:
			A.add_word(word, word)

		I = []
		L = [x for x in A.keys(b"cat")]
		self.assertEqual(len(L), len(I))
		self.assertEqual(set(L), set(I))


class TestTrieIteratorsInvalidate(TestTrieStorePyObjectsBase):
	"Test invalidating iterator when trie is changed"

	def helper(self, method):
		A = self.A
		for i, w in enumerate(self.words):
			A.add_word(w, i+1)

		it = method()
		w  = next(it)
		A.add_word("should fail", 1)
		with self.assertRaises(ValueError):
			w = next(it)


	def test_keys(self):
		self.helper(self.A.keys)


	def test_values(self):
		self.helper(self.A.values)


	def test_items(self):
		self.helper(self.A.items)


class TestAutomatonBase(unittest.TestCase):
	def setUp(self):
		self.A = ahocorasick.Automaton();
		self.words  = b"he her hers she".split()
		self.string = b"_sherhershe_"
		self.correct_positons = [
			(3, b"she"),
			(3, b"he"),
			(4, b"her"),
			(6, b"he"),
			(7, b"her"),
			(8, b"hers"),
			(10, b"she"),
			(10, b"he")
		]


	def add_words(self):
		for word in self.words:
			self.A.add_word(word, word)


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


	def test_make_automaton2(self):
		A = self.A
		self.assertEqual(A.kind, ahocorasick.EMPTY)

		self.add_words()
		self.assertEqual(A.kind, ahocorasick.TRIE)

		A.make_automaton()
		self.assertEqual(A.kind, ahocorasick.AHOCORASICK)

		A.add_word("rollback?", True)
		self.assertEqual(A.kind, ahocorasick.TRIE)


class TestAutomatonSearch(TestAutomatonBase):
	"Test searching using constructed automaton (method find_all)"

	def test_find_all1(self):
		"no action is performed until automaton is constructed"
		A = self.A
		self.assertEqual(A.kind, ahocorasick.EMPTY)
		self.assertEqual(A.find_all(self.string, 'any arg'), None)

		A.add_word("word", None)
		self.assertEqual(A.kind, ahocorasick.TRIE)
		self.assertEqual(A.find_all(self.string, 'any arg'), None)


	def test_find_all2(self):
		A = self.add_words_and_make_automaton()

		L = []
		def callback(index, word):
			L.append((index, word))

		A.find_all(self.string, callback)

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
		A.find_all(self.string[start:end], callback)
		C = [(pos+start, word) for pos, word in L]

		L = []
		A.find_all(self.string, callback, start, end)

		self.assertEqual(L, C)


class TestAutomatonIterSearch(TestAutomatonBase):
	"Test searching using constructed automaton (iterator)"

	def test_iter1(self):
		A = self.A
		self.assertEqual(A.kind, ahocorasick.EMPTY)
		with self.assertRaises(AttributeError):
			A.iter(self.string)

		A.add_word("word", None)
		self.assertEqual(A.kind, ahocorasick.TRIE)
		with self.assertRaises(AttributeError):
			A.iter(self.string)


	def test_iter2(self):
		A = self.add_words_and_make_automaton()

		L = []
		for index, word in A.iter(self.string):
			L.append((index, word))

		C = self.correct_positons
		self.assertEqual(L, C)


	def test_iter3(self):
		A = self.add_words_and_make_automaton()

		start = 4
		end = 9

		C = []
		for index, word in A.iter(self.string[start:end]):
			C.append((index + start, word))

		L = []
		for index, word in A.iter(self.string, start, end):
			L.append((index, word))

		self.assertEqual(L, C)


	def test_iter_set(self):
		A = self.add_words_and_make_automaton()
		parts = b"_sh erhe rshe _".split()

		it = A.iter(b"")
		print()
		for part in parts:
			it.set(part)
			print(part, ":")
			for item in it:
				print(item)


	def test_iter_compare_with_find_all(self):
		A = self.add_words_and_make_automaton()

		# results from find_all
		L = []
		def callback(index, word):
			L.append((index, word))

		A.find_all(self.string, callback)

		# results from iterator
		C = []
		for index, word in A.iter(self.string):
			C.append((index, word))

		self.assertEqual(L, C)


class TestAutomatonIterInvalidate(TestAutomatonBase):
	"Test if searching iterator is invalidated when trie/automaton change"

	def test_iter1(self):
		A = self.add_words_and_make_automaton()

		it = A.iter(self.string)
		w  = next(it)
		A.add_word("should fail", 1)
		with self.assertRaises(ValueError):
			w = next(it)


	def test_iter2(self):
		A = self.add_words_and_make_automaton()

		it = A.iter(self.string)
		w  = next(it)
		A.clear()
		with self.assertRaises(ValueError):
			w = next(it)


print_dumps = False

class TestPickle(TestAutomatonBase):
	"Test pickling/unpickling"

	def test_pickle(self):
		import pickle

		A = self.add_words_and_make_automaton();
		if print_dumps:
			print(pickle.dumps(A))


	def test_unpickle(self):
		import pickle
		A = self.add_words_and_make_automaton();
		dump = pickle.dumps(A)
		B = pickle.loads(dump)

		if print_dumps:
			print([x for x in B.items()])
			print([x for x in B.iter(self.string)])

		self.assertEqual(len(A), len(B))

		AL = set(key for key in A.keys())
		BL = set(key for key in B.keys())
		self.assertEqual(AL, BL)


class TestTrieStoreInts(unittest.TestCase):
	"Test storing plain ints as values (instead of python objects)"

	def setUp(self):
		self.A = ahocorasick.Automaton(ahocorasick.STORE_INTS);
		self.words = b"word python aho corasick \x00\x00\x00".split()


	def test_add_word1(self):
		A = self.A

		# by default next values are stored
		for word in self.words:
			A.add_word(word)

		I = list(range(1, len(self.words) + 1))
		L = [A.get(word) for word in self.words]
		self.assertEqual(I, L)


	def test_add_word2(self):
		A = self.A

		# store arbitrary ints
		for i, word in enumerate(self.words):
			A.add_word(word, i + 123)

		I = list(range(123, 123 + len(self.words)))
		L = [A.get(word) for word in self.words]
		self.assertEqual(I, L)


	def test_add_word3(self):
		# not a number
		with self.assertRaises(TypeError):
			self.A.add_word(b"xyz", None)

	
	def test_iter(self):
		A = self.A
		for word in self.words:
			A.add_word(word);

		I = set(range(1, len(A) + 1))
		L1 = [val for val in A.values()]
		L2 = [val for key, val in A.items()]

		self.assertEqual(L1, L2)
		self.assertEqual(set(L1), I)


	def test_find_all_and_iter(self):
		words  = b"he her hers she".split()
		string = b"_sherhershe_"

		A = self.A
		for word in words:
			A.add_word(word)

		A.make_automaton()

		# find_all()
		C = []
		def callback(index, value):
			C.append((index, value))
		
		A.find_all(string, callback);

		# iter()
		L = [(index, value) for index, value in A.iter(string)]

		#
		self.assertEqual(C, L)


class TestTrieStoreLengths(unittest.TestCase):
	"""Test storing plain ints -- length of words --- as values
	(instead of python objects)"""

	def setUp(self):
		self.A = ahocorasick.Automaton(ahocorasick.STORE_LENGTH);
		self.words = b"word python aho corasick \x00\x00\x00".split()


	def test_add_word1(self):
		A = self.A

		# by default next values are stored
		for word in self.words:
			A.add_word(word)

		for key, value in A.items():
			self.assertEqual(len(key), value)


if __name__ == '__main__':
	unittest.main()
