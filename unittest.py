import unittest
import ahocorasick
import ahocorasick

class TestTrieStorePyObjects(unittest.TestCase):
	def setUp(self):
		self.A = A = ahocorasick.Automaton();
		self.s = b"he rshershidamanza "

		words = b"he e hers his she hi him man he".split()
		for i, w in enumerate(words):
			A.add_word(w, (i, w))

	def test_search_all(self):
		def callback(index, value):
			print(index, value)

		self.A.search_all(self.s, callback);


	def test_iter1(self):

		for item in self.A.iter(self.s):
			print(item)


if __name__ == '__main__':
	unittest.main()
