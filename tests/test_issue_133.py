# -*- coding: utf-8 -*-

"""
    Aho-Corasick string search algorithm.

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
    License   : public domain
"""
import ahocorasick

from pytestingutils import conv


def test_issue133_iter_long_1():
    automaton = ahocorasick.Automaton()
    automaton.add_word(conv("b"), "b")
    automaton.add_word(conv("abc"), "abc")
    automaton.make_automaton()

    res = list(automaton.iter_long(conv("abb")))

    expected = [(1, "b"), (2, "b")]
    assert res == expected


def test_issue133_iter_long_2():
    automaton = ahocorasick.Automaton()
    for word in ["b", "c", "abd"]:
        converted = conv(word)
        automaton.add_word(converted, word)
    automaton.make_automaton()

    res = list(automaton.iter_long(conv("abc")))

    expected = [(1, "b"), (2, "c")]
    assert res == expected


def test_issue133_iter_long_with_multibyte_characters():
    automaton = ahocorasick.Automaton()
    for word in ["知识产权", "国家知识产权局"]:
        converted = conv(word)
        automaton.add_word(converted, word)
    automaton.make_automaton()

    res = list(automaton.iter_long(conv("国家知识产权")))
    if ahocorasick.unicode:
        expected = [(5, "知识产权")]
    else:
        # UTF-8-bytes
        expected = [(17, "知识产权")]
    assert res == expected
