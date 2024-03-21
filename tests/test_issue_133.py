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


def test_issue133_iter_long_3():
    automaton = ahocorasick.Automaton()
    for word in ["trimethoprim", "sulfamethoxazole", "meth"]:
        converted = conv(word)
        automaton.add_word(converted, word)
    automaton.make_automaton()

    res = list(automaton.iter_long(conv("sulfamethoxazole and trimethoprim")))

    expected = [(15, "sulfamethoxazole"), (32, "trimethoprim")]
    assert res == expected

def test_issue133_iter_long_4():
    automaton = ahocorasick.Automaton()
    for word in ["is", "this", "is this a dream?"]:
        converted = conv(word)
        automaton.add_word(converted, word)
    automaton.make_automaton()

    res = list(automaton.iter_long(conv("is this a test?")))

    expected = [(1, "is"), (6, "this")]
    assert res == expected

def test_issue133_iter_long_5():
    automaton = ahocorasick.Automaton()
    for word in ["th", "this", "is this a dream?"]:
        converted = conv(word)
        automaton.add_word(converted, word)
    automaton.make_automaton()

    res = list(automaton.iter_long(conv("is this a test?")))

    expected = [(6, "this")]
    assert res == expected



def test_issue133_iter_long_with_multibyte_characters():
    automaton = ahocorasick.Automaton()
    for word in ["知识产权", "国家知识产权局2"]:
        converted = conv(word)
        automaton.add_word(converted, word)
    automaton.make_automaton()

    res = list(automaton.iter_long(conv("国家知识产权局1")))
    if ahocorasick.unicode:
        expected = [(5, "知识产权")]
    else:
        # UTF-8-bytes
        expected = [(17, "知识产权")]
    assert res == expected
