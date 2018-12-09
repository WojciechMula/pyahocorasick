import sys
import os
import random
import pickle

import ahocorasick


def main():
    options = Options()
    app = TestApplication(options)
    app.run()


class Options(object):
    def __init__(self):
        self.seed       = 0
        self.words      = 30000
        self.maxlength  = 100
        try:
            self.picklepath = sys.argv[1]
        except IndexError:
            self.picklepath = 'pickle_stresstest.pickle'


        self.save = True
        self.load = True
        self.compare = True


chars = 'abcdefghijklmnopqestuvwxyzABCDEFGHIJKLMNOPQESTUVWXYZ0123456789.,;:-'

class TestApplication(object):
    def __init__(self, options):
        self.options = options
        self.words   = set()

        random.seed(options.seed)

    
    def run(self):
        self.A = ahocorasick.Automaton()

        if self.options.compare and not self.options.save:
            self.generate_words()

        if self.options.save:
            self.add_words()
            self.pickle()
            self.A.clear()

        if self.options.load:
            self.unpickle()

        if self.options.compare:
            self.compare()
    

    def add_words(self):
        n = self.options.words

        print("Adding %d words" % n)
        while n > 0:
            word = self.generate_random_word()
            if self.options.compare:
                self.words.add(word)

            if self.A.add_word(word, True):
                n -= 1

        print("Automaton statistics:")
        d = self.A.get_stats()
        print("- nodes_count  : %d" % d['longest_word'])
        print("- words_count  : %d" % d['words_count'])
        print("- links_count  : %d" % d['links_count'])
        print("- longest_word : %d" % d['longest_word'])
        print("- sizeof_node  : %d" % d['sizeof_node'])
        print("- total_size   : %d" % d['total_size'])


    def generate_words(self):
        n = self.options.words

        print("Generating %d words" % n)
        while len(self.words) < n:
            word = self.generate_random_word()
            self.words.add(word)


    def pickle(self):
        path = self.options.picklepath

        print("Saving automaton in %s" % path)
        with open(path, 'wb') as f:
            pickle.dump(self.A, f)

        size = os.path.getsize(path)
        print("   file size is %s" % format_size(size))


    def unpickle(self):
        path = self.options.picklepath

        print("Loading automaton from %s" % path)
        with open(path, 'rb') as f:
            self.A = pickle.load(f)


    def compare(self):
        print("Comparing added words with restored")

        for word in self.A:
            self.words.remove(word)

        if self.words:
            print("Not all words were restored (%d missing)" % len(self.words))


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


if __name__ == '__main__':
    main()
