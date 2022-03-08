# -*- coding: utf-8 -*-
"""
    Aho-Corasick string search algorithm.

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
    License   : public domain
"""

import os
import sys
import unittest

from pathlib import Path

import ahocorasick

from pytestingutils import conv


on_linux = str(sys.platform).lower().startswith('linux')


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


def use_memory():
    here = Path(__file__).parent
    with open(here.parent.parent / 'README.rst') as f:
        data = f.read()
        data = conv(data)
        data = data.split()

    ac = ahocorasick.Automaton()
    for i, word in enumerate(data):
        ac.add_word(word, i)

    ac.make_automaton()

    for i in range(1024):
        s = list(ac.keys())


class TestMemory(unittest.TestCase):

    @unittest.skipIf(not on_linux, "Works only on linux")
    def test_does_not_leak_memory(self):
        before = get_memory_usage()
        use_memory()
        after = get_memory_usage()
        assert before == after
