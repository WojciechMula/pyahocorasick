import pyahocorasick


def exportdot(trie, file):

	def writeln(text=""):
		file.write(text + "\n")

	writeln("digraph ahocorasick {")

	def walk(node):
		queue = [node]
		while queue:
			node = queue.pop()
			yield node

			for child in node.children.itervalues():
				if child != node:
					queue.append(child)

	nodes = list(walk(trie.root))

	# nodes
	for node in nodes:
		if node.output != pyahocorasick.nil:
			writeln("\tnode%d [shape=doublecircle, label=\"\"]" % id(node))
		else:
			writeln("\tnode%d [shape=circle, label=\"\"]" % id(node))

	# trie edges
	for node in nodes:
		for letter, child in node.children.iteritems():
			nodeid = id(node)
			destid = id(child)
			if destid == id(trie.root):
				# do not show self-links of root node created during make_automaton
				continue

			if letter.isalnum():
				label = letter
			else:
				label = '%02x' % ord(letter)

			writeln("\tnode%d -> node%d [label=\"%s\"]" % (nodeid, destid, label))

	# fail links
	for node in nodes:
		nodeid = id(node)
		failid = id(node.fail)

		if failid != pyahocorasick.nil:
			writeln("\tnode%d -> node%d [color=blue]" % (nodeid, failid))

	writeln("}")


if __name__ == '__main__':
	A = pyahocorasick.Trie()

	A.add_word("he", 0)
	A.add_word("her", 1)
	A.add_word("hers", 2)
	A.add_word("she", 3)
	A.add_word("cat", 4)
	A.add_word("shield", 5)

	with open('trie.dot', 'wt') as f:
		exportdot(A, f)

	A.make_automaton()

	with open('ahocorasick.dot', 'wt') as f:
		exportdot(A, f)
