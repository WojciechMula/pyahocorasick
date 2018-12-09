import os
import sys

def main():
    app = Application(sys.argv[1], sys.argv[2])
    if app.run():
        sys.exit(0)
    else:
        sys.exit(1)


class Application(object):
    def __init__(self, srcdir, path):
        self.srcdir  = srcdir
        self.path    = path
        self.sources = set()
        self.dump    = 0

    
    def run(self):
        self.gather_sources()

        with open(self.path, 'rt') as f:
            leaks = self.analyze(f)

        if not leaks:
            return True
        else:
            print("Following references found in %s (file -> line no)" % self.path)
            for name in sorted(leaks):
                lines = ', '.join(map(str, leaks[name]))
                print("- %s: %s" % (name, lines))


    def gather_sources(self):
        for path in os.listdir(self.srcdir):
            if path.endswith('.c'):
                self.sources.add(path)


    def analyze(self, file):
        result = {}
        for k, line in enumerate(file):
            if 'by 0x' in line or 'at 0x' in line:
                try:
                    # by 0xfffff: function (file.c:1234)
                    #                       ^^^^^^
                    index = line.rindex('(') + 1
                    name  = line[index:]
                    if name.startswith('in '):
                        continue

                    index = name.index(':')
                    name = name[:index]

                    if self.dump:
                        print(name)
                except ValueError:
                    continue

                if name in self.sources:
                    if name not in result:
                        result[name] = []

                    result[name].append(k)

        return result


if __name__ == '__main__':
    main()

