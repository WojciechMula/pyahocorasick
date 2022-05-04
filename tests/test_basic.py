# -*- coding: utf-8 -*-

"""
    Aho-Corasick string search algorithm.

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
    License   : public domain
"""

import pytest

import ahocorasick

from pytestingutils import on_windows


@pytest.mark.skipif(ahocorasick.unicode, reason="Run only with bytes build")
@pytest.mark.xfail(reason="Fails on Windows")
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


@pytest.mark.skipif(ahocorasick.unicode, reason="Run only with bytes build")
@pytest.mark.xfail(reason="Fails on everywhere")
def test_basic_items_keys_and_values_with_bytes_build():
    automaton = ahocorasick.Automaton()
    words = b"he e hers his she hi him man he".split()
    #         1  2 3    4   5   6  7   8   9
    for i, w in enumerate(words, 1):
        automaton.add_word(w, (i, w))

    expected_keys = [b'e', b'hers', b'his', b'she', b'hi', b'him', b'man', b'he', ]
    expected_values = [
        # the second addition munges this by design, like in a dict (1, b'he'),
        (2, b'e'),
        (3, b'hers'),
        (4, b'his'),
        (5, b'she'),
        (6, b'hi'),
        (7, b'him'),
        (8, b'man'),
        (9, b'he'),
    ]

    expected_items = [
        # the second addition munges this by design, like in a dict  (b'he', (1, b'he')),
        (b'e', (2, b'e')),
        (b'hers', (3, b'hers')),
        (b'his', (4, b'his')),
        (b'she', (5, b'she')),
        (b'hi', (6, b'hi')),
        (b'him', (7, b'him')),
        (b'man', (8, b'man')),
        (b'he', (9, b'he')),
    ]

    assert sorted(automaton.keys()) == sorted(expected_keys)
    assert sorted(automaton.values()) == sorted(expected_values)
    assert sorted(dict(automaton.items()).values()) == sorted(expected_values)
    assert sorted(dict(automaton.items()).keys()) == sorted(expected_keys)

    automaton.make_automaton()

    assert sorted(automaton.keys()) == sorted(expected_keys)
    assert sorted(automaton.values()) == sorted(expected_values)
    assert sorted(dict(automaton.items()).values()) == sorted(expected_values)
    assert sorted(dict(automaton.items()).keys()) == sorted(expected_keys)
    assert sorted(automaton.items()) == sorted(expected_items)


@pytest.mark.skipif(not ahocorasick.unicode, reason="Run only with unicode build")
@pytest.mark.xfail(on_windows, reason="Fails on Windows")
def test_basic_unicode():
    automaton = ahocorasick.Automaton()
    words = "he e hers his she hi him man he".split()
    #        0  1 2    3   4   5  6   7   8
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


@pytest.mark.skipif(not ahocorasick.unicode, reason="Run only with unicode build")
@pytest.mark.xfail(on_windows, reason="Fails on Windows")
def test_basic_items_keys_and_values_with_unicode_build():
    automaton = ahocorasick.Automaton()
    words = 'he e hers his she hi him man he'.split()
    #         0 1    2   3   4  5   6   7  8
    for i, w in enumerate(words):
        automaton.add_word(w, (i, w))

    expected_keys = ['man', 'she', 'e', 'hi', 'him', 'his', 'he', 'hers']

    expected_values = [
        (7, 'man'),
        (4, 'she'),
        (1, 'e'),
        (5, 'hi'),
        (6, 'him'),
        (3, 'his'),
        (8, 'he'),
        (2, 'hers'),
    ]

    assert sorted(automaton.keys()) == sorted(expected_keys)
    assert sorted(automaton.values()) == sorted(expected_values)
    assert sorted(dict(automaton.items()).values()) == sorted(expected_values)
    assert sorted(dict(automaton.items()).keys()) == sorted(expected_keys)

    automaton.make_automaton()

    assert sorted(automaton.keys()) == sorted(expected_keys)
    assert sorted(automaton.values()) == sorted(expected_values)
    assert sorted(dict(automaton.items()).values()) == sorted(expected_values)
    assert sorted(dict(automaton.items()).keys()) == sorted(expected_keys)
