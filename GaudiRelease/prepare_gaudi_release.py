#!/usr/bin/env python
'''
Script to prepare the release of Gaudi.

@author Marco Clemencic
'''

import os
import sys
import logging
import re
from subprocess import check_output, CalledProcessError


def checkGitVersion():
    '''
    Ensure we have a usable version of Git (>= 1.7.9.1).

    See:
    * https://raw.githubusercontent.com/git/git/master/Documentation/RelNotes/1.7.9.1.txt
    * https://github.com/git/git/commit/36ed1913e1d5de0930e59db6eeec3ccb2bd58bd9
    '''
    version = check_output(['git', '--version']).split()[-1]
    if versionKey(version) < versionKey('1.7.9.1'):
        raise RuntimeError(
            'bad version of git found: %s (1.7.9.1 required)' % version)


_VK_RE = re.compile(r'(\d+|\D+)')


def versionKey(x):
    '''
    Key function to be passes to list.sort() to sort strings
    as version numbers.
    '''
    return [int(i) if i[0] in '0123456789' else i
            for i in _VK_RE.split(x)
            if i]


def findLatestTag():
    '''
    Return the latest Gaudi tag (of the format "v*r*...").
    '''
    logging.info('looking for latest tag')
    cmd = ['git', 'tag']
    logging.debug('using command %r', cmd)
    output = check_output(cmd)
    vers_exp = re.compile(r'^v\d+r\d+(p\d+)?$')
    tags = [tag
            for tag in output.splitlines()
            if vers_exp.match(tag)]
    if tags:
        tags.sort(key=versionKey)
        logging.info('found %s', tags[-1])
        return tags[-1]
    logging.info('no valid tag found')


def releaseNotes(path=os.curdir, from_tag=None, branch=None):
    '''
    Return the release notes (in the old LHCb format) extracted from git
    commits for a given path.
    '''
    cmd = ['git', 'log',
           '--first-parent', '--date=short',
           '--pretty=format:! %ad - commit %h%n%n%w(80,1,3)- %s%n%n%b%n']
    if from_tag:
        cmd.append('{0}..{1}'.format(from_tag, branch or ''))
    elif branch:
        cmd.append(branch)
    cmd.append('--')
    cmd.append(path)
    logging.info('preparing release notes for %s%s', path,
                 ' since ' + from_tag if from_tag else '')
    logging.debug('using command %r', cmd)
    # remove '\r' characters from commit messages
    out = check_output(cmd).replace('\r', '')

    # replace ' - commit 123abc' with ' - Contributor Name (commit 123abc)'
    def add_contributors(match):
        try:
            authors = set(check_output(['git', 'log', '--pretty=format:%aN',
                                        '{0}^1..{0}^2'.format(match.group(1)),
                                        ]).splitlines())
            return ' - {0} (commit {1})'.format(', '.join(sorted(authors)),
                                                match.group(1))
        except CalledProcessError:
            return match.group(0)
    out = re.sub(r' - commit ([0-9a-f]+)', add_contributors, out)
    return out


def updateReleaseNotes(path, notes):
    '''
    Smartly prepend the content of notes to the release.notes file in path.
    '''
    notes_filename = os.path.join(path, 'doc', 'release.notes')
    logging.info('updating %s', notes_filename)
    from itertools import takewhile, dropwhile

    def dropuntil(predicate, iterable):
        return dropwhile(lambda x: not predicate(x), iterable)
    with open(notes_filename) as notes_file:
        orig_data = iter(list(notes_file))
    header = takewhile(str.strip, orig_data)
    with open(notes_filename, 'w') as notes_file:
        notes_file.writelines(header)
        notes_file.write('\n')
        notes_file.writelines(l.rstrip() + '\n' for l in notes.splitlines())
        notes_file.write('\n')
        notes_file.writelines(
            dropuntil(re.compile(r'^!?============').match, orig_data))


def tag_bar(pkg, version=None):
    title = ' %s %s ' % (pkg, version)
    letf_chars = (78 - len(title)) / 2
    right_chars = 78 - letf_chars - len(title)
    separator = ('=' * letf_chars) + title + ('=' * right_chars)
    return separator


def main():
    logging.basicConfig(level=logging.DEBUG)
    os.chdir(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    # Find the version of HEPTools (LCG)
    for l in open('toolchain.cmake'):
        m = re.match(r'^\s*set\(\s*heptools_version\s+(\S*)\s*\)', l)
        if m:
            HEPToolsVers = m.group(1)
            print "Using HEPTools", HEPToolsVers
            break
    else:
        logging.error('Cannot find HEPTools version')
        sys.exit(1)

    # Collect all the packages in the project with their directory
    def all_subdirs():
        for dirpath, dirnames, filenames in os.walk(os.curdir):
            if 'CMakeLists.txt' in filenames and dirpath != os.curdir:
                dirnames[:] = []
                yield dirpath
            else:
                dirnames[:] = [dirname for dirname in dirnames
                               if (not dirname.startswith('build.') and
                                   dirname != 'cmake')]

    # Ask for the version of the project
    latest_tag = findLatestTag()

    old_version = latest_tag.split('_')[-1]
    new_version = raw_input(("The old version of the project is %s, "
                             "which is the new one? ") % old_version)

    release_notes = {}
    # for each package in the project update the release.notes
    for pkgdir in all_subdirs():
        updateReleaseNotes(pkgdir,
                           tag_bar('Gaudi', new_version) + '\n\n' +
                           releaseNotes(pkgdir, latest_tag, 'master'))

    # update the global CMakeLists.txt
    out = []
    for l in open('CMakeLists.txt'):
        if l.strip().startswith('gaudi_project'):
            l = 'gaudi_project(Gaudi %s)\n' % new_version
        out.append(l)
    open('CMakeLists.txt', "w").writelines(out)


if __name__ == '__main__':
    checkGitVersion()
    main()
