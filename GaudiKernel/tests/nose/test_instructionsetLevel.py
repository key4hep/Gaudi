import os
from subprocess import Popen, PIPE


def test():
    out = Popen(['instructionsetLevel.exe', 'all'],
                stdout=PIPE).communicate()[0]
    out = out.decode('utf-8')
    known_flags = set(l.strip() for l in out.splitlines())

    expected = set()
    for l in open('/proc/cpuinfo'):
        if l.startswith('flags'):
            flags = set(l.strip().split())
            if 'pni' in flags:
                flags.add('sse3')
            expected = flags.intersection(known_flags)
            break

    out = Popen(['instructionsetLevel.exe'], stdout=PIPE).communicate()[0]
    out = out.decode('utf-8')
    found = set(l.strip() for l in out.splitlines())
    assert expected == found, ('expected: {0}, found: {1}'.format(
        sorted(expected), sorted(found)))
