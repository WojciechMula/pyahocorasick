
.. include:: ../README.rst


API Overview
============

This is a quick tour of the API for the C **ahocorasick** module.
See the full API doc for more details. The pure Python module has a slightly different interface.

The module ``ahocorasick`` contains a few constants and the main ``Automaton`` class.


Module constants
----------------

 - ``ahocorasick.unicode`` --- see `Unicode and bytes`_

 - ``ahocorasick.STORE_ANY``, ``ahocorasick.STORE_INTS``,
   ``ahocorasick.STORE_LENGTH`` --- see `Automaton class`_

 - ``ahocorasick.KEY_STRING`` ``ahocorasick.KEY_SEQUENCE``
   --- see `Automaton class`_

 - ``ahocorasick.EMPTY``, ``ahocorasick.TRIE``, ``ahocorasick.AHOCORASICK``
   --- see `Automaton Attributes`_

 - ``ahocorasick.MATCH_EXACT_LENGTH``, ``ahocorasick.MATCH_AT_MOST_PREFIX``,
   ``ahocorasick.MATCH_AT_LEAST_PREFIX`` --- see description of the keys method


Automaton class
---------------

Note: ``Automaton`` instances are `pickle-able <https://docs.python.org/3/library/pickle.html>`_
meaning that you can create ahead of time an eventually large automaton then save it to disk
and re-load it later to reuse it over and over as a persistent multi-string search index.
Internally, Automaton implements the ``__reduce__() magic method``.


``Automaton([value_type], [key_type])``

    Create a new empty Automaton optionally passing a `value_type` to indicate
    what is the type of associated values (default to any Python object type).
    It can be one of ``ahocorasick.STORE_ANY``, ``ahocorasick.STORE_INTS`` or
    ``ahocorasick.STORE_LENGTH``. In the last case the length of the key will
    be stored in the automaton. The optional argument `key_type` can be
    ``ahocorasick.KEY_STRING`` or ``ahocorasick.KEY_SEQUENCE``. In the latter
    case keys will be tuples of integers. The size of integer depends on the
    version and platform Python is running on, but for versions of Python >=
    3.3, it is guaranteed to be 32-bits.

Automaton Trie methods
----------------------

The Automaton class has the following main trie-like methods:

``add_word(key, [value]) => bool``
    Add a ``key`` string to the dict-like trie and associate this key with a ``value``.

``remove_word(key) => bool``
    Remove a ``key`` string from the dict-like trie.

``pop(key) => value``
    Remove a ``key`` string from the dict-like trie and return the associated ``value``.

``exists(key) => bool`` or ``key in ...``
    Return True if the key is present in the trie.

``match(key) => bool``
    Return True if there is a prefix (or key) equal to ``key`` present in the trie.


Automaton Dictionary-like methods
---------------------------------

A pyahocorasick Automaton trie behaves more or less like a Python dictionary and
implements a subset of dict-like methods. Some of them are:

``get(key[, default])``
    Return the value associated with the ``key`` string. Similar to `dict.get()`.

``keys([prefix, [wildcard, [how]]]) => yield strings``
    Return an iterator on keys.

``values([prefix, [wildcard, [how]]]) => yield object``
    Return an iterator on values associated with each keys.

``items([prefix, [wildcard, [how]]]) => yield tuple (string, object)``
    Return an iterator on tuples of (key, value).

Wildcard search
~~~~~~~~~~~~~~~

The methods ``keys``, ``values`` and ``items`` can be called with an optional
**wildcard**. A wildcard character is equivalent to a question mark used in glob
patterns (?) or a dot (.) in regular expressions. You can use any character you
like as a wildcard.

Note that it is not possible to escape a wildcard to match it exactly.
You need instead to select another wildcard character not present in the
provided prefix. For example::

    automaton.keys("hi?", "?")  # would match "him", "his"
    automaton.keys("XX?", "X")  # would match "me?", "he?" or "it?"

Aho-Corasick methods
--------------------

The Automaton class has the following main Aho-Corasick methods:

``make_automaton()``
    Finalize and create the Aho-Corasick automaton.

``iter(string, [start, [end]])``
    Perform the Aho-Corasick search procedure using the provided input ``string``.
    Return an iterator of tuples (end_index, value) for keys found in string.

AutomatonSearchIter class
~~~~~~~~~~~~~~~~~~~~~~~~~

Instances of this class are returned by the ``iter`` method of an ``Automaton``.
This iterator can be manipulated through its `set()` method.

``set(string, [reset]) => None``
    Set a new string to search eventually keeping the current Automaton state to
    continue searching for the next chunk of a string.

    For example::

        >>> it = A.iter(b"")
        >>> while True:
        ...     buffer = receive(server_address, 4096)
        ...     if not buffer:
        ...         break
        ...     it.set(buffer)
        ...     for index, value in it:
        ...         print(index, '=>', value)

    When ``reset`` is ``True`` then processing is restarted. For example this code::

        >>> for string in string_set:
        ...     for index, value in A.iter(string)
        ...         print(index, '=>', value)

    does the same job as::

        >>> it = A.iter(b"")
        >>> for string in string_set:
        ...     it.set(it, True)
        ...     for index, value in it:
        ...         print(index, '=>', value)


Automaton Attributes
--------------------

The Automaton class has the following attributes:

``kind`` [readonly]
    Return the state of the ``Automaton`` instance.

``store`` [readonly]
    Return the type of values stored in the Automaton as specified at creation.


Saving and loading automaton
----------------------------

There is support for two method of saving and loading an automaton:

* the standard ``pickle`` protocol,
* custom ``save`` and ``load`` methods.

While pickling is more convenient to use, it has quite high memory
requirements. The ``save``/``load`` method try to overcome this
problem.

.. warning::

    Neither format of pickle nor save are safe. Although there are
    a few sanity checks, they are not sufficient to detect all
    possible input errors.


Pickle
~~~~~~

.. code:: python

    import ahocorasick
    import pickle

    # build automaton

    A = ahocorasick.Automaton()
    # ... A.add_data, A.make_automaton

    # save current state
    with open(path, 'wb') as f:
        pickle.dump(A, f)

    # load saved state
    with open(path, 'rb') as f:
        B = pickle.load(f)


Save/load methods
~~~~~~~~~~~~~~~~~

.. code:: python

    import ahocorasick
    import pickle

    # build automaton

    A = ahocorasick.Automaton()
    # ... A.add_data, A.make_automaton

    # save current state
    A.save(path, pickle.dumps)

    # load saved state
    B = ahocorasick.load(path, pickle.loads)


Automaton method ``save`` requires ``path`` to the file which will store data.
If the automaton type is ``STORE_ANY``, i.e. values associated with words are
any python objects, then ``save`` requires also another argument, a callable.
The callable serializes python object into bytes; in the example above we
use standard pickle ``dumps`` function.

Module method ``load`` also requires ``path`` to file that has data previously
saved. Because at the moment of loading data we don't know what is the store
attribute of automaton, the second argument - a callable - is required.  The
callable must convert back given bytes object into python value, that will be
stored in automaton. Similarly, standard ``pickle.loads`` function can be passed.


Other Automaton methods
-----------------------

The Automaton class has a few other interesting methods:

``dump() => (list of nodes, list of edges, list of fail links)``
    Return a three-tuple of lists describing the Automaton as a graph of
    (nodes, edges, failure links).
    The source repository and source package also contains the  ``dump2dot.py``
    script that converts ``dump()`` results to a `graphviz <http://graphviz.org>`_ dot
    format for convenient visualization of the trie and Automaton data structure.

``get_stats() => dict``
    Return a dictionary containing Automaton statistics.
    Note that the real size occupied by the data structure could be larger because
    of `internal memory fragmentation <http://en.wikipedia.org/Memory%20fragmentation>`_
    that can occur in a memory manager.

``__sizeof__() => int``
    Return the approximate size in bytes occupied by the Automaton instance.
    Also available by calling sys.getsizeof(automaton instance).


Examples
========

::

    >>> import ahocorasick
    >>> A = ahocorasick.Automaton()

    >>> # add some key words to trie
    >>> for index, word in enumerate('he her hers she'.split()):
    ...   A.add_word(word, (index, word))

    >>> # test that these key words exists in the trie all right
    >>> 'he' in A
    True
    >>> 'HER' in A
    False
    >>> A.get('he')
    (0, 'he')
    >>> A.get('she')
    (3, 'she')
    >>> A.get('cat', '<not exists>')
    '<not exists>'
    >>> A.get('dog')
    Traceback (most recent call last):
      File "<stdin>", line 1, in <module>
    KeyError
    >>> A.remove_word('he')
    True
    >>> A.remove_word('he')
    False
    >>> A.pop('she')
    (3, 'she')
    >>> 'she' in A
    False

    >>> # convert the trie in an Aho-Corasick automaton
    >>> A = ahocorasick.Automaton()
    >>> for index, word in enumerate('he her hers she'.split()):
    ...   A.add_word(word, (index, word))
    >>> A.make_automaton()

    >>> # then find all occurrences of the stored keys in a string
    >>> for item in A.iter('_hershe_'):
    ...  print(item)
    ...
    (2, (0, 'he'))
    (3, (1, 'her'))
    (4, (2, 'hers'))
    (6, (3, 'she'))
    (6, (0, 'he'))


Example of the keys method behavior
-----------------------------------

::

    >>> import ahocorasick
    >>> A = ahocorasick.Automaton()

    >>> # add some key words to trie
    >>> for index, word in enumerate('cat catastropha rat rate bat'.split()):
    ...   A.add_word(word, (index, word))

    >>> # Search some prefix
    >>> list(A.keys('cat'))
    ['cat', 'catastropha']

    >>> # Search with a wildcard: here '?' is used as a wildcard. You can use any character you like.
    >>> list(A.keys('?at', '?', ahocorasick.MATCH_EXACT_LENGTH))
    ['bat', 'cat', 'rat']

    >>> list(A.keys('?at?', '?', ahocorasick.MATCH_AT_MOST_PREFIX))
    ['bat', 'cat', 'rat', 'rate']

    >>> list(A.keys('?at?', '?', ahocorasick.MATCH_AT_LEAST_PREFIX))
    ['rate']


API Reference
=============

.. include:: automaton_constructor.rst
.. include:: automaton_add_word.rst
.. include:: automaton_exists.rst
.. include:: automaton_get.rst
.. include:: automaton_longest_prefix.rst
.. include:: automaton_match.rst
.. include:: automaton_len.rst
.. include:: automaton_remove_word.rst
.. include:: automaton_pop.rst
.. include:: automaton_clear.rst
.. include:: automaton_keys.rst
.. include:: automaton_items.rst
.. include:: automaton_values.rst
.. include:: automaton_make_automaton.rst
.. include:: automaton_iter.rst
.. include:: automaton_find_all.rst
.. include:: automaton___reduce__.rst
.. include:: automaton_save.rst
.. include:: module_load.rst
.. include:: automaton___sizeof__.rst
.. include:: automaton_get_stats.rst
.. include:: automaton_dump.rst
.. include:: automaton_search_iter_set.rst

