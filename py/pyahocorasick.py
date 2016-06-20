# -*- coding: utf-8 -*-
"""
	Aho-Corasick string search algorithm.

	Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl
	License   : public domain
"""

from collections import deque

nil = object()	# used to distinguish from None

class TrieNode(object):
	"""
	Node of trie/Aho-Corasick automaton
	"""

	__slots__ = ['char', 'output', 'fail', 'children']

	def __init__(self, char):
		"""
		Constructs an empty node
		"""

		self.char = char		# character
		self.output = nil		# an output function for this node
		self.fail = nil			# fail link used by Aho-Corasick automaton
		self.children = {}		# children

	def __repr__(self):
		"""
		Textual representation of node.
		"""

		if self.output is not nil:
			return "<TrieNode '%s' '%s'>" % (self.char, self.output)
		else:
			return "<TrieNode '%s'>" % self.char


class Trie(object):
	"""
	Trie/Aho-Corasick automaton.
	"""

	def __init__(self):
		"""
		Construct an empty trie
		"""

		self.root = TrieNode('')


	def __get_node(self, word):
		"""
		Private function retrieving a final node of trie
		for given word

		Returns node or None, if the trie doesn't contain the word.
		"""

		node = self.root
		for c in word:
			try:
				node = node.children[c]
			except KeyError:
				return None

		return node


	def get(self, word, default=nil):
		"""
		Retrieves output value associated with word.

		If there is no word returns default value,
		and if default is not given rises KeyError.
		"""

		node = self.__get_node(word)
		output = nil
		if node:
			output = node.output

		if output is nil:
			if default is nil:
				raise KeyError("no key '%s'" % word)
			else:
				return default
		else:
			return output


	def keys(self):
		"""
		Generator returning all keys (i.e. word) stored in trie
		"""

		for key, _ in self.items():
			yield key


	def values(self):
		"""
		Generator returning all values associated with words stored in a trie.
		"""

		for _, value in self.items():
			yield value


	def items(self):
		"""
		Generator returning all keys and values stored in a trie.
		"""

		L = []
		def aux(node, s):
			s = s + node.char
			if node.output is not nil:
				L.append((s, node.output))

			for child in node.children.values():
				if child is not node:
					aux(child, s)

		aux(self.root, '')
		return iter(L)


	def __len__(self):
		"""
		Calculates number of words in a trie.
		"""

		stack = deque()
		stack.append(self.root)
		n = 0
		while stack:
			node = stack.pop()
			if node.output is not nil:
				n += 1

			for child in node.children.values():
				stack.append(child)

		return n


	def add_word(self, word, value):
		"""
		Adds word and associated value.

		If word already exists, its value is replaced.
		"""
		if not word:
			return

		node = self.root
		for c in word:
			try:
				node = node.children[c]
			except KeyError:
				n = TrieNode(c)
				node.children[c] = n
				node = n

		node.output = value


	def clear(self):
		"""
		Clears trie.
		"""

		self.root = TrieNode('')


	def exists(self, word):
		"""
		Checks if whole word is present in the trie.
		"""

		node = self.__get_node(word)
		if node:
			return bool(node.output != nil)
		else:
			return False


	def match(self, word):
		"""
		Checks if word is a prefix of any existing word in the trie.
		"""

		return (self.__get_node(word) is not None)


	def make_automaton(self):
		"""
		Converts trie to Aho-Corasick automaton.
		"""

		queue = deque()

		# 1.
		for i in range(256):
			c = chr(i)
			if c in self.root.children:
				node = self.root.children[c]
				node.fail = self.root	# f(s) = 0
				queue.append(node)
			else:
				self.root.children[c] = self.root

		# 2.
		while queue:
			r = queue.popleft()
			for node in r.children.values():
				queue.append(node)
				state = r.fail
				while node.char not in state.children:
						state = state.fail

				node.fail = state.children.get(node.char, self.root)


	def iter(self, string):
		"""
		Generator performs Aho-Corasick search string algorithm, yielding
		tuples containing two values:
		- position in string
		- outputs associated with matched strings
		"""
		state = self.root
		for index, c in enumerate(string):
			while c not in state.children:
				state = state.fail

			state = state.children.get(c, self.root)

			tmp = state
			output = []
			while tmp is not nil:
				if tmp.output is not nil:
					output.append(tmp.output)

				tmp = tmp.fail

			if output:
				yield (index, output)

	def iter_long(self, string):
		"""
		Generator performs a modified Aho-Corasick search string algorithm,
		which maches only the longest word.

		"""
		state = self.root
		last  = None

		index = 0
		while index < len(string):
			c = string[index]

			if c in state.children:
				state = state.children[c]

				if state.output is not nil:
					# save the last node on the path
					last = (state.output, index)

				index += 1
			else:
				if last:
					# return the saved match
					yield last

					# and start over, as we don't want overlapped results
					# Note: this leads to quadratic complexity in the worst case
					index = last[1] + 1
					state = self.root
					last  = None
				else:
					# if no output, perform classic Aho-Corasick algorithm
					while c not in state.children:
						state = state.fail

		# corner case
		if last:
			yield last

	def find_all(self, string, callback):
		"""
		Wrapper on iter method, callback gets an iterator result
		"""
		for index, output in self.iter(string):
			callback(index, output)



if __name__ == '__main__':

	def demo():
		words = "he hers his she hi him man".split()

		t = Trie();
		for w in words:
			t.add_word(w, w)

		s = "he rshershidamanza "

		t.make_automaton()
		for res in t.items():
			print(res)

		for res in t.iter(s):
			print
			print('%s' % s)
			pos, matches = res
			for fragment in matches:
				print('%s%s' % ((pos - len(fragment) + 1)*' ', fragment))

	demo()


	def bug():
		patterns = ['GT-C3303','SAMSUNG-GT-C3303K/']
		text = 'SAMSUNG-GT-C3303i/1.0 NetFront/3.5 Profile/MIDP-2.0 Configuration/CLDC-1.1'

		t = Trie()
		for pattern in patterns:
			ret = t.add_word(pattern, (0, pattern))

		t.make_automaton()

		res = list(t.iter(text))

		assert len(res) == 1, 'failed'

	bug()

# vim: ts=4 sw=4 nowrap

