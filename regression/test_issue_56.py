import ahocorasick




def iter_results(s):
    r = []
    for x in A.iter(teststr):
        r.append(x)

    return r


def find_all_results(s):

    r = []
    
    def append(x, s):
        r.append((x, s))

    A.find_all(s, append)

    return r


A = ahocorasick.Automaton()

for word in ("poke", "go", "pokegois", "egoist"):
    A.add_word(word, word)

A.make_automaton()

teststr = 'pokego pokego  pokegoist'
expected = iter_results(teststr)
findall  = find_all_results(teststr)

if findall != expected:
    print("expected: %s" % expected)
    print("findall : %s" % findall)
    assert findall == expected

