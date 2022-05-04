# -*- coding: utf-8 -*-
"""
    Aho-Corasick string search algorithm.

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
    License   : public domain
"""

import pickle

import ahocorasick
from pytestingutils import conv


def test_issue_26_pickle_workds():
    a = ahocorasick.Automaton(ahocorasick.STORE_INTS)
    a.add_word(conv('abc'), 12)
    a.make_automaton()
    pickle.dumps(a)
