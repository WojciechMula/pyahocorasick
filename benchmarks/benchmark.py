from time import clock
from random import choice, randint, seed
from sys import stdout

import ahocorasick


def write(str):
    stdout.write(str)
    stdout.flush()


def writeln(str):
    stdout.write(str)
    stdout.write('\n')


class ElapsedTime:
    def __init__(self, msg):
        self.msg = msg

    def __enter__(self):
        write("%-40s: " % self.msg)
        self.start = clock()

    def __exit__(self, a1, a2, a3):
        self.stop = clock()
        writeln("%0.3f s" % self.get_time())

    def get_time(self):
        return self.stop - self.start


class Test:

    def __init__(self, max_word_length, count):
        self.min_word_length = 3
        self.max_word_length = max_word_length
        self.count = count
        self.words = []
        self.inexisting = []
        self.input = ""

        self.automaton = None
        seed(0) # make sure that tests will be repeatable

    def init_data(self):
        
        def random_word(length):
            chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
            return ''.join(choice(chars) for _ in xrange(length))

        for i in xrange(self.count):
            length = randint(self.min_word_length, self.max_word_length)
            self.words.append(random_word(length))

            length = randint(self.min_word_length, self.max_word_length)
            self.inexisting.append(random_word(length))


        self.input = random_word(self.count)

        assert(len(self.words) == len(self.inexisting))

    def add_words(self):
        
        self.automaton = ahocorasick.Automaton()
        A = self.automaton
        for word in self.words:
            A.add_word(word, word)

    def build(self):
        
        self.automaton.make_automaton()

    def lookup(self):

        n = len(self.words)

        A = self.automaton
        for i in xrange(n):
            A.get(self.words[i])
            A.get(self.inexisting[i], "unknown")
    

    def search(self):

        A = self.automaton
        n = 0
        for item in A.iter(self.input):
            n += 1


    def run(self):
        
        with ElapsedTime("Generating data (%d words)" % self.count):
            self.init_data()
        
        with ElapsedTime("Add words"):
            self.add_words()
        
        with ElapsedTime("Building automaton"):
            self.build()

        with ElapsedTime("Look up"):
            self.lookup()

        with ElapsedTime("Search"):
            self.search()

def main():
    
    test = Test(32, 1000000)
    test.run()

if __name__ == '__main__':
    main()
