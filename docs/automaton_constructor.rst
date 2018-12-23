Automaton(value_type=ahocorasick.STORE_ANY, [key_type])
--------------------------------------------------------------------------------

Create a new empty Automaton. Both ``value_type`` and ``key_type`` are optional.

``value_type`` is one of these constants:

- ahocorasick.STORE_ANY [*default*] : The associated value can be any Python object.
- ahocorasick.STORE_LENGTH : The length of an added string key is automatically
  used as the associated value stored in the trie for that key.
- ahocorasick.STORE_INTS : The associated value must be a 32-bit integer.

``key_type`` defines the type of data that can be stored in an automaton; it is one of
these constants and defines type of data might be stored:

- ahocorasick.KEY_STRING [*default*] : string
- ahocorasick.KEY_SEQUENCE : sequences of integers; The size of integer depends
  the version and platform Python, but for versions of Python >= 3.3, it is
  guaranteed to be 32-bits.


Examples
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code:: python

    >>> import ahocorasick
    >>> A = ahocorasick.Automaton()
    >>> A
    <ahocorasick.Automaton object at 0x7f1da1bdf7f0>
    >>> B = ahocorasick.Automaton(ahocorasick.STORE_ANY)
    >>> B
    <ahocorasick.Automaton object at 0x7f1da1bdf6c0>
    >>> C = ahocorasick.Automaton(ahocorasick.STORE_INTS, ahocorasick.KEY_STRING)
    >>> C
    <ahocorasick.Automaton object at 0x7f1da1527f10>
