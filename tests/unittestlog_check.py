import sys

def main():
    path = sys.argv[1]

    app = Application(path)
    if app.run():
        sys.exit(0)
    else:
        sys.exit(1)


class Application(object):
    def __init__(self, path):
        self.path = path

    def run(self):
        with open(self.path, 'rt') as f:
            lines = [line.rstrip() for line in f if line.rstrip()]
            errors = self.analyze(lines)

        return errors


    def analyze(self, lines):
        error_sep     = '======================================================================'
        traceback_sep = '----------------------------------------------------------------------'

        index  = 0
        result = True
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
            if error != 'MemoryError':
                print('%s: %s' % (function, error))
                result = False

        return result


if __name__ == '__main__':
    main()
