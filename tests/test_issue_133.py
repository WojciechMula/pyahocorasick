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

    res = list(automaton.iter_long("abb"))

    expected = [(1, "b"), (2, "b")]
    assert res == expected


def test_issue133_2():
    automaton = ahocorasick.Automaton()
    for word in ["b", "c", "abd"]:
        automaton.add_word(word, word)
    automaton.make_automaton()

    res = list(automaton.iter_long("abc"))

    expected = [(1, "b"), (2, "c")]
    assert res == expected


def test_issue133_3():
    automaton = ahocorasick.Automaton()
    for word in ["知识产权", "国家知识产权局"]:
        automaton.add_word(word, word)
    automaton.make_automaton()

    res = list(automaton.iter_long("国家知识产权"))

    expected = [(5, "知识产权")]
    assert res == expected
