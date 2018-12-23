pop(word)
--------------------------------------------------------------------------------

Remove given word from a trie and return associated values. Raise a ``KeyError``
if the word was not found.

Examples
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code:: python

    >>> import ahocorasick
    >>> A = ahocorasick.Automaton()
    >>> A.add_word("cat", 1)
    True
    >>> A.add_word("dog", 2)
    True
    >>> A.pop("elephant")
    Traceback (most recent call last):
      File "<stdin>", line 1, in <module>
    KeyError
    >>> A.pop("cat")
    1
    >>> A.pop("dog")
    2
    >>> A.pop("cat")
    Traceback (most recent call last):
      File "<stdin>", line 1, in <module>
    KeyError
