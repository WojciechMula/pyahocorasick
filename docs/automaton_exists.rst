exists(key) -> boolean
----------------------------------------------------------------------

Return True if the ``key`` is present in the trie. Same as using the 'in' keyword.

Examples
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code:: python

    >>> import ahocorasick
    >>> A = ahocorasick.Automaton()
    >>> A.add_word("cat", 1)
    True
    >>> A.exists("cat")
    True
    >>> A.exists("dog")
    False
    >>> 'elephant' in A
    False
    >>> 'cat' in A
    True
