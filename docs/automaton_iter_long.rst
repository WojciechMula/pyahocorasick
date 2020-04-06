iter_long(string, [start, [end]])
----------------------------------------------------------------------

Perform the modified Aho-Corasick search procedure which matches
the longest words from set.

Return an iterator of tuples (``end_index``, ``value``) for keys found in
string where:

- ``end_index`` is the end index in the input string where a trie key
  string was found.
- ``value`` is the value associated with the found key string.

The ``start`` and ``end`` optional arguments can be used to limit the search
to an input string slice as in ``string[start:end]``.


Example
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The default Aho-Corasick algorithm returns all occurrences of words stored
in the automaton, including substring of other words from string. Method
``iter_long`` reports only the longest match.

For set of words {"he", "her", "here"} and a needle "he here her" the
default algorithm finds following words: "he", "he", "her", "here", "he",
"her", while the modified one yields only: "he", "here", "her".

.. code:: python

    >>> import ahocorasick
    >>> A = ahocorasick.Automaton()
    >>> A.add_word("he", "he")
    True
    >>> A.add_word("her", "her")
    True
    >>> A.add_word("here", "here")
    True
    >>> A.make_automaton()
    >>> needle = "he here her"
    >>> list(A.iter_long(needle))
    [(1, 'he'), (6, 'here'), (10, 'her')]
    >>> list(A.iter(needle))
    [(1, 'he'), (4, 'he'), (5, 'her'), (6, 'here'), (9, 'he'), (10, 'her')]
