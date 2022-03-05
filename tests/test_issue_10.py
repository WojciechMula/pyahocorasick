# -*- coding: utf-8 -*-
"""
    Aho-Corasick string search algorithm.

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
    License   : public domain
"""

import ahocorasick

ac = ahocorasick.Automaton()
ac.add_word('S', 1)
ac.make_automaton()
buffer = 'SSS'

def case_1():
    count = 0
    for item in ac.iter(buffer, 0, 3):  # this causes an error
        print(item)
        count += 1

    assert(count == 3)

def case_2():
    count = 0
    for item in ac.iter(buffer, 0, 2):  # no error, but it misses the last 'S' in the buffer
        print(item)
        count += 1

    assert(count == 2)

case_1()
case_2()
