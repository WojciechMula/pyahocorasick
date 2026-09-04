# -*- coding: utf-8 -*-

"""Regression tests for longest, non-overlapping iteration."""

import ahocorasick

from pytestingutils import conv


def iter_long(patterns, text):
    automaton = ahocorasick.Automaton()
    for pattern in patterns:
        automaton.add_word(conv(pattern), pattern)
    automaton.make_automaton()

    return list(automaton.iter_long(conv(text)))


def test_match_reached_via_failure_link_at_end_of_input():
    assert iter_long(["abc", "b"], "ab") == [(1, "b")]


def test_match_reached_via_multiple_failure_links_on_mismatch():
    assert iter_long(["abcd", "bcx", "c"], "abcz") == [(2, "c")]
