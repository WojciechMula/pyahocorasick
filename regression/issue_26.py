import ahocorasick as aho
a=aho.Automaton(aho.STORE_INTS)
a.add_word('abc', 12)
a.make_automaton()
import pickle
p=pickle.dumps(a)
