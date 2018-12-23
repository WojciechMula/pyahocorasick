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
            action = fields[0]
            if action == 'A':
                id   = fields[1]
                addr = fields[2]
                size = int(fields[3])

                assert addr not in self.memory
                self.memory[addr] = (id, size)

            elif action == 'R':

                id      = fields[1]
                oldaddr = fields[2]
                newaddr = fields[3]
                size    = int(fields[4])

                try:
                    key = int(oldaddr, 16)
                    del self.memory[oldaddr]
                except ValueError:
                    pass

                assert newaddr not in self.memory
                self.memory[newaddr] = (id, size)

            elif action == 'F':

                addr = fields[1]
                if addr in self.memory:
                    del self.memory[addr]


    def print_leaks(self):
        n = len(self.memory)
        if n == 0:
            return

        print('There are %d leaks:' % n)
        tmp = [(int(id), addr, size) for addr, (id, size) in self.memory.items()]
        tmp.sort(key=lambda item: item[0])

        for id, addr, size in tmp:
            print('#%s: %s %d' % (id, addr, size))


if __name__ == '__main__':
    main()

