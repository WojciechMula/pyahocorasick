# -*- coding: utf-8 -*-
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
    version          = '1.1.0',

    ext_modules      = [module],

    description      = "Aho-Corasick automaton",
    author           = "Wojciech Muła",
    author_email     = "wojciech_mula@poczta.onet.pl",
    maintainer       = "Wojciech Muła",
    maintainer_email = "wojciech_mula@poczta.onet.pl",
    url              = "http://github.com/WojciechMula/pyahocorasick",
    platforms        = ["Linux", "Windows"],
    license          = "BSD (3 clauses)",
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
