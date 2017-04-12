import pickle
import time

import ahocorasick
import psutil
import requests

automaton = ahocorasick.Automaton()

r = requests.get('https://en.wikipedia.org/wiki/Aho%E2%80%93Corasick_algorithm')
assert r.ok

for word in r.text.split():
    if word.isalpha():
        automaton.add_word(word.encode('utf8').lower(), word)

automaton.make_automaton()
pickled = pickle.dumps(automaton)

print('Cycles    Free MiB')
for i in range(10000):
    if i % 1000 == 0:
        free = psutil.virtual_memory().free
        print('{:05d}     {}'.format(i, free/1000000))
    unpickled = pickle.loads(pickled)
    time.sleep(0.001)
