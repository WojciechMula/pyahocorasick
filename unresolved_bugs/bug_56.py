import ahocorasick

try:
    xrange
except NameError:
    xrange = range

def get_words():
    with open('bug_56_words.txt') as f:
        for word in f:
            word = word.strip()
            yield word

            # yield all subwords
            n = len(word) + 1
            for i in xrange(0, n):
                for j in xrange(i + 1, n):
                    yield word[i:j]

print("Adding words...")
A = ahocorasick.Automaton()

for word in get_words():
    if not word:
        continue

    A.add_word(word, word)

print("%d words added" % (A.get_stats()['words_count']))

print("Building an automaton...")
A.make_automaton()

print("Test")

def p(*args):
    print(args)

A.find_all('pokego', p)
