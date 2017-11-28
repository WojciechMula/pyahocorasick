import ahocorasick
import _pickle

A = ahocorasick.Automaton()
for i in range(0, 65):
    A.add_word(str(i), (i, i))

path = 'aho'
with open(path, 'wb') as f:
    _pickle.dump(A, f)

with open(path, 'rb') as f:
    _pickle.load(f)

