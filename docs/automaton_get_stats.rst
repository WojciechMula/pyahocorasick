get_stats() -> dict
----------------------------------------------------------------------

Return a dictionary containing Automaton statistics.

- *nodes_count*  - total number of nodes
- *words_count*  - number of distinct words (same as ``len(automaton)``)
- *longest_word* - length of the longest word
- *links_count*  - number of edges
- *sizeof_node*  - size of single node in bytes
- *total_size*   - total size of trie in bytes (about
  nodes_count * size_of node + links_count * size of pointer).

Examples
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code:: python

    >>> import ahocorasick
    >>> A = ahocorasick.Automaton()
    >>> A.add_word("he", None)
    True
    >>> A.add_word("her", None)
    True
    >>> A.add_word("hers", None)
    True
    >>> A.get_stats()
    {'nodes_count': 5, 'words_count': 3, 'longest_word': 4, 'links_count': 4, 'sizeof_node': 40, 'total_size': 232}
