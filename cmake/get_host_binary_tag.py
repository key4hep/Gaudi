#!/usr/bin/env python
#####################################################################################
# (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
'''
Inspect the system to return the BINARY_TAG string.

Note that the 4th element will be always "-opt".

Inspired by
* https://github.com/HEP-SF/documents/tree/master/HSF-TN/draft-2015-NAM
* https://github.com/HEP-SF/tools
'''
from __future__ import print_function

import os
import re
import sys
import platform
from subprocess import check_output, STDOUT
from collections import OrderedDict

# available flags per architecture
# source: https://gitlab.cern.ch/lhcb-core/LbPlatformUtils/blob/master/LbPlatformUtils/architectures.py
ARCH_DEFS = OrderedDict([
    ('cannonlake',
     set([
         'pclmulqdq', 'avx', 'pku', 'umip', 'avx512dq', 'bmi1', 'fsgsbase',
         'avx512ifma', 'avx512bw', 'clflushopt', 'sse4_2', 'sse4_1', 'lm',
         'adx', '3dnowprefetch', 'sha_ni', 'fma', 'mmx', 'avx512cd', 'avx512f',
         'pni', 'rdseed', 'popcnt', 'sse', 'f16c', 'xsavec', 'aes', 'avx2',
         'sse2', 'avx512vbmi', 'bmi2', 'ssse3', 'movbe', 'rdrand', 'avx512vl'
     ])),
    ('skylake_avx512',
     set([
         'pclmulqdq', 'avx', 'pku', 'avx512dq', 'fsgsbase', 'avx512bw',
         'clflushopt', 'sse4_2', 'sse4_1', 'lm', 'adx', '3dnowprefetch', 'fma',
         'mmx', 'avx512cd', 'avx512f', 'clwb', 'pni', 'rdseed', 'popcnt',
         'sse', 'f16c', 'xsavec', 'aes', 'avx2', 'sse2', 'bmi1', 'bmi2',
         'ssse3', 'movbe', 'rdrand', 'avx512vl'
     ])),
    ('skylake',
     set([
         'avx', 'fsgsbase', 'clflushopt', 'sse4_2', 'sse4_1', 'lm', 'adx',
         '3dnowprefetch', 'fma', 'mmx', 'pclmulqdq', 'pni', 'rdseed', 'popcnt',
         'sse', 'f16c', 'xsavec', 'aes', 'avx2', 'sse2', 'bmi1', 'bmi2',
         'ssse3', 'movbe', 'rdrand'
     ])),
    ('broadwell',
     set([
         'avx', 'fsgsbase', 'sse4_2', 'sse4_1', 'lm', 'adx', '3dnowprefetch',
         'fma', 'mmx', 'pclmulqdq', 'pni', 'rdseed', 'popcnt', 'sse', 'f16c',
         'aes', 'avx2', 'sse2', 'bmi1', 'bmi2', 'ssse3', 'movbe', 'rdrand'
     ])),
    ('haswell',
     set([
         'avx', 'fsgsbase', 'sse4_2', 'sse4_1', 'lm', 'fma', 'mmx', 'aes',
         'pni', 'popcnt', 'sse', 'f16c', 'pclmulqdq', 'avx2', 'sse2', 'bmi1',
         'bmi2', 'ssse3', 'movbe', 'rdrand'
     ])),
    ('ivybridge',
     set([
         'pni', 'aes', 'sse4_2', 'mmx', 'sse2', 'sse4_1', 'lm', 'pclmulqdq',
         'ssse3', 'fsgsbase', 'popcnt', 'rdrand', 'sse', 'avx', 'f16c'
     ])),
    ('sandybridge',
     set([
         'pni', 'aes', 'sse4_2', 'mmx', 'sse2', 'sse4_1', 'lm', 'pclmulqdq',
         'ssse3', 'popcnt', 'sse', 'avx'
     ])),
    ('westmere',
     set([
         'pni', 'aes', 'sse4_2', 'mmx', 'sse2', 'sse4_1', 'lm', 'pclmulqdq',
         'ssse3', 'popcnt', 'sse'
     ])),
    ('nehalem',
     set([
         'pni', 'sse4_2', 'mmx', 'sse2', 'sse4_1', 'lm', 'ssse3', 'popcnt',
         'sse'
     ])), ('core2', set(['pni', 'mmx', 'sse2', 'lm', 'ssse3', 'sse'])),
    ('x86_64', set([]))
])


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
    output = check_output([cmd, '-v'], stderr=STDOUT, env=env).decode('utf-8')
    m = re.search(r'(gcc|clang|icc|LLVM) version (\d+)\.(\d+)', output)
    if not m:  # prevent crashes if the compiler is not supported
        return 'unknown'
    comp = 'clang' if m.group(1) == 'LLVM' else m.group(1)
    vers = m.group(2)
    if (comp == 'gcc' and int(vers) < 7) or comp == 'clang':
        vers += m.group(3)
    return comp + vers


def compiler_id():
    return _compiler_version()


def arch():
    # Get host flags from /proc/cpuinfo
    host_flags = set()
    if sys.platform == 'darwin':
        for l in check_output(['sysctl', '-a']).decode('utf-8').split('\n'):
            if l.startswith('machdep.cpu.features') or l.startswith(
                    'machdep.cpu.extfeatures') or l.startswith(
                        'machdep.cpu.leaf7_features'):
                host_flags.update([f.lower() for f in l.split()[1:]])
    else:
        for l in open('/proc/cpuinfo'):
            if l.startswith('flags'):
                host_flags.update(l.split()[2:])
                break
    # compare with known arhitectures
    for arch, flags in ARCH_DEFS.items():
        if host_flags.issuperset(flags):
            return arch
    return 'x86_64'  # if nothing is found, assume x86_64


print('-'.join([arch(), os_id(), compiler_id(), 'opt']))
