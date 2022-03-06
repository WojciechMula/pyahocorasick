import ahocorasick


def iter_results(teststr, automaton):
    r = []
    for x in automaton.iter(teststr):
        r.append(x)

    return r


def find_all_results(teststr, automaton):
    r = []

    def append(x, teststr):
        r.append((x, teststr))

    automaton.find_all(teststr, append)
    return r


def test_issue56():
    automaton = ahocorasick.Automaton()

    for word in ("poke", "go", "pokegois", "egoist"):
        automaton.add_word(word, word)

    automaton.make_automaton()

    teststr = 'pokego pokego  pokegoist'
    expected = iter_results(teststr, automaton)
    findall = find_all_results(teststr, automaton)

    assert findall == expected

