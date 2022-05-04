# -*- coding: utf-8 -*-

"""
    Aho-Corasick string search algorithm.

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
    License   : public domain
"""

from unittest.case import skipIf

import ahocorasick


@skipIf(not ahocorasick.unicode, "Run only with unicode build")
def test_issue_53_unicode():
    auto = ahocorasick.Automaton()
    auto.add_word('wounded', 'wounded')

    auto.make_automaton()

    results = list(auto.iter('Winning \U0001F629 so gutted, can\'t do anything for 4 weeks... Myth. #wounded'))
    expected = [(67, 'wounded')]
    assert results == expected

    expected = [(65, 'wounded')]
    results = list(auto.iter('Winning so gutted, can\'t do anything for 4 weeks... Myth. #wounded'))
    assert results == expected


@skipIf(ahocorasick.unicode, "Run only with bytes build")
def test_issue_53_bytes():
    auto = ahocorasick.Automaton()
    auto.add_word(b'wounded', b'wounded')

    auto.make_automaton()

    results = list(auto.iter('Winning \U0001F629 so gutted, can\'t do anything for 4 weeks... Myth. #wounded'.encode(encoding='utf_8')))
    expected = [(70, b'wounded')]
    assert results == expected

    expected = [(65, b'wounded')]
    results = list(auto.iter(b'Winning so gutted, can\'t do anything for 4 weeks... Myth. #wounded'))
    assert results == expected
