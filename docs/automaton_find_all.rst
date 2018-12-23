find_all(string, callback, [start, [end]])
----------------------------------------------------------------------

Perform the Aho-Corasick search procedure using the provided input ``string``
and iterate over the matching tuples (``end_index``, ``value``) for keys found
in string.  Invoke the ``callback`` callable for each matching tuple.

The callback callable must accept two positional arguments:
- end_index is the end index in the input string where a trie key string was found.
- value is the value associated with the found key string.

The start and end optional arguments can be used to limit the search to an
input string slice as in string[start:end].

Equivalent to a loop on iter() calling a callable at each iteration.
