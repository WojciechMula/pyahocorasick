# -*- coding: utf-8 -*-
"""
    Aho-Corasick string search algorithm.

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
    License   : public domain
"""

import os
from pathlib import Path

import pytest

import ahocorasick

from pytestingutils import conv
from pytestingutils import on_linux


def get_memory_usage():
    # Linux only
    pid = os.getpid()

    lines = []
    try:
        with open('/proc/%d/status' % pid, 'rt') as f:
            lines = f.readlines()
    except:
        pass

    for line in lines:
        if line.startswith('VmSize'):
            return float(line.split()[1])

    return 0


def build_automaton():
    here = Path(__file__).parent
    with open(here.parent.parent / 'README.rst') as f:
        data = f.read()
        data = conv(data)
        data = data.split()

    ac = ahocorasick.Automaton()
    for i, word in enumerate(data):
        ac.add_word(word, i)

    ac.make_automaton()
    return ac


@pytest.mark.skipif(not on_linux, reason="Works only on linux")
def does_not_leak_memory():
    # This is problematic, some memory leaks are still there
    # so this needs to be investigated and re-enabled after fixing
    # See issues:
    # https://github.com/WojciechMula/pyahocorasick/issues/135
    # https://github.com/WojciechMula/pyahocorasick/issues/198
    ac = build_automaton()
    before = get_memory_usage()

    for i in range(1024):
        s = list(ac.keys())

    after = get_memory_usage()
    assert before == after
