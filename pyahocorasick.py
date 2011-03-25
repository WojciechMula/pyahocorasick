class TrieNode(object):
	__slots__ = ['c', 'output', 'fail', 'child']
	def __init__(self, c):
		self.c = c
		self.output = set()
		self.fail = None
		self.child = {}

	def __repr__(self):
		return "<TrieNode '%s' '%s'>" % (self.c, self.output)


class Trie(object):
	def __init__(self):
		self.root = TrieNode('')

	def add(self, s):
		if not s:
			return

		node = self.root
		for i, c in enumerate(s):
			try:
				node = node.child[c]
			except KeyError:
				n = TrieNode(c)
				node.child[c] = n
				node = n

		node.output.add(s)

	def make_fail(self):
		queue = []

		# 1.
		for i in range(256):
			c = chr(i)
			if c in self.root.child:
				node = self.root.child[c]
				node.fail = self.root	# f(s) = 0
				queue.append(node)
			else:
				self.root.child[c] = self.root

		# 2.
		while queue:
			r = queue.pop(0);
			for node in r.child.values():
				queue.append(node)
				state = r.fail
				c = node.c
				while c not in state.child:
						state = state.fail

				node.fail = state.child.get(c, self.root)
				node.output.update(node.fail.output)



	def determinize(self):
		queue = []

		# 1.
		for node in self.root.child.values():
			if node != self.root:
				queue.append(node)

		# 2.
		while queue:
			r = queue.pop(0)
			for i in range(256):
				c = chr(i)
				if c not in r.child:
					state = r.fail
					while c not in state.child:
							state = state.fail

					r.child[c] = state.child[c]
				else:
					queue.append(r.child[c])


	def match(self, s):
		print(s)
		state = self.root
		for index, c in enumerate(s):
			while c not in state.child:
				state = state.fail

			state = state.child.get(c, self.root)

			if state.output:
				print(index, ", ".join(state.output))


	def match_determinized(self, s):
		state = self.root
		for index, c in enumerate(s):
			state = state.child[c]

			if state.output:
				print(index, ", ".join(state.output))


	def print(self):
		def aux(node, s):
			if node.output:
				print(s + node.c, node.output)
			
			for n in node.child.values():
				aux(n, s + node.c)

		for node in self.root.child.values():
			if node != self.root:
				aux(node, '')


if __name__ == '__main__':
	words = "a ab bc bca c caa".split()
	words = "he hers his she hi him man".split()

	t = Trie();
	#for w in words:
	#	t.add(w)
	for w in open('xxx/test1.txt'):
		t.add(w.rstrip())

	s = "he rshershidamanza "

	t.make_fail()
	#t.print()
	print("="*10)
	t.match(s)

	print("="*10)
	t.determinize()
	t.match_determinized(s)


	#input()

# vim: ts=4 sw=4 nowrap

