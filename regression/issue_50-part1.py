from ahocorasick import Automaton
from pickle import load, dump

auto = Automaton()
auto.add_word('abc', 'abc')

auto.add_word('def', 'def')

with open('automaton-wee.pickle', 'wb') as dest:
  dump(auto, dest)
