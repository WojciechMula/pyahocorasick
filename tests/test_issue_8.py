# -*- coding: utf-8 -*-

"""
    Aho-Corasick string search algorithm.

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
    License   : public domain
"""
from unittest.case import skipIf

import ahocorasick

from pytestingutils import conv

test_sentences_rus = ["!ASM Print",
"!ASM Print, tyre компания er",
"!ASM Print, рекламно-производственная компания rr",
"!Action Pact!",
"!T.O.O.H.!",
"!YES, лингвистический центр",
"!ts, магазин",
"!ФЕСТ",
'"100-th" department store',
'"1000 мелочей"',
'"1001 мелочь"',
'"19 отряд Федеральной противопожарной службы по Ленинградской области"',
'"У Друзей"',
'"ШТОРЫ и не только..."']

test_sentences_pl = [
    "wąż",  # a snake
    "mąż",  # a husband - why so similar :)
    "żółć",
    "aż",
    "waży"
]


@skipIf(not ahocorasick.unicode, "Run only with unicode build")
def test_create_automata_rus_does_not_crash_unicode():
    A = ahocorasick.Automaton()
    for sentences in test_sentences_rus[-7:]:
        for index, word in enumerate(sentences.split(' ')):
            A.add_word(word, (index, word))

    A.make_automaton()


@skipIf(not ahocorasick.unicode, "Run only with unicode build")
def test_create_and_iter_automata_pl_works_unicode():
    A = ahocorasick.Automaton()
    for index, word in enumerate(test_sentences_pl):
        A.add_word(word, (index, word))

    A.make_automaton()
    results = []
    for item in A.iter("wyważyć"):
        results.append(item)
    assert results == [(4, (3, 'aż')), (5, (4, 'waży'))]


@skipIf(ahocorasick.unicode, "Run only with bytes build")
def test_create_automata_rus_does_not_crash_bytes():
    A = ahocorasick.Automaton()
    for sentences in test_sentences_rus[-7:]:
        for index, word in enumerate(sentences.split(' ')):
            A.add_word(conv(word), (index, word))

    A.make_automaton()


@skipIf(ahocorasick.unicode, "Run only with bytes build")
def test_create_and_iter_automata_pl_works_bytes():
    A = ahocorasick.Automaton()
    for index, word in enumerate(test_sentences_pl):
        A.add_word(conv(word), (index, word))

    A.make_automaton()
    results = []
    for item in A.iter(conv("wyważyć")):
        results.append(item)
    assert results == [(5, (3, 'aż')), (6, (4, 'waży'))]

