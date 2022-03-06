# -*- coding: utf-8 -*-
"""
    Aho-Corasick string search algorithm.

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
    License   : public domain
"""

import pickle

import ahocorasick as aho


def test_issue_26_pickle_workds():
    a = aho.Automaton(aho.STORE_INTS)
    a.add_word('abc', 12)
    a.make_automaton()
    pickle.dumps(a)
