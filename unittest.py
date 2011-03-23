import unittest
import ahocorasick

class TestTrieStorePyObjectsBase(unittest.TestCase):
	def setUp(self):
		self.A = ahocorasick.Automaton();
		self.words = b"word python aho corasick \x00\x00\x00".split()
		self.inexisting = b"test foo bar dword".split()


class TestTrieMethods(TestTrieStorePyObjectsBase):
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


	def test_clear(self):
		A = self.A
		self.assertTrue(A.kind == ahocorasick.EMPTY)
		
		for w in self.words:
			A.add_word(w, w)

		self.assertEqual(len(A), len(self.words))

		A.clear()
		self.assertEqual(A.kind, ahocorasick.EMPTY)
		self.assertEqual(len(A), 0)


	def test_match(self):
		A = self.A
		words = b"word python aho corasick \x00\x00\x00".split()
		for w in self.words:
			A.add_word(w, w)

		for w in self.words:
			self.assertTrue(A.match(w))

		for w in self.inexisting:
			self.assertFalse(A.match(w))


	def test_contains(self):
		A = self.A
		for w in self.words:
			A.add_word(w, w)

		for w in self.words:
			self.assertTrue(w in A)
		
		for w in self.inexisting:
			self.assertTrue(w not in A)


	def test_match_prefix(self):
		A = self.A
		for w in self.words:
			A.add_word(w, w)

		prefixes = b"w wo wor word p py pyt pyth pytho python \x00 \x00\x00 \x00\x00\x00".split()
		for w in prefixes:
			self.assertTrue(A.match_prefix(w))
		
		inexisting = b"wa apple pyTon \x00\x00\x00\x00".split()
		for w in inexisting:
			self.assertFalse(A.match_prefix(w))


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


class TestTrieIterators(TestTrieStorePyObjectsBase):
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


class TestTrieIteratorsInvalidate(TestTrieStorePyObjectsBase):
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
			(3, b'she'),
			(3, b'he'),
			(4, b'her'),
			(6, b'he'),
			(7, b'her'),
			(8, b'hers'),
			(10, b'she'),
			(10, b'he')
		]

	def add_words(self):
		for word in self.words:
			self.A.add_word(word, word)


	def add_words_and_make_automaton(self):
		self.add_words()
		self.A.make_automaton()


class TestAutomatonConstruction(TestAutomatonBase):

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

		A.add_word(b"rollback?", True)
		self.assertEqual(A.kind, ahocorasick.TRIE)

	
class TestAutomatonSearch(TestAutomatonBase):

	def test_search_all1(self):
		"not action is performed until automaton is constructed"
		A = self.A
		self.assertEqual(A.kind, ahocorasick.EMPTY)
		self.assertEqual(A.search_all(self.string, 'any arg'), None)
		
		A.add_word(b"word", None)
		self.assertEqual(A.kind, ahocorasick.TRIE)
		self.assertEqual(A.search_all(self.string, 'any arg'), None)


	def test_search_all2(self):
		A = self.A
		self.add_words_and_make_automaton()

		L = []
		def callback(index, word):
			L.append((index, word))

		A.search_all(self.string, callback)

		C = self.correct_positons
		self.assertEqual(L, C)


	def test_search_all3(self):
		A = self.A
		self.add_words_and_make_automaton()

		L = []
		def callback(index, word):
			L.append((index, word))

		start = 4
		end = 9

		L = []
		A.search_all(self.string[start:end], callback)
		C = [(pos+start, word) for pos, word in L]

		L = []
		A.search_all(self.string, callback, start, end)

		self.assertEqual(L, C)


class TestAutomatonIterSearch(TestAutomatonBase):

	def test_iter1(self):
		A = self.A
		self.assertEqual(A.kind, ahocorasick.EMPTY)
		with self.assertRaises(AttributeError):
			A.iter(self.string)
		
		A.add_word(b"word", None)
		self.assertEqual(A.kind, ahocorasick.TRIE)
		with self.assertRaises(AttributeError):
			A.iter(self.string)


	def test_iter2(self):
		A = self.A
		self.add_words_and_make_automaton()
		
		L = []
		for index, word in A.iter(self.string):
			L.append((index, word))

		C = self.correct_positons
		self.assertEqual(L, C)
		
		
	def test_iter3(self):
		A = self.A
		self.add_words_and_make_automaton()
		
		start = 4
		end = 9

		C = []
		for index, word in A.iter(self.string[start:end]):
			C.append((index + start, word))

		L = []
		for index, word in A.iter(self.string, start, end):
			L.append((index, word))

		self.assertEqual(L, C)

	
	def test_iter_search_all(self):
		A = self.A
		self.add_words_and_make_automaton()
		
		# results from search_all
		L = []
		def callback(index, word):
			L.append((index, word))

		A.search_all(self.string, callback)

		# results from iterator
		C = []
		for index, word in A.iter(self.string):
			C.append((index, word))

		self.assertEqual(L, C)


class TestAutomatonIterInvalidate(TestAutomatonBase):
	
	def test_iter1(self):
		A = self.A
		self.add_words_and_make_automaton()

		it = A.iter(self.string)
		w  = next(it)
		A.add_word("should fail", 1)
		with self.assertRaises(ValueError):
			w = next(it)


	def test_iter2(self):
		A = self.A
		self.add_words_and_make_automaton()

		it = A.iter(self.string)
		w  = next(it)
		A.clear()
		with self.assertRaises(ValueError):
			w = next(it)
		

if __name__ == '__main__':
	unittest.main()
