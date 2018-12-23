match(key) -> bool
----------------------------------------------------------------------

Return True if there is a prefix (or key) equal to key present in the trie.

For example if the key 'example' has been added to the trie, then calls to
match('e'), match('ex'), ..., match('exampl') or match('example') all return
True. But exists() is True only when calling exists('example').

Examples
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code:: python

    >>> import ahocorasick
    >>> A = ahocorasick.Automaton()
    >>> A.add_word("example", True)
    True
    >>> A.match("e")
    True
    >>> A.match("ex")
    True
    >>> A.match("exa")
    True
    >>> A.match("exam")
    True
    >>> A.match("examp")
    True
    >>> A.match("exampl")
    True
    >>> A.match("example")
    True
    >>> A.match("examples")
    False
    >>> A.match("python")
    False
