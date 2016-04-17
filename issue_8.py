import ahocorasick

test_sentences = ["!ASM Print",
"!ASM Print, tyre компания er",
"!ASM Print, рекламно-производственная компания rr",
"!Action Pact!",
"!T.O.O.H.!",
"!YES, лингвистический центр",
"!ts, магазин",
"!ФЕСТ",
'"100-th" department store',
'"1000 мелочей"',
'"1001 мелочь"',
'"19 отряд Федеральной противопожарной службы по Ленинградской области"',
'"У Друзей"',
'"ШТОРЫ и не только..."']

def create_sutomata_rus():
    A = ahocorasick.Automaton()
    for sentences in test_sentences[-7:]:
        for index, word in enumerate(sentences.split(' ')):
            A.add_word(word, (index, word))

    A.make_automaton()


if __name__ == '__main__':
    create_sutomata_rus()
