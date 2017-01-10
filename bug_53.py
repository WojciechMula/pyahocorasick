import ahocorasick

a = ahocorasick.Automaton()
a.add_word('punched', 'punched')

a.make_automaton()

test_string = 'punched 🙈 punched?!'

for item in a.iter(test_string):
    start = item[0] - len(item[1]) + 1
    print(item, test_string[start:item[0] + 1])
