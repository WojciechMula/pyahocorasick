import ahocorasick

print("Generating dictionary...")
dictionary = (("longstring" * 10 + str(a), "longstring") for a in range(100000000))

print("Adding words to automation...")
automation = ahocorasick.Automaton()
for i, (word, matches) in enumerate(dictionary):
    if i != 0 and i % 100000 == 0:
        print("Processed {} items...".format(i))
    automation.add_word(word, (word, matches))

print("Creating automation...")
automation.make_automaton()
