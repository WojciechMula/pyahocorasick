# -*- coding: utf-8 -*-

"""
Aho-Corasick string search algorithm.

Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
WWW       : http://0x80.pl
License   : BSD-3-Clause (see LICENSE)
"""

import os

from setuptools import setup
from setuptools import Extension

# check for presence of environment variables
build_as_bytes = "AHOCORASICK_BYTES" in os.environ
build_as_unicode = "AHOCORASICK_UNICODE" in os.environ

if build_as_unicode and build_as_bytes:
    raise Exception(
        "Cannot build pyahocorasick both as bytes and unicode: "
        "only one of AHOCORASICK_UNICODE and AHOCORASICK_BYTES environment "
        "variable should be set."
    )

# we default to unicode
elif not build_as_unicode and not build_as_bytes:
    build_as_unicode = True

if build_as_unicode:
    macros = [("AHOCORASICK_UNICODE", "")]
else:
    macros = []


def get_long_description():
    import io
    with io.open("README.rst", encoding="UTF-8") as f:
        return f.read()


module = Extension(
    "ahocorasick",
    sources=[
        "src/pyahocorasick.c",
    ],
    define_macros=macros,
    depends=[
        "src/common.h",
        "src/Automaton.c",
        "src/Automaton.h",
        "src/Automaton_pickle.c",
        "src/AutomatonItemsIter.c",
        "src/AutomatonItemsIter.h",
        "src/AutomatonSearchIter.c",
        "src/AutomatonSearchIter.h",
        "src/AutomatonSearchIterLong.c",
        "src/AutomatonSearchIterLong.h",
        "src/trie.c",
        "src/trie.h",
        "src/slist.c",
        "src/utils.c",
        "src/trienode.c",
        "src/trienode.h",
        "src/msinttypes/stdint.h",
        "src/inline_doc.h",
        "src/pickle/pickle.h",
        "src/pickle/pickle_data.h",
        "src/pickle/pickle_data.c",
        "src/custompickle/custompickle.h",
        "src/custompickle/custompickle.c",
        "src/custompickle/pyhelpers.h",
        "src/custompickle/pyhelpers.c",
        "src/custompickle/save/automaton_save.h",
        "src/custompickle/save/automaton_save.c",
        "src/custompickle/save/savebuffer.h",
        "src/custompickle/save/savebuffer.c",
        "src/custompickle/load/module_automaton_load.h",
        "src/custompickle/load/module_automaton_load.c",
        "src/custompickle/load/loadbuffer.h",
        "src/custompickle/load/loadbuffer.c",
        "src/pycallfault/pycallfault.h",
        "src/pycallfault/pycallfault.c",
    ],
)

setup(
    name="pyahocorasick",
    version="2.0.0",
    ext_modules=[module],

    description=(
        "pyahocorasick is a fast and memory efficient library for exact or "
        "approximate multi-pattern string search.  With the ``ahocorasick.Automaton`` "
        "class, you can find multiple key string occurrences at once in some input "
        "text.  You can use it as a plain dict-like Trie or convert a Trie to an "
        "automaton for efficient Aho-Corasick search. And pickle to disk for easy "
        "reuse of large automatons. Implemented in C and tested on Python 3.6+. "
        "Works on Linux, macOS and Windows. BSD-3-Cause license."
    ),
    author="Wojciech Muła",
    author_email="wojciech_mula@poczta.onet.pl",
    maintainer="Wojciech Muła",
    maintainer_email="wojciech_mula@poczta.onet.pl",
    url="http://github.com/WojciechMula/pyahocorasick",
    platforms=["Linux", "MacOSX", "Windows"],
    license=" BSD-3-Clause and Public-Domain",
    long_description=get_long_description(),
    long_description_content_type="text/x-rst",
    keywords=[
        "aho-corasick",
        "trie",
        "automaton",
        "dictionary",
    ],
    classifiers=[
        "Development Status :: 5 - Production/Stable",
        "License :: OSI Approved :: BSD License",
        "Programming Language :: C",
        "Programming Language :: Python :: 3",
        "Topic :: Software Development :: Libraries",
        "Topic :: Text Editors :: Text Processing",
    ],
    extras_require={
        "testing": ["pytest", "twine", "setuptools", "wheel", ],
    },
    python_requires=">=3.6",
)
