import pyahocorasick

test_cases = [
    # example provided by @Ulitochka
    {
        'words'     : ["alpha", "alpha beta", "gamma", "gamma alpha"],
        'input'     : "I went to alpha beta the alpha other day gamma alpha to pick up some spam",
        'expected'  : [("alpha beta", 19), ("alpha", 29), ("gamma alpha", 51)]
    },

    {
        'words'     : ["alpha", "alpha beta", "beta gamma", "gamma"],
        'input'     : "Cats have not idea what alpha beta gamma means",
        'expected'  : [("alpha beta", 33), ("gamma", 39)]
    },

    {
        'words'     : ["alpha", "alpha beta", "beta gamma", "gamma"],
        'input'     : "Cats have not idea what alpha beta gamma",
        'expected'  : [("alpha beta", 33), ("gamma", 39)]
    },
]


def test(case):

    tree = pyahocorasick.Trie()
    for word in case['words']:
        tree.add_word(word, word)

    tree.make_automaton()

    actual = [item for item in tree.iter_long(case['input'])]

    if actual != case['expected']:
        print("ERROR:")
        print(actual)
        print(case['expected'])
        assert(False)


if __name__ == '__main__':
    for data in test_cases:
        test(data)

    print("OK")
