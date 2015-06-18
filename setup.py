# -*- coding: utf-8 -*-
from distutils.core import setup, Extension

def get_readme():
    with open('README.rst', 'rt') as f:
        return f.read()


module = Extension(
    'ahocorasick',
    sources = [
        'pyahocorasick.c'
    ],
    define_macros = [
        ('AHOCORASICK_UNICODE', ''),    # when defined unicode strings are supported
    ],
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
    version          = '1.0.0',

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
        "Programming Language :: Python :: 3",
        "Topic :: Software Development :: Libraries",
        "Topic :: Text Editors :: Text Processing",
    ],
)
