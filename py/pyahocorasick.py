# -*- coding: iso-8859-2 -*-
"""
	Aho-Corasick string search algorithm.
	
	Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl/proj/pyahocorasick/
	License   : public domain
	Date      : $Date$

	$Id$
"""

nil = object()	# used to distinguish from None

class TrieNode(object):
	__slots__ = ['char', 'output', 'fail', 'children']
	def __init__(self, char):
		self.char = char
		self.output = nil
		self.fail = nil
		self.children = {}

	def __repr__(self):
		if self.output is not nil:
			return "<TrieNode '%s' '%s'>" % (self.char, self.output)
		else:
			return "<TrieNode '%s'>" % self.char


class Trie(object):
	def __init__(self):
		self.root = TrieNode('')


	def __get_node(self, word):
		node = self.root
		for c in word:
			try:
				node = node.children[c]
			except KeyError:
				return None

		return node


	def get(self, word, default=nil):
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
		for key, _ in self.items():
			yield key


	def values(self):
		for _, value in self.items():
			yield value

	
	def items(self):
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
		stack = [self.root]
		n = 0
		while stack:
			n += 1
			node = stack.pop()
			for child in node.children.values():
				stack.append(child)

		return n


	def add_word(self, word, value):
		if not word:
			return

		node = self.root
		for i, c in enumerate(word):
			try:
				node = node.children[c]
			except KeyError:
				n = TrieNode(c)
				node.children[c] = n
				node = n

		node.output = value


	def clear(self):
		self.root = TrieNode('')


	def exists(self, word):
		node = self.__get_node(word)
		if node:
			return bool(node.output != nil)
		else:
			return False


	def match(self, word):
		return (self.__get_node(word) is not None)
		

	def make_automaton(self):
		queue = []

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
			r = queue.pop(0);
			for node in r.children.values():
				queue.append(node)
				state = r.fail
				while node.char not in state.children:
						state = state.fail

				node.fail = state.children.get(node.char, self.root)


	def find_all(self, string, callback, start=None, end=None):
		for index, output in self.iter():
			callback(index, output)


	def iter(self, string, start=None, end=None):
		start = start if start is not None else 0
		end = end if end is not None else len(string)

		state = self.root
		for index, c in enumerate(string[start:end]):
			while c not in state.children:
				state = state.fail

			state = state.children.get(c, self.root)

			tmp = state
			output = []
			while tmp is not nil and tmp.output is not nil:
				output.append(tmp.output)
				tmp = tmp.fail

			if output:
				yield (index + start, output)





if __name__ == '__main__':
	words = "a ab bc bca c caa".split()
	words = "he hers his she hi him man".split()

	t = Trie();
	for w in words:
		t.add_word(w, w)

	s = "he rshershidamanza "

	t.make_automaton()
	#t.print()
	print("="*10)
	for res in t.items():
		print(res)

	for res in t.iter(s):
		print(res)

	#input()

# vim: ts=4 sw=4 nowrap

