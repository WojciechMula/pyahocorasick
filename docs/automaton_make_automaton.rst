make_automaton()
----------------------------------------------------------------------

Finalize and create the Aho-Corasick automaton based on the keys already added
to the trie. This does not require additional memory. After successful creation
the ``Automaton.kind`` attribute is set to ``ahocorasick.AHOCORASICK``.
