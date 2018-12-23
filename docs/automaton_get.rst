get(key[, default])
----------------------------------------------------------------------

Return the value associated with the key string.

Raise a ``KeyError`` exception if the key is not in the trie and no default is provided.

Return the optional default value if provided and the key is not in the trie.

Example
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code:: python

    >>> import ahocorasick
    >>> A = ahocorasick.Automaton()
    >>> A.add_word("cat", 42)
    True
    >>> A.get("cat")
    42
    >>> A.get("dog")
    Traceback (most recent call last):
      File "<stdin>", line 1, in <module>
    KeyError
    >>> A.get("dog", "good dog")
    'good dog'

