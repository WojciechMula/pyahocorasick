# -*- coding: utf-8 -*-
"""
    Aho-Corasick string search algorithm.

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
    License   : public domain
"""

import ahocorasick
from pytestingutils import conv


def test_issue_10_case_1():
    ac = ahocorasick.Automaton()
    ac.add_word(conv('S'), 1)
    ac.make_automaton()
    buffer = conv('SSS')
    count = 0
    for _item in ac.iter(buffer, 0, 3):  # this causes an error
        count += 1

    assert count == 3


def test_issue_10_case_2():
    ac = ahocorasick.Automaton()
    ac.add_word(conv('S'), 1)
    ac.make_automaton()
    buffer = conv('SSS')
    count = 0
    for _item in ac.iter(buffer, 0, 2):  # no error, but it misses the last 'S' in the buffer
        count += 1

    assert count == 2
