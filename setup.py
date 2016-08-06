# -*- coding: utf-8 -*-

"""
    Aho-Corasick string search algorithm.

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
    License   : BSD-3-Clause (see LICENSE)
"""

try:
    from setuptools import setup, Extension
except ImportError:
    from distutils.core import setup, Extension

from sys import version_info as version


def get_readme():
    with open('README.rst', 'rt') as f:
        body = f.read()

        marker1 = '.. image'
        marker2 = '.. contents'
        
        s = body.index(marker1)
        e = body.index(marker2)

        body = body[:s] + body[e:]

        return body


if version.major not in [2, 3]:
    raise ValueError("Python %s is not supported" % version)

if version.major == 3:
    macros = [
        ('AHOCORASICK_UNICODE', ''),    # when defined unicode strings are supported
    ]
else:
    macros = []


module = Extension(
    'ahocorasick',
    sources = [
        'pyahocorasick.c'
    ],
    define_macros = macros,
    depends = [
        'common.h',
        'Automaton.c',
        'Automaton.h',
        'Automaton_pickle.c',
        'AutomatonItemsIter.c',
        'AutomatonItemsIter.h',
        'AutomatonSearchIter.c',
        'AutomatonSearchIter.h',
        'trie.c',
        'trie.h',
        'slist.c',
        'utils.c',
        'trienode.c',
        'trienode.h',
    ],
)


setup(
    name             = 'pyahocorasick',
    version          = '1.1.2',

    ext_modules      = [module],

    description      = ("pyahocorasick is a fast and memory efficient library for "
                        "fast exact or approximate multi-pattern string search. "
                        "It is implemented in C and tested on Python 2.7 and 3.4+."),
    author           = "Wojciech Muła",
    author_email     = "wojciech_mula@poczta.onet.pl",
    maintainer       = "Wojciech Muła",
    maintainer_email = "wojciech_mula@poczta.onet.pl",
    url              = "http://github.com/WojciechMula/pyahocorasick",
    platforms        = ["Linux", "MacOSX", "Windows"],
    license          = " BSD-3-Clause and Public-Domain",
    long_description = get_readme(),
    keywords         = [
        "aho-corasick",
        "trie",
        "automaton",
        "dictionary",
    ],
    classifiers      = [
        "Development Status :: 5 - Production/Stable",
        "License :: OSI Approved :: BSD License",
        "Programming Language :: C",
        "Programming Language :: Python :: 2",
        "Programming Language :: Python :: 3",
        "Topic :: Software Development :: Libraries",
        "Topic :: Text Editors :: Text Processing",
    ],
)
