import ahocorasick

ac = ahocorasick.Automaton()
ac.add_word('S', 1)
ac.make_automaton()
buffer = 'SSS'
ac.iter(buffer, 0, 3) # this causes an error
ac.iter(buffer, 0, 2) # no error, but it misses the last 'S' in the buffer
