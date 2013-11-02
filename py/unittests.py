import unittest
from pyahocorasick import Trie

class TestTrie(unittest.TestCase):
	def testEmptyTrieShouldNotContainsAnyWords(self):
		t = Trie()
		self.assertEqual(len(t), 0)


	def testAddedWordShouldBeCountedAndAvailableForRetrieval(self):
		t = Trie()
		t.add_word('python', 'value')
		self.assertEqual(len(t), 1)
		self.assertEqual(t.get('python'), 'value')


	def testAddingExistingWordShouldReplaceAssociatedValue(self):
		t = Trie()
		t.add_word('python', 'value')
		self.assertEqual(len(t), 1)
		self.assertEqual(t.get('python'), 'value')

		t.add_word('python', 'other')
		self.assertEqual(len(t), 1)
		self.assertEqual(t.get('python'), 'other')

	def testGetUnknowWordWithoutDefaultValueShouldRaiseException(self):
		t = Trie()
		with self.assertRaises(KeyError):
			t.get('python')


	def testGetUnknowWordWithDefaultValueShouldReturnDefault(self):
		t = Trie()
		self.assertEqual(t.get('python', 'default'), 'default')


	def testExistShouldDetectAddedWords(self):
		t = Trie()
		t.add_word('python', 'value')
		t.add_word('ada', 'value')

		self.assertTrue(t.exists('python'))
		self.assertTrue(t.exists('ada'))


	def testExistShouldReturnFailOnUnknownWord(self):
		t = Trie()
		t.add_word('python', 'value')

		self.assertFalse(t.exists('ada'))


	def testMatchShouldDetecAllPrefixesIncludingWord(self):
		t = Trie()
		t.add_word('python', 'value')
		t.add_word('ada', 'value')

		self.assertTrue(t.match('a'))
		self.assertTrue(t.match('ad'))
		self.assertTrue(t.match('ada'))

		self.assertTrue(t.match('p'))
		self.assertTrue(t.match('py'))
		self.assertTrue(t.match('pyt'))
		self.assertTrue(t.match('pyth'))
		self.assertTrue(t.match('pytho'))
		self.assertTrue(t.match('python'))


	def testItemsShouldReturnAllItemsAlreadyAddedToTheTrie(self):
		t = Trie()

		t.add_word('python', 1)
		t.add_word('ada', 2)
		t.add_word('perl', 3)
		t.add_word('pascal', 4)
		t.add_word('php', 5)

		result = list(t.items())
		self.assertEquals(len(result), 5)
		self.assertIn(('python', 1), result)
		self.assertIn(('ada',    2), result)
		self.assertIn(('perl',   3), result)
		self.assertIn(('pascal', 4), result)
		self.assertIn(('php',    5), result)


	def testKeysShouldReturnAllKeysAlreadyAddedToTheTrie(self):
		t = Trie()

		t.add_word('python', 1)
		t.add_word('ada', 2)
		t.add_word('perl', 3)
		t.add_word('pascal', 4)
		t.add_word('php', 5)

		result = list(t.keys())
		self.assertEquals(len(result), 5)
		self.assertIn('python',result)
		self.assertIn('ada',   result)
		self.assertIn('perl',  result)
		self.assertIn('pascal',result)
		self.assertIn('php',   result)


	def testValuesShouldReturnAllValuesAlreadyAddedToTheTrie(self):
		t = Trie()

		t.add_word('python', 1)
		t.add_word('ada', 2)
		t.add_word('perl', 3)
		t.add_word('pascal', 4)
		t.add_word('php', 5)

		result = list(t.values())
		self.assertEquals(len(result), 5)
		self.assertIn(1, result)
		self.assertIn(2, result)
		self.assertIn(3, result)
		self.assertIn(4, result)
		self.assertIn(5, result)


	def testClearShouldRemoveEveryting(self):
		t = Trie()

		t.add_word('python', 1)
		t.add_word('ada', 2)
		t.add_word('perl', 3)
		t.add_word('pascal', 4)
		t.add_word('php', 5)

		self.assertEqual(len(t), 5)
		self.assertEqual(len(list(t.items())), 5)

		t.clear()

		self.assertEqual(len(t), 0)
		self.assertEqual(len(list(t.items())), 0)


	def testIterShouldMatchAllStrings(self):

		def get_test_automaton():
			words = "he her hers his she hi him man himan".split()

			t = Trie();
			for w in words:
				t.add_word(w, w)

			t.make_automaton()

			return t


		test_string = "he she himan"

		t = get_test_automaton()
		result = list(t.iter(test_string))

		# there are 5 matching positions
		self.assertEquals(len(result), 5)

		# result should have be valid, i.e. returned position and substring
		# must match substring from test string
		for end_index, strings in result:
			for s in strings:
				n = len(s)
				self.assertEqual(s, test_string[end_index - n + 1 : end_index + 1])


	def testFindAllShouldGetTheSameDataAsIter(self):

		def get_test_automaton():
			words = "he her hers his she hi him man himan".split()

			t = Trie();
			for w in words:
				t.add_word(w, w)

			t.make_automaton()

			return t

		find_all_arguments = []

		def find_all_callback(end_index, strings):
			find_all_arguments.append((end_index, strings))

		t = get_test_automaton()
		test_string = "he she himan"

		t.find_all(test_string, find_all_callback)

		result_items = list(t.iter(test_string))
		self.assertEquals(find_all_arguments, result_items)


if __name__ == '__main__':
	unittest.main()
