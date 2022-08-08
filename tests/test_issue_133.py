# -*- coding: utf-8 -*-

"""
    Aho-Corasick string search algorithm.

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
    License   : public domain
"""
import ahocorasick


def test_issue133():
    automaton = ahocorasick.Automaton()
    automaton.add_word("b", "b")
    automaton.add_word("abc", "abc")
    automaton.make_automaton()

    res = []
    for i, s in automaton.iter_long("abb"):
        res.append((i, s))

    expected = [(1, "b"), (2, "b")]
    assert res == expected
