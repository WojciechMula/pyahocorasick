import ahocorasick
import sys
import os

ac = ahocorasick.Automaton()
ac.add_word('SSSSS', 1)
ac.make_automaton()

def get_memory_usage():
    # Linux only
    pid = os.getpid()

    lines = []
    try:
        with open('/proc/%d/status' % pid, 'rt') as f:
            lines = f.readlines()
    except:
        pass

    for line in lines:
        if line.startswith('VmSize'):
            return float(line.split()[1])

    return 0

def test():
    with open('README.rst', 'r') as f:
        data = f.read()[:1024*2]

    for loop in range(1000):
        for start in range(0, len(data) - 20):
            ac.iter(data, start)


if __name__ == '__main__':
    
    before = get_memory_usage()
    test()
    after = get_memory_usage()

    print("Memory's usage growth: %s (before = %s, after = %s)" % (after - before, before, after))
    assert(before == after)
