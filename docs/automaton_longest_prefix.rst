longest_prefix(string) => integer
----------------------------------------------------------------------

Return the length of the longest prefix of string that exists in the trie.

Examples
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code:: python

    >>> import ahocorasick
    >>> A = ahocorasick.Automaton()
    >>> A.add_word("he", True)
    True
    >>> A.add_word("her", True)
    True
    >>> A.add_word("hers", True)
    True
    >>> A.longest_prefix("she")
    0
    >>> A.longest_prefix("herself")
    4
