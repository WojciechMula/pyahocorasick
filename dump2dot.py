"""
	Aho-Corasick string search algorithm.

	Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl
	License   : public domain
"""

import ahocorasick

from ahocorasick import EMPTY, TRIE, AHOCORASICK;


def dump2dot(automaton, file):

	def writeln(text=""):
		file.write(text + "\n")

	if automaton.kind == EMPTY:
		writeln("digraph empty {}")
		return

	if automaton.kind == TRIE:
		name = "trie"
	else:
		name = "ahocorasick"


	writeln("digraph %s {" % name)

	nodes, edges, fail = automaton.dump()

	# nodes
	for nodeid, end in nodes:
		if end:
			writeln("\tnode%d [shape=doublecircle, label=\"\"]" % nodeid)
		else:
			writeln("\tnode%d [shape=circle, label=\"\"]" % nodeid)

	# trie edges
	for nodeid, label, destid in edges:
		writeln("\tnode%d -> node%d [label=%s]" % (nodeid, destid, str(label, 'ascii')))

	# fail links
	for nodeid, failid in fail:
		writeln("\tnode%d -> node%d [color=blue]" % (nodeid, failid))

	writeln("}")


if __name__ == '__main__':
	A = ahocorasick.Automaton(ahocorasick.STORE_LENGTH)

	A.add_word("he")
	A.add_word("her")
	A.add_word("hers")
	A.add_word("she")
	A.add_word("cat")
	A.add_word("shield")

	with open('trie.dot', 'wt') as f:
		dump2dot(A, f)

	A.make_automaton()
	with open('ahocorasick.dot', 'wt') as f:
		dump2dot(A, f)
