import ahocorasick
A = ahocorasick.Automaton()
for index, word in enumerate("he her hers she".split()):
   A.add_word(word, (index, word))
   A.clear()
