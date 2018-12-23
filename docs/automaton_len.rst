len() -> integer
----------------------------------------------------------------------

Return the number of distinct keys added to the trie.

Examples
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code:: python

    >>> import ahocorasick
    >>> A = ahocorasick.Automaton()
    >>> len(A)
    0
    >>> A.add_word("python", 1)
    True
    >>> len(A)
    1
    >>> A.add_word("elephant", True)
    True
    >>> len(A)
    2
