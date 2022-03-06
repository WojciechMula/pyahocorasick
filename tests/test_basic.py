# -*- coding: utf-8 -*-

"""
    Aho-Corasick string search algorithm.

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
    License   : public domain
"""

import ahocorasick


def test_basic():
    automaton = ahocorasick.Automaton()
    words = "he e hers his she hi him man he".split()
    #         0 1    2   3   4  5   6   7  8
    for i, w in enumerate(words):
        automaton.add_word(w, (i, w))
    query = "he rshershidamanza "
    #        01234567890123
    automaton.make_automaton()

    assert query[2:8] == ' rsher'
    results = list(automaton.iter(string=query, start=2, end=8))
    assert results == [(6, (4, 'she')), (6, (8, 'he')), (6, (1, 'e'))]

    res = []

    def callback(index, item):
        res.append(dict(index=index, item=item))

    assert query[2:11] == ' rshershi'
    automaton.find_all(query, callback, 2, 11)

    expected = [
        {'index': 6, 'item': (4, 'she')},
        {'index': 6, 'item': (8, 'he')},
        {'index': 6, 'item': (1, 'e')},
        {'index': 8, 'item': (2, 'hers')},
        {'index': 10, 'item': (5, 'hi')},
    ]

    assert res == expected
