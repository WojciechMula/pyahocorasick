# -*- coding: utf-8 -*-

"""
	Aho-Corasick string search algorithm.

	Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl
	License   : public domain
"""

import ahocorasick
import sys

print(dir(ahocorasick))


def is_python_2():
    return sys.version_info.major == 2


a = ahocorasick.Automaton()
words = b"he e hers his she hi him man he"
if not is_python_2():
    words = words.decode('utf8')
for i,w in enumerate(words.split()):
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
if not is_python_2():
    s = s.decode('utf8')

a.make_automaton()
for item in a.iter(s, 2, 8):
	print(item)

print("==")

def callback(index, item):
	print(index, item)

a.find_all(s, callback, 2, 11)
