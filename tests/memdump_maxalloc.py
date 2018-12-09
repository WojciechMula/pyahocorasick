import sys

def main():
    try:
        path = sys.argv[1]
    except IndexError:
        path = 'memory.dump'

    app = Application(path)
    app.run()


class Application(object):
    def __init__(self, path):
        self.path = path


    def run(self):
        with open(self.path, 'rt') as f:
            print(max(self.ids(f)))


    def ids(self, file):
        for i, line in enumerate(file):
            fields = line.split()
            if fields[0] == 'A':
                yield int(fields[1])


if __name__ == '__main__':
    main()

