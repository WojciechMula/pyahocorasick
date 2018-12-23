remove_word(word) -> bool
--------------------------------------------------------------------------------

Remove given word from a trie. Return True if words was found, False otherwise.

Examples
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code:: python

    >>> import ahocorasick
    >>> A = ahocorasick.Automaton()
    >>> A.add_word("cat", 1)
    True
    >>> A.add_word("dog", 2)
    True
    >>> A.remove_word("cat")
    True
    >>> A.remove_word("cat")
    False
    >>> A.remove_word("dog")
    True
    >>> A.remove_word("dog")
    False
    >>> 
