# -*- coding: utf-8 -*-
"""
    Aho-Corasick string search algorithm.

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
    License   : public domain
"""

import os
import sys

import ahocorasick


try:
    range = xrange # for Py2
except NameError:
    pass

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

def test():
    with open('README.rst', 'r') as f:
        data = f.read().split()

    ac = ahocorasick.Automaton()
    for i, word in enumerate(data):
        ac.add_word(word, i)

    ac.make_automaton()

    for i in range(1024):
        s = list(ac.keys())


if __name__ == '__main__':

    before = get_memory_usage()
    test()
    after = get_memory_usage()

    print("Memory's usage growth: %s (before = %s, after = %s)" % (after - before, before, after))
    assert(before == after)

