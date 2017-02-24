from ahocorasick import Automaton
auto = Automaton()
auto.add_word('wounded', 'wounded')

auto.make_automaton()

for item in auto.iter('Winning \U0001F629 so gutted, can\'t do anything for 4 weeks... Myth. #wounded'):
    print(item)

for item in auto.iter('Winning so gutted, can\'t do anything for 4 weeks... Myth. #wounded'):
    print(item)
