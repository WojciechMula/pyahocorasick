"""
	Aho-Corasick string search algorithm.

	Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl
	License   : public domain
"""

import ahocorasick
import os

from ahocorasick import EMPTY, TRIE, AHOCORASICK;


def dump2dot(automaton, file):

	def writeln(text=""):
		file.write(text + "\n")

	def nodename(nodeid):
		return 'node%x' % (nodeid & 0xffffffff)

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
			attr = '[shape=doublecircle, label=""]'
		else:
			attr = '[shape=circle, label=""]'

		writeln("\t%s %s" % (nodename(nodeid), attr))

	def format_label(label):
		label = str(label, 'ascii')
		label = label.replace('"', r'\"')
		return '"%s"' % label

	# trie edges
	for nodeid, label, destid in edges:
		writeln("\t%s -> %s [label=%s]" % (nodename(nodeid), nodename(destid), format_label(label)))

	# fail links
	for nodeid, failid in fail:
		writeln("\t%s -> %s [color=blue]" % (nodename(nodeid), nodename(failid)))

	writeln("}")


def show(automaton):
	path = '/dev/shm/%s.dot' % os.getpid()
	with open(path, 'wt') as f:
		dump2dot(automaton, f)

	os.system("xdot %s" % path)
	#os.system("dotty %s" % path)
	os.unlink(path)


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
