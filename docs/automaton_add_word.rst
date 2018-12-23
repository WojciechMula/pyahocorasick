add_word(key, [value]) -> boolean
--------------------------------------------------------------------------------

Add a key string to the dict-like trie and associate this key with a value.
value is optional or mandatory depending how the ``Automaton`` instance was
created.  Return True if the word key is inserted and did not exists in the
trie or False otherwise. The value associated with an existing word is replaced.

The value is either mandatory or optional:

- If the Automaton was created without argument (the default) as ``Automaton()``
  or with ``Automaton(ahocorasik.STORE_ANY)`` then the value is required and can
  be any Python object.
- If the Automaton was created with ``Automaton(ahocorasik.STORE_INTS)`` then the
  value is optional. If provided it must be an integer, otherwise it defaults to
  ``len(automaton)`` which is therefore the order index in which keys are added
  to the trie.
- If the Automaton was created with ``Automaton(ahocorasik.STORE_LENGTH)`` then
  associating a value is not allowed - ``len(word)`` is saved automatically as
  a value instead.

Calling add_word() invalidates all iterators only if the new key did not exist
in the trie so far (i.e. the method returned True).

Examples
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code:: python

    >>> import ahocorasick
    >>> A = ahocorasick.Automaton()
    >>> A.add_word("pyahocorasick")
    Traceback (most recent call last):
      File "<stdin>", line 1, in <module>
    ValueError: A value object is required as second argument.
    >>> A.add_word("pyahocorasick", (42, 'text'))
    True
    >>> A.get("pyhocorasick")
    (42, 'text')
    >>> A.add_word("pyahocorasick", 12)
    False
    >>> A.get("pyhocorasick")
    12

.. code:: python

    >>> import ahocorasick
    >>> B = ahocorasick.Automaton(ahocorasick.STORE_INTS)
    >>> B.add_word("cat")
    True
    >>> B.get()
    Traceback (most recent call last):
      File "<stdin>", line 1, in <module>
    IndexError: tuple index out of range
    >>> B.get("cat")
    1
    >>> B.add_word("dog")
    True
    >>> B.get("dog")
    2
    >>> B.add_word("tree", 42)
    True
    >>> B.get("tree")
    42
    >>> B.add_word("cat", 43)
    False
    >>> B.get("cat")
    43
