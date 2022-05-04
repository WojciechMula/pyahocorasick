# -*- coding: utf-8 -*-

"""
    Aho-Corasick string search algorithm.

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
    License   : public domain
"""
import ahocorasick

from pytestingutils import conv


def iter_results(teststr, automaton):
    r = []
    for x in automaton.iter(teststr):
        r.append(x)

    return r


def find_all_results(teststr, automaton):
    r = []

    def append(x, teststr):
        r.append((x, teststr))

    automaton.find_all(teststr, append)
    return r


def test_issue56():
    automaton = ahocorasick.Automaton()

    for word in ("poke", "go", "pokegois", "egoist"):
        automaton.add_word(conv(word), word)

    automaton.make_automaton()

    teststr = conv("pokego pokego  pokegoist")
    expected = iter_results(teststr, automaton)
    findall = find_all_results(teststr, automaton)

    assert findall == expected

