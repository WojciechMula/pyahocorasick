from ahocorasick import Automaton
from pickle import load, dump

with open('automaton-wee.pickle', 'rb') as src:
  auto = load(src)
