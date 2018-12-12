import sys
import os
import random
import gzip
import pickle
import optparse

import ahocorasick

from optparse import OptionParser


def main():
    options = parse_args()
    app = TestApplication(options)
    app.run()


chars = 'abcdefghijklmnopqestuvwxyzABCDEFGHIJKLMNOPQESTUVWXYZ0123456789.,;:-'

class TestApplication(object):
    def __init__(self, options):
        self.options = options
        self.words   = []

        random.seed(options.seed)


    def run(self):
        self.A = ahocorasick.Automaton()

        self.add_words()
        self.remove()


    def add_words(self):
        if self.options.random:
            self.__add_random_words()
        else:
            self.__add_from_file()

        print("Automaton statistics:")
        d = self.A.get_stats()
        print("- nodes_count  : %d" % d['nodes_count'])
        print("- words_count  : %d" % d['words_count'])
        print("- links_count  : %d" % d['links_count'])
        print("- longest_word : %d" % d['longest_word'])
        print("- sizeof_node  : %d" % d['sizeof_node'])
        print("- total_size   : %d" % d['total_size'])


    def remove(self):
        print("Removing %d words" % len(self.words))
        random.shuffle(self.words)
        for word in self.words:
            self.A.remove_word(word)


        print("Automaton statistics:")
        d = self.A.get_stats()
        print("- nodes_count  : %d" % d['nodes_count'])
        print("- words_count  : %d" % d['words_count'])
        print("- links_count  : %d" % d['links_count'])
        print("- longest_word : %d" % d['longest_word'])
        print("- sizeof_node  : %d" % d['sizeof_node'])
        print("- total_size   : %d" % d['total_size'])

    def __add_random_words(self):
        n = self.options.words

        print("Adding %d words" % n)
        while n > 0:
            word = self.generate_random_word()
            if self.A.add_word(word, True):
                n -= 1
                self.words.append(word)


    def __add_from_file(self):
        n = self.options.words

        print("Adding %d words from %s" % (n, self.options.file_gz))
        for i, word in enumerate(self.read()):
            if i > n:
                return

            self.A.add_word(word, True)
            self.words.append(word)


    def generate_words(self):
        if self.options.random:
            self.__generate_random_words()
        else:
            self.__load_words()


    def __generate_random_words(self):
        n = self.options.words

        print("Generating %d words" % n)
        while len(self.words) < n:
            word = self.generate_random_word()
            self.words.add(word)


    def __load_words(self):
        n = self.options.words
        print ("Loading %d words from %s" % (n, self.options.file_gz))
        for i, word in enumerate(self.read()):
            if i < n:
                self.words.add(word)
            else:
                return


    def read(self):
        with gzip.open(self.options.file_gz, "rt", encoding="utf-8") as f:
            for line in f:
                yield line.strip()


    def generate_random_word(self):
        n = random.randint(1, self.options.maxlength + 1)
        s = ''
        for i in range(n):
            s += random.choice(chars)

        return s


def format_size(size):
    units = [
        ('GB', 1024**3),
        ('MB', 1024**2),
        ('kB', 1024),
    ]

    for suffix, threshold in units:
        if size > threshold:
            return '%0.2f %s (%d bytes)' % (float(size)/threshold, suffix, size)

    return '%d bytes' % size


def parse_args():

    parser = OptionParser()
    parser.add_option(
        "--max-words", dest='words', type=int, default=50000, metavar='N',
        help="maximum number of words generated/loaded"
    )

    parser.add_option(
        "--random", dest='random', action='store_true', default=False,
        help="generate random words"
    )

    parser.add_option(
        "--seed", dest='seed', type=int, default=0, metavar='INT',
        help="random seed"
    )

    parser.add_option(
        "--random-max-len", dest='maxlength', type=int, default=100, metavar='K',
        help="maximum count of characters in a word"
    )

    parser.add_option(
        "--file-gz", metavar='FILE',
        help="load words from utf8-encoded gz file"
    )

    (options, rest) = parser.parse_args()

    if not (options.file_gz or options.random):
        raise parser.error("pass --random or --file-gz option")

    return options


if __name__ == '__main__':
    main()
