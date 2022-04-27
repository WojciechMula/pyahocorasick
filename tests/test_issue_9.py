# -*- coding: utf-8 -*-
"""
    Aho-Corasick string search algorithm.

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
    License   : public domain
"""

import os
import unittest
from pathlib import Path

import ahocorasick

from pytestingutils import conv
from pytestingutils import on_linux


def build_automaton():
    ac = ahocorasick.Automaton()
    ac.add_word(conv('SSSSS'), 1)
    ac.make_automaton()
    return ac


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


@unittest.skipIf(not on_linux, "Works only on linux")
class MemoryUsageDoesNotGrow(unittest.TestCase):

    def test_memory_usage_does_not_grow(self):

        ac = build_automaton()
        
        here = Path(__file__).parent
        with open(here.parent / 'README.rst') as f:
            data = f.read()[:1024 * 2]
            data = conv(data)

        before = get_memory_usage()

        for _ in range(1000):
            for start in range(0, len(data) - 20):
                ac.iter(data, start)

        after = get_memory_usage()
        assert before == after
