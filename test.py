"""
	Aho-Corasick string search algorithm.

	Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl
	License   : public domain
"""

import ahocorasick

print(dir(ahocorasick))

a = ahocorasick.Automaton()
words = b"he e hers his she hi him man he".split()
for i,w in enumerate(words):
	a.add_word(w, (i, w))

#print(len(a), len(set(words)))
#print(a.get(b"hea", 1))

"""
for w in a.keys():
	print(w)

for w in a.values():
	print(w)

for w in a.items():
	print(w)
"""

s = b"he rshershidamanza "

a.make_automaton()
for item in a.iter(s, 2, 8):
	print(item)

print("==")

def callback(index, item):
	print(index, item)

a.search_all(s, callback, 2, 11)
