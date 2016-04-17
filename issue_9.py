import ahocorasick
import sys

ac = ahocorasick.Automaton()
ac.add_word('SSSSS', 1)
ac.make_automaton()

with open('README.rst', 'r') as f:
    data = f.read()

for loop in range(1000):
    for start in range(0, len(data) - 20):
        ac.iter(data, start)
        print('.', end='')
        sys.stdout.flush()
