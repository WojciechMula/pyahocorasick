clear()
----------------------------------------------------------------------

Remove all keys from the trie. This method invalidates all iterators.

Examples
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code:: python

    >>> import ahocorasick
    >>> A = ahocorasick.Automaton()
    >>> A.add_word("cat", 1)
    True
    >>> A.add_word("dog", 2)
    True
    >>> A.add_word("elephant", 3)
    True
    >>> len(A)
    3
    >>> A.clear()
    >>> len(A)
    0
