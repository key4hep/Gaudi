#!/usr/bin/env python
'''
Inspect the system to return the BINARY_TAG string.

Note that the 4th element will be always "-opt".

Inspired by
* https://github.com/HEP-SF/documents/tree/master/HSF-TN/draft-2015-NAM
* https://github.com/HEP-SF/tools
'''

import os
import re
import platform
from subprocess import check_output, STDOUT


def _Linux_os():
    dist = platform.linux_distribution(full_distribution_name=False)
    dist_name = dist[0].lower()
    dist_version = dist[1]
    if dist_name in ('redhat', 'centos'):
        if 'CERN' in open('/etc/%s-release' % dist_name).read():
            dist_name = 'slc'
        dist_version = dist_version.split('.', 1)[0]
    elif dist_name == 'debian':
        # there's a problem with vanilla Python not recognizing Ubuntu
        # see https://sft.its.cern.ch/jira/browse/SPI-961
        try:
            for l in open('/etc/lsb-release'):
                if l.startswith('DISTRIB_ID='):
                    dist_name = l.strip()[11:].lower()
                elif l.startswith('DISTRIB_RELEASE='):
                    dist_version = l.strip()[16:]
        except:
            pass  # lsb-release is missing
    if dist_name == 'ubuntu':
        dist_version = dist_version.replace('.', '')
    elif dist_name == '':
        # Fall back on a generic Linux build if distribution detection fails
        dist_name = 'linux'
    return dist_name + dist_version


def _Darwin_os():
    version = platform.mac_ver()[0].split('.')
    return 'macos' + ''.join(version[:2])


def _Windows_os():
    return 'win' + platform.win32_ver()[1].split('.', 1)[0]


def _unknown_os():
    return 'unknown'


os_id = globals().get('_%s_os' % platform.system(), _unknown_os)


def _compiler_version(cmd=os.environ.get('CC', 'cc')):
    # prevent interference from localization
    env = dict(os.environ)
    env['LC_ALL'] = 'C'
    m = re.search(r'(gcc|clang|icc|LLVM) version (\d+)\.(\d+)',
                  check_output([cmd, '-v'], stderr=STDOUT, env=env))
    if not m:  # prevent crashes if the compiler is not supported
        return 'unknown'
    comp = 'clang' if m.group(1) == 'LLVM' else m.group(1)
    vers = m.group(2)
    if (comp == 'gcc' and int(vers) < 7) or comp == 'clang':
        vers += m.group(3)
    return comp + vers


def compiler_id():
    return _compiler_version()


arch = platform.machine()
if arch == 'AMD64':  # this is what we get on Windows
    arch = 'x86_64'

print('-'.join([arch, os_id(), compiler_id(), 'opt']))
