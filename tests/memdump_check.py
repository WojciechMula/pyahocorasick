import sys

def main():
    try:
        path = sys.argv[1]
    except IndexError:
        path = 'memory.dump'

    app = Application(path)
    if app.run():
        sys.exit(0)
    else:
        sys.exit(1)


class Application(object):
    def __init__(self, path):
        self.path = path
        self.memory = {}


    def run(self):
        with open(self.path, 'rt') as f:
            self.analyze(f)

        self.print_leaks()

        return len(self.memory) == 0


    def analyze(self, file):
        self.memory  = {}

        for i, line in enumerate(file):
            fields = line.split()
            if fields[0] == 'A':
                id   = fields[1]
                addr = fields[2]
                size = int(fields[3])

                assert addr not in self.memory
                self.memory[addr] = (id, size)

            elif fields[0] == 'F':

                addr = fields[1]
                if addr in self.memory: # at the moment we don't intercept realloc
                    del self.memory[addr]


    def print_leaks(self):
        n = len(self.memory)
        if n == 0:
            return

        print('There are %d leaks:' % n)
        for addr, (id, size) in self.memory.items():
            print('#%s: %s %d' % (id, addr, size))


if __name__ == '__main__':
    main()

