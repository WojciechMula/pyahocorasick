# -*- coding: utf-8 -*-
"""
    Aho-Corasick string search algorithm.

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
    License   : public domain
"""

import ahocorasick


def build_int_sequence_automaton():
    A = ahocorasick.Automaton(ahocorasick.STORE_ANY, ahocorasick.KEY_SEQUENCE)
    A.add_word((1, 2), "val_for_1_2")
    A.add_word((1, 2, 3), "val_for_1_2_3")
    A.add_word((1, 2, 4, 5), "val_for_1_2_4_5")
    A.add_word((1, 3), "val_for_1_3")
    A.add_word((5,), "val_for_5")
    return A


def test_issue_216_keys_with_prefix_on_key_sequence():
    A = build_int_sequence_automaton()

    result = set(A.keys((1, 2)))

    assert result == {"\x01\x02", "\x01\x02\x03", "\x01\x02\x04\x05"}


def test_issue_216_values_with_prefix_on_key_sequence():
    A = build_int_sequence_automaton()

    result = set(A.values((1, 2)))

    assert result == {"val_for_1_2", "val_for_1_2_3", "val_for_1_2_4_5"}


def test_issue_216_items_with_prefix_on_key_sequence():
    A = build_int_sequence_automaton()

    result = set(A.items((1, 2)))

    assert result == {
        ("\x01\x02", "val_for_1_2"),
        ("\x01\x02\x03", "val_for_1_2_3"),
        ("\x01\x02\x04\x05", "val_for_1_2_4_5"),
    }


def test_issue_216_keys_without_prefix_on_key_sequence_still_works():
    A = build_int_sequence_automaton()

    result = set(A.keys())

    assert result == {
        "\x01\x02",
        "\x01\x02\x03",
        "\x01\x02\x04\x05",
        "\x01\x03",
        "\x05",
    }
