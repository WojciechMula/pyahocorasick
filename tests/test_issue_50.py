
import tempfile
from pathlib import Path
from pickle import load, dump

from ahocorasick import Automaton


def test_issue_50():
    auto = Automaton()
    auto.add_word('abc', 'abc')

    auto.add_word('def', 'def')
    tempdir = Path(tempfile.mkdtemp(prefix="ahocorasick-test-"))
    pickled = tempdir / 'automaton-wee.pickle'

    with open(pickled, 'wb') as dest:
        dump(auto, dest)

    with open(pickled, 'rb') as src:
        auto2 = load(src)

    assert list(auto.items()) == list(auto2.items())
