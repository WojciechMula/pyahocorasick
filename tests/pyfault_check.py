import sys

def main():
    path = sys.argv[1]

    app = Application(path)
    app.run()


class Application(object):
    def __init__(self, path):
        self.path = path

    def run(self):
        with open(self.path, 'rt') as f:
            lines = [line.rstrip() for line in f if line.rstrip()]
            self.analyze(lines)


    def analyze(self, lines):
        error_sep     = '======================================================================'
        traceback_sep = '----------------------------------------------------------------------'

        index  = 0
        while True:
            try:
                index = lines.index(error_sep, index)
            except ValueError:
                break

            index += 1
            function = lines[index]
            index += 1
            start = lines.index(traceback_sep, index)
            end   = lines.index(traceback_sep, start + 1)
            index = end + 1

            error = lines[end - 1]
            print('%s: %s' % (function, error))


if __name__ == '__main__':
    main()
