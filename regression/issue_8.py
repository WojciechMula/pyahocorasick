# -*- coding: utf-8 -*-
import ahocorasick

test_sentences_rus = ["!ASM Print",
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

test_sentences_pl = [
    "wąż",  # a snake
    "mąż",  # a husband - why so similar :)
    "żółć",
    "aż",
    "waży"
]

def create_sutomata_rus():
    A = ahocorasick.Automaton()
    for sentences in test_sentences_rus[-7:]:
        for index, word in enumerate(sentences.split(' ')):
            A.add_word(word, (index, word))

    A.make_automaton()


def create_and_iter_sutomata_pl():
    A = ahocorasick.Automaton()
    for index, word in enumerate(test_sentences_pl):
        A.add_word(word, (index, word))

    A.make_automaton()
    for item in A.iter("wyważyć"):
        print(item)

if __name__ == '__main__':
    create_sutomata_rus()
    create_and_iter_sutomata_pl()
