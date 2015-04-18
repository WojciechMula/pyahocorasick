import ahocorasick

A = ahocorasick.Automaton()

# add some words to trie
for index, word in enumerate("he her hers she".split()):
    A.add_word(word, (index, word))

A = None #### segfault here
