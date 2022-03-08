# -*- coding: utf-8 -*-

"""
    Aho-Corasick string search algorithm.

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
    License   : public domain
"""
from unittest.case import skipIf

import ahocorasick


@skipIf(ahocorasick.unicode, "Run only with bytes build")
def test_basic_bytes():
    automaton = ahocorasick.Automaton()
    words = b"he e hers his she hi him man he".split()
    #         0 1    2   3   4  5   6   7  8
    for i, w in enumerate(words):
        automaton.add_word(w, (i, w))
    query = b"he rshershidamanza "
    #        01234567890123
    automaton.make_automaton()

    assert query[2:8] == b' rsher'
    results = list(automaton.iter(string=query, start=2, end=8))
    assert results == [(6, (4, b'she')), (6, (8, b'he')), (6, (1, b'e'))]

    res = []

    def callback(index, item):
        res.append(dict(index=index, item=item))

    assert query[2:11] == b' rshershi'
    automaton.find_all(query, callback, 2, 11)

    expected = [
        {'index': 6, 'item': (4, b'she')},
        {'index': 6, 'item': (8, b'he')},
        {'index': 6, 'item': (1, b'e')},
        {'index': 8, 'item': (2, b'hers')},
        {'index': 10, 'item': (5, b'hi')},
    ]

    assert res == expected


@skipIf(not ahocorasick.unicode, "Run only with unicode build")
def test_basic_unicode():
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
