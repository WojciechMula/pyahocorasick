# -*- coding: utf-8 -*-

"""
    Aho-Corasick string search algorithm.

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
    License   : public domain
"""

import tempfile
from pathlib import Path
from pickle import load, dump

from ahocorasick import Automaton

from pytestingutils import conv


def test_issue_50():
    auto = Automaton()
    auto.add_word(conv('abc'), 'abc')

    auto.add_word(conv('def'), 'def')
    tempdir = Path(tempfile.mkdtemp(prefix="ahocorasick-test-"))
    pickled = tempdir / 'automaton-wee.pickle'

    with open(pickled, 'wb') as dest:
        dump(auto, dest)

    with open(pickled, 'rb') as src:
        auto2 = load(src)

    assert list(auto.items()) == list(auto2.items())
