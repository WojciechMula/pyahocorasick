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

from sys import version_info as python_version


def get_long_description():
    """
    Strip the content index from the long description.
    """
    import codecs 
    with codecs.open('README.rst', encoding='UTF-8') as f:
        readme = [line for line in f if not line.startswith('.. contents::')]
        return ''.join(readme)


if python_version.major not in [2, 3]:
    raise ValueError('Python %s is not supported' % python_version)


if python_version.major == 3:
    macros = [
        # when defined unicode strings are supported
        ('AHOCORASICK_UNICODE', ''),
    ]
else:
    # On Python 2, unicode strings are not supported (yet).
    macros = []


module = Extension(
    'ahocorasick',
    sources=[
        'pyahocorasick.c',
    ],
    define_macros=macros,
    depends=[
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
        'msinttypes/stdint.h',
        'src/inline_doc.h',
        'src/pickle/pickle.h',
        'src/pickle/pickle_data.h',
        'src/pickle/pickle_data.c',
        'src/custompickle/custompickle.h',
        'src/custompickle/custompickle.c',
        'src/custompickle/pyhelpers.h',
        'src/custompickle/pyhelpers.c',
        'src/custompickle/save/automaton_save.h',
        'src/custompickle/save/automaton_save.c',
        'src/custompickle/save/savebuffer.h',
        'src/custompickle/save/savebuffer.c',
        'src/custompickle/load/module_automaton_load.h',
        'src/custompickle/load/module_automaton_load.c',
        'src/custompickle/load/loadbuffer.h',
        'src/custompickle/load/loadbuffer.c',
        'src/pycallfault/pycallfault.h',
        'src/pycallfault/pycallfault.c',
    ],
)


setup(
    name='pyahocorasick',
    version='1.4.0',
    ext_modules=[module],

    description=(
        'pyahocorasick is a fast and memory efficient library for exact or '
        'approximate multi-pattern string search.  With the ahocorasick.Automaton '
        'class, you can find multiple key strings occurrences at once in some input '
        'text.  You can use it as a plain dict-like Trie or convert a Trie to an '
        'automaton for efficient Aho-Corasick search.  Implemented in C and tested '
        'on Python 2.7 and 3.4+.  Works on Linux, Mac and Windows. BSD-3-clause license.'
    ),
    author='Wojciech Muła',
    author_email='wojciech_mula@poczta.onet.pl',
    maintainer='Wojciech Muła',
    maintainer_email='wojciech_mula@poczta.onet.pl',
    url='http://github.com/WojciechMula/pyahocorasick',
    platforms=['Linux', 'MacOSX', 'Windows'],
    license=' BSD-3-Clause and Public-Domain',
    long_description=get_long_description(),
    long_description_content_type="text/x-rst",
    keywords=[
        'aho-corasick',
        'trie',
        'automaton',
        'dictionary',
    ],
    classifiers=[
        'Development Status :: 5 - Production/Stable',
        'License :: OSI Approved :: BSD License',
        'Programming Language :: C',
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 3',
        'Topic :: Software Development :: Libraries',
        'Topic :: Text Editors :: Text Processing',
    ],
)
