"""Concurrency and reentrancy regressions for the C extension."""

from collections import Counter
from concurrent.futures import ThreadPoolExecutor
import faulthandler
import os
import pickle
import subprocess
import sys
import sysconfig
from threading import Barrier

import pytest

import ahocorasick
from pytestingutils import conv


@pytest.fixture(autouse=True)
def deadlock_timeout():
    # Future timeouts alone cannot interrupt executor shutdown or a C deadlock.
    faulthandler.dump_traceback_later(60, exit=True)
    try:
        yield
    finally:
        faulthandler.cancel_dump_traceback_later()


def run_parallel(*functions):
    barrier = Barrier(len(functions), timeout=10)

    def run(function):
        barrier.wait()
        return function()

    with ThreadPoolExecutor(max_workers=len(functions)) as pool:
        futures = [pool.submit(run, function) for function in functions]
        return [future.result(timeout=30) for future in futures]


def make_automaton(store=ahocorasick.STORE_ANY):
    automaton = ahocorasick.Automaton(store)
    for i, word in enumerate(("he", "she", "his", "hers")):
        value = {"word": word} if store == ahocorasick.STORE_ANY else i
        automaton.add_word(conv(word), value)
    automaton.make_automaton()
    return automaton


@pytest.mark.skipif(not sysconfig.get_config_var("Py_GIL_DISABLED"),
                    reason="Requires a free-threaded interpreter")
def test_import_does_not_enable_gil():
    assert not sys._is_gil_enabled()
    env = os.environ.copy()
    env.pop("PYTHON_GIL", None)
    subprocess.run(
        [sys.executable, "-Werror::RuntimeWarning", "-c",
         "import sys; assert not sys._is_gil_enabled(); "
         "import ahocorasick; assert not sys._is_gil_enabled()"],
        env=env, check=True, timeout=30,
    )


@pytest.mark.parametrize("store", [ahocorasick.STORE_ANY, ahocorasick.STORE_INTS,
                                   ahocorasick.STORE_LENGTH])
def test_concurrent_add_remove_and_pop(store):
    automaton = ahocorasick.Automaton(store)

    def worker(number):
        for i in range(150):
            key = conv("%d:%03d" % (number, i))
            value = [number, i] if store == ahocorasick.STORE_ANY else i
            expected = len(key) if store == ahocorasick.STORE_LENGTH else value
            assert automaton.add_word(key, value)
            assert automaton.get(key) == expected
            if i % 3 == 0:
                assert automaton.pop(key) == expected
            elif i % 3 == 1:
                assert automaton.remove_word(key)
        return number

    run_parallel(*(lambda n=n: worker(n) for n in range(4)))
    assert len(automaton) == 200
    assert len(list(automaton.items())) == 200
    assert automaton.get_stats()["words_count"] == 200


@pytest.mark.parametrize("store", [ahocorasick.STORE_ANY, ahocorasick.STORE_INTS,
                                   ahocorasick.STORE_LENGTH])
def test_search_while_pickling_and_saving(store, tmp_path):
    automaton = make_automaton(store)
    text = conv("ushers his she hers " * 20)
    expected = list(automaton.iter(text))
    expected_long = list(automaton.iter_long(text))
    expected_items = dict(automaton.items())

    def search():
        for _ in range(60):
            assert list(automaton.iter(text)) == expected
            assert list(automaton.iter_long(text)) == expected_long

    def serialize():
        for _ in range(60):
            restored = pickle.loads(pickle.dumps(automaton))
            assert list(restored.iter(text)) == expected
            assert dict(restored.items()) == expected_items

    def save():
        path = str(tmp_path / "automaton")
        for _ in range(30):
            if store == ahocorasick.STORE_ANY:
                automaton.save(path, pickle.dumps)
            else:
                automaton.save(path)
            restored = ahocorasick.load(path, pickle.loads)
            assert list(restored.iter(text)) == expected
            nodes, edges, failure_links = automaton.dump()
            ids = {node for node, _ in nodes}
            assert all(source in ids and target in ids for source, _, target in edges)
            assert all(source in ids and target in ids for source, target in failure_links)

    run_parallel(search, search, serialize, save)


@pytest.mark.parametrize("method", ["iter", "iter_long", "keys", "values", "items"])
def test_shared_iterator_yields_each_match_once(method):
    automaton = ahocorasick.Automaton()
    words = [conv("%04d" % i) for i in range(500)]
    for i, word in enumerate(words):
        automaton.add_word(word, i)
    automaton.make_automaton()

    def iterator():
        if method in ("iter", "iter_long"):
            return getattr(automaton, method)(conv("|").join(words))
        return getattr(automaton, method)()

    expected = Counter(iterator())
    shared = iterator()
    results = run_parallel(*(lambda: list(shared) for _ in range(4)))
    assert Counter(value for result in results for value in result) == expected


@pytest.mark.parametrize("method", ["iter", "iter_long"])
def test_set_and_next_on_shared_iterator(method):
    automaton = ahocorasick.Automaton()
    for word in ("a", "\U0001f40d"):
        automaton.add_word(conv(word), word)
    automaton.make_automaton()
    shared = getattr(automaton, method)(conv("a"))

    def reset():
        for i in range(500):
            shared.set(conv("a" if i % 2 else "\U0001f40d"), True)

    def consume():
        for _ in range(1000):
            result = next(shared, None)
            if result is not None:
                assert result[1] in ("a", "\U0001f40d")

    run_parallel(reset, consume, consume)
    automaton.clear()
    assert shared.set(conv("a"), True) is None
    with pytest.raises(ValueError, match="automaton has changed"):
        next(shared)


def test_search_and_inspection_during_mutation():
    automaton = make_automaton()
    text = conv("ushers his she hers " * 5)

    def mutate():
        for i in range(200):
            automaton.clear()
            automaton.add_word(conv("he"), i)
            automaton.add_word(conv("she"), i)
            automaton.make_automaton()

    def read():
        for _ in range(300):
            for method in ("iter", "iter_long", "items"):
                try:
                    iterator = getattr(automaton, method)(text) if method != "items" else automaton.items()
                    list(iterator)
                except (AttributeError, ValueError):
                    # Constructing an iterator can race with clear/build, and
                    # mutations invalidate iterators already in progress.
                    pass
            automaton.get(conv("he"), None)
            automaton.exists(conv("he"))
            automaton.match(conv("h"))
            automaton.longest_prefix(conv("hers"))
            automaton.get_stats()
            automaton.__sizeof__()
            automaton.kind
            restored = pickle.loads(pickle.dumps(automaton))
            assert len(list(restored.keys())) == len(restored)

    run_parallel(mutate, read, read)


def test_find_all_callback_can_use_automaton_from_another_thread():
    automaton = make_automaton()
    observed = []
    with ThreadPoolExecutor(max_workers=1) as pool:
        def callback(index, value):
            observed.append(pool.submit(automaton.get, conv(value["word"])).result(timeout=5))

        automaton.find_all(conv("ushers"), callback)
    assert len(observed) == 3


def test_find_all_callback_mutation_invalidates_search():
    automaton = make_automaton()
    with pytest.raises(ValueError, match="automaton has changed"):
        automaton.find_all(conv("ushers"), lambda *args: automaton.clear())


def test_save_callback_can_mutate_original(tmp_path):
    automaton = make_automaton()
    expected = list(automaton.iter(conv("ushers")))
    path = str(tmp_path / "snapshot")
    with ThreadPoolExecutor(max_workers=1) as pool:
        def serialize(value):
            pool.submit(automaton.clear).result(timeout=5)
            return pickle.dumps(value)

        automaton.save(path, serialize)
    restored = ahocorasick.load(path, pickle.loads)
    assert list(restored.iter(conv("ushers"))) == expected
    assert len(automaton) == 0


def test_value_destructors_observe_completed_mutations():
    automaton = ahocorasick.Automaton()
    observed = []

    class Value:
        def __del__(self):
            observed.append((len(automaton), automaton.get(conv("key"), None)))

    automaton.add_word(conv("key"), Value())
    automaton.add_word(conv("key"), "replacement")
    assert observed == [(1, "replacement")]
    automaton.add_word(conv("key"), Value())
    automaton.remove_word(conv("key"))
    assert observed[-1] == (0, None)
    automaton.add_word(conv("key"), Value())
    automaton.clear()
    assert observed[-1] == (0, None)


def test_unpickle_copies_caller_owned_lists():
    automaton = ahocorasick.Automaton()
    automaton.add_word(conv("key"), [0])
    constructor, args = automaton.__reduce__()
    values = args[-1]

    def change_values():
        for i in range(2000):
            values[0] = [i]

    def restore():
        for _ in range(500):
            value = constructor(*args).get(conv("key"))
            assert isinstance(value, list) and len(value) == 1

    run_parallel(change_values, restore, restore)


def test_unpickle_while_caller_clears_bytes_list():
    automaton = make_automaton()
    constructor, args = automaton.__reduce__()
    buffers = args[0]
    original = buffers[0]

    def change_buffers():
        for _ in range(2000):
            buffers.clear()
            buffers.append(original)

    def restore():
        for _ in range(500):
            try:
                restored = constructor(*args)
            except ValueError:
                continue  # A snapshot of the empty list is invalid pickle data.
            assert len(restored) == len(automaton)

    run_parallel(change_buffers, restore, restore)


@pytest.mark.parametrize("method", ["iter", "iter_long"])
def test_sequence_conversion_can_replace_automaton(method):
    automaton = ahocorasick.Automaton(ahocorasick.STORE_ANY, ahocorasick.KEY_SEQUENCE)
    automaton.add_word((1,), "old")
    automaton.make_automaton()

    class Index:
        def __index__(self):
            automaton.clear()
            automaton.add_word((1,), "new")
            automaton.make_automaton()
            return 1

    assert list(getattr(automaton, method)((Index(),))) == [(0, "new")]


@pytest.mark.parametrize("method", ["iter", "iter_long"])
@pytest.mark.parametrize("reset", [False, True])
def test_set_boolean_conversion_can_invalidate_iterator(method, reset):
    automaton = make_automaton()
    iterator = getattr(automaton, method)(conv("ushers"))

    class Reset:
        def __bool__(self):
            automaton.clear()
            return reset

    assert iterator.set(conv("ushers"), Reset()) is None
    with pytest.raises(ValueError, match="automaton has changed"):
        next(iterator)


@pytest.mark.parametrize("method", ["iter", "iter_long"])
@pytest.mark.parametrize("reset", [False, True])
def test_set_preserves_invalidation_until_next(method, reset):
    automaton = make_automaton()
    iterator = getattr(automaton, method)(conv("ushers"))
    automaton.clear()
    assert iterator.set(conv("ushers"), reset) is None
    with pytest.raises(ValueError, match="automaton has changed"):
        next(iterator)


def test_replaced_values_remain_alive_for_readers():
    class Value:
        def __init__(self, number):
            self.number = number

    automaton = ahocorasick.Automaton()
    automaton.add_word(conv("he"), Value(0))
    automaton.make_automaton()
    iterator = automaton.iter(conv("he" * 2000))

    def replace():
        for i in range(2000):
            automaton.add_word(conv("he"), Value(i))

    def search():
        for _, value in iterator:
            assert 0 <= value.number < 2000

    def get():
        for _ in range(2000):
            assert 0 <= automaton.get(conv("he")).number < 2000

    run_parallel(replace, search, get)
