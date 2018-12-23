iter(string, [start, [end]], ignore_white_space=False)
----------------------------------------------------------------------

Perform the Aho-Corasick search procedure using the provided input string.

Return an iterator of tuples (``end_index``, ``value``) for keys found in
string where:

- ``end_index`` is the end index in the input string where a trie key
  string was found.
- ``value`` is the value associated with the found key string.

The ``start`` and ``end`` optional arguments can be used to limit the search
to an input string slice as in ``string[start:end]``.

The ``ignore_white_space`` optional arguments can be used to ignore white
spaces from input string.
