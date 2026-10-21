# -*- coding: utf-8 -*-

"""
    Aho-Corasick string search algorithm.

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl
    License   : public domain
"""

import os
import subprocess
import sys

import pytest

import ahocorasick

from pytestingutils import conv, on_windows


@pytest.mark.parametrize("key_type", [ahocorasick.KEY_STRING, ahocorasick.KEY_SEQUENCE])
@pytest.mark.parametrize("start,end,indexes", [
    (-1, -1, [0, 1, 2]), (0, -1, [0, 1, 2]), (-1, 3, [0, 1, 2]),
    (1, -1, [1, 2]), (0, 0, []), (2, 1, []), (0, -2, []), (8, -1, []),
])
def test_iter_bounds_preserve_existing_behavior(key_type, start, end, indexes):
    convert = conv if key_type == ahocorasick.KEY_STRING else lambda word: tuple(map(ord, word))
    automaton = ahocorasick.Automaton(ahocorasick.STORE_ANY, key_type)
    automaton.add_word(convert("a"), 1)
    automaton.make_automaton()
    text = convert("aaa")
    expected = [(index, 1) for index in indexes]
    assert list(automaton.iter(text, start, end)) == expected
    assert list(automaton.iter(text, start=start, end=end)) == expected
    assert list(automaton.iter(convert(""), start, end)) == []


@pytest.mark.parametrize("start,end", [(-2**31, 2**31 - 1), (0, 2**31 - 1)])
def test_iter_bounds_cannot_read_outside_input(start, end):
    automaton = ahocorasick.Automaton()
    automaton.add_word(conv("a"), 1)
    automaton.make_automaton()
    assert list(automaton.iter(conv("aaa"), start, end)) == [(0, 1), (1, 1), (2, 1)]
    assert list(automaton.iter(conv(""), start, end)) == []


@pytest.mark.parametrize("bound", ["start", "end"])
def test_iter_bound_overflow_is_unchanged(bound):
    automaton = ahocorasick.Automaton()
    automaton.add_word(conv("a"), 1)
    automaton.make_automaton()
    with pytest.raises(OverflowError):
        automaton.iter(conv("aaa"), **{bound: 2**40})


@pytest.mark.parametrize("value", [0, sys.maxsize, -sys.maxsize - 1])
def test_integer_values_keep_full_width(value):
    automaton = ahocorasick.Automaton(ahocorasick.STORE_INTS)
    key = conv("key")
    automaton.add_word(key, value)
    automaton.make_automaton()
    assert automaton.get(key) == value
    assert list(automaton.values()) == [value]
    assert list(automaton.items()) == [(key, value)]
    assert list(automaton.iter(key)) == [(len(key) - 1, value)]
    assert list(automaton.iter_long(key)) == [(len(key) - 1, value)]
    matches = []
    automaton.find_all(key, lambda index, item: matches.append((index, item)))
    assert matches == [(len(key) - 1, value)]
    assert automaton.pop(key) == value


@pytest.mark.skipif(ahocorasick.unicode, reason="Run only with bytes build")
@pytest.mark.xfail(reason="Fails on Windows")
def test_basic_bytes():
    automaton = ahocorasick.Automaton()
    words = b"he e hers his she hi him man he".split()
    #         0 1    2   3   4  5   6   7  8
    for i, w in enumerate(words):
        automaton.add_word(w, (i, w))
    query = b"he rshershidamanza "
    #        01234567890123
    automaton.make_automaton()

    assert query[2:8] == b' rsher'
    results = list(automaton.iter(string=query, start=2, end=8))
    assert results == [(6, (4, b'she')), (6, (8, b'he')), (6, (1, b'e'))]

    res = []

    def callback(index, item):
        res.append(dict(index=index, item=item))

    assert query[2:11] == b' rshershi'
    automaton.find_all(query, callback, 2, 11)

    expected = [
        {'index': 6, 'item': (4, b'she')},
        {'index': 6, 'item': (8, b'he')},
        {'index': 6, 'item': (1, b'e')},
        {'index': 8, 'item': (2, b'hers')},
        {'index': 10, 'item': (5, b'hi')},
    ]

    assert res == expected


@pytest.mark.skipif(ahocorasick.unicode, reason="Run only with bytes build")
def test_basic_items_keys_and_values_with_bytes_build():
    automaton = ahocorasick.Automaton()
    words = b"he e hers his she hi him man he".split()
    #         1  2 3    4   5   6  7   8   9
    for i, w in enumerate(words, 1):
        automaton.add_word(w, (i, w))

    expected_keys = [b'e', b'hers', b'his', b'she', b'hi', b'him', b'man', b'he', ]
    expected_values = [
        # the second addition munges this by design, like in a dict (1, b'he'),
        (2, b'e'),
        (3, b'hers'),
        (4, b'his'),
        (5, b'she'),
        (6, b'hi'),
        (7, b'him'),
        (8, b'man'),
        (9, b'he'),
    ]

    expected_items = [
        # the second addition munges this by design, like in a dict  (b'he', (1, b'he')),
        (b'e', (2, b'e')),
        (b'hers', (3, b'hers')),
        (b'his', (4, b'his')),
        (b'she', (5, b'she')),
        (b'hi', (6, b'hi')),
        (b'him', (7, b'him')),
        (b'man', (8, b'man')),
        (b'he', (9, b'he')),
    ]

    assert sorted(automaton.keys()) == sorted(expected_keys)
    assert sorted(automaton.values()) == sorted(expected_values)
    assert sorted(dict(automaton.items()).values()) == sorted(expected_values)
    assert sorted(dict(automaton.items()).keys()) == sorted(expected_keys)

    automaton.make_automaton()

    assert sorted(automaton.keys()) == sorted(expected_keys)
    assert sorted(automaton.values()) == sorted(expected_values)
    assert sorted(dict(automaton.items()).values()) == sorted(expected_values)
    assert sorted(dict(automaton.items()).keys()) == sorted(expected_keys)
    assert sorted(automaton.items()) == sorted(expected_items)


@pytest.mark.skipif(not ahocorasick.unicode, reason="Run only with unicode build")
@pytest.mark.xfail(on_windows, reason="Fails on Windows")
def test_basic_unicode():
    automaton = ahocorasick.Automaton()
    words = "he e hers his she hi him man he".split()
    #        0  1 2    3   4   5  6   7   8
    for i, w in enumerate(words):
        automaton.add_word(w, (i, w))
    query = "he rshershidamanza "
    #        01234567890123
    automaton.make_automaton()

    assert query[2:8] == ' rsher'
    results = list(automaton.iter(string=query, start=2, end=8))
    assert results == [(6, (4, 'she')), (6, (8, 'he')), (6, (1, 'e'))]

    res = []

    def callback(index, item):
        res.append(dict(index=index, item=item))

    assert query[2:11] == ' rshershi'
    automaton.find_all(query, callback, 2, 11)

    expected = [
        {'index': 6, 'item': (4, 'she')},
        {'index': 6, 'item': (8, 'he')},
        {'index': 6, 'item': (1, 'e')},
        {'index': 8, 'item': (2, 'hers')},
        {'index': 10, 'item': (5, 'hi')},
    ]

    assert res == expected


@pytest.mark.skipif(not ahocorasick.unicode, reason="Run only with unicode build")
@pytest.mark.xfail(on_windows, reason="Fails on Windows")
def test_basic_items_keys_and_values_with_unicode_build():
    automaton = ahocorasick.Automaton()
    words = 'he e hers his she hi him man he'.split()
    #         0 1    2   3   4  5   6   7  8
    for i, w in enumerate(words):
        automaton.add_word(w, (i, w))

    expected_keys = ['man', 'she', 'e', 'hi', 'him', 'his', 'he', 'hers']

    expected_values = [
        (7, 'man'),
        (4, 'she'),
        (1, 'e'),
        (5, 'hi'),
        (6, 'him'),
        (3, 'his'),
        (8, 'he'),
        (2, 'hers'),
    ]

    assert sorted(automaton.keys()) == sorted(expected_keys)
    assert sorted(automaton.values()) == sorted(expected_values)
    assert sorted(dict(automaton.items()).values()) == sorted(expected_values)
    assert sorted(dict(automaton.items()).keys()) == sorted(expected_keys)

    automaton.make_automaton()

    assert sorted(automaton.keys()) == sorted(expected_keys)
    assert sorted(automaton.values()) == sorted(expected_values)
    assert sorted(dict(automaton.items()).values()) == sorted(expected_values)
    assert sorted(dict(automaton.items()).keys()) == sorted(expected_keys)


@pytest.mark.skipif(os.name != "posix", reason="Requires POSIX file-size limits")
@pytest.mark.parametrize("limit,size", [(0, 0), (1024, 100000)])
def test_save_reports_write_errors(tmp_path, limit, size):
    # Exercise errors during serialization and while closing buffered output.
    # Keep signal handlers and resource limits in a separate process.
    script = '''
import resource
import signal
import sys
import ahocorasick

signal.signal(signal.SIGXFSZ, signal.SIG_IGN)
original_limits = resource.getrlimit(resource.RLIMIT_FSIZE)
resource.setrlimit(resource.RLIMIT_FSIZE, (int(sys.argv[2]), original_limits[1]))
automaton = ahocorasick.Automaton()
automaton.add_word("key" if ahocorasick.unicode else b"key", None)
try:
    automaton.save(sys.argv[1], lambda value: bytes(int(sys.argv[3])))
except OSError:
    pass
else:
    raise AssertionError("save() did not report the write failure")
finally:
    resource.setrlimit(resource.RLIMIT_FSIZE, original_limits)
'''
    subprocess.run([sys.executable, "-c", script, str(tmp_path / "automaton"),
                    str(limit), str(size)], check=True, timeout=30)
