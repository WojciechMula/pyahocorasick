import sys
import os
import random
import gzip
import pickle
import optparse
import time

from optparse import OptionParser


def main():
    options = parse_args()
    app = TestApplication(options)
    app.run()


chars = 'abcdefghijklmnopqestuvwxyzABCDEFGHIJKLMNOPQESTUVWXYZ0123456789.,;:-'

class TestApplication(object):
    def __init__(self, options):
        self.options = options

        random.seed(options.seed)


    def run(self):
        n = self.options.words

        for i in range(n):
            print(self.generate_random_word())


    def generate_random_word(self):
        n = random.randint(1, self.options.maxlength + 1)
        s = ''
        for i in range(n):
            s += random.choice(chars)

        return s


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

    (options, rest) = parser.parse_args()

    return options


if __name__ == '__main__':
    main()
