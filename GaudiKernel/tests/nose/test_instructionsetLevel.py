import os
from subprocess import Popen, PIPE


def test():
    out = Popen(['instructionsetLevel.exe', 'all'],
                stdout=PIPE).communicate()[0]
    known_flags = set(l.strip() for l in out.splitlines())

    expected = set()
    for l in open('/proc/cpuinfo'):
        if l.startswith('flags'):
            expected = set(l.strip().split()).intersection(known_flags)
            break

    out = Popen(['instructionsetLevel.exe'], stdout=PIPE).communicate()[0]
    found = set(l.strip() for l in out.splitlines())
    # FIXME: these seem not to be reported by Linux
    found -= set(['sse3', 'avx512f'])
    assert expected == found, ('expected: {0}, found: {1}'.format(
        sorted(expected), sorted(found)))
