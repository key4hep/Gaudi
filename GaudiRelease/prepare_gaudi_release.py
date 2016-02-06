#!/usr/bin/env python
'''
Script to prepare the release of Gaudi.

@author Marco Clemencic
'''

import os
import sys
import logging
import re
import ConfigParser
from subprocess import Popen, PIPE

def checkGitVersion():
    '''
    Ensure we have a usable version of Git (>= 1.7.9.1).

    See:
    * https://raw.githubusercontent.com/git/git/master/Documentation/RelNotes/1.7.9.1.txt
    * https://github.com/git/git/commit/36ed1913e1d5de0930e59db6eeec3ccb2bd58bd9
    '''
    proc = Popen(['git', '--version'], stdout=PIPE)
    version = proc.communicate()[0].split()[-1]
    if proc.returncode:
        raise RuntimeError('could not get git version')
    if versionKey(version) < versionKey('1.7.9.1'):
        raise RuntimeError('bad version of git found: %s (1.7.9.1 required)' % version)

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
    Return the latest Gaudi tag (of the format "GAUDI/GAUDI_*").
    '''
    logging.info('looking for latest tag')
    cmd = ['git', 'tag']
    logging.debug('using command %r', cmd)
    proc = Popen(cmd, stdout=PIPE)
    output = proc.communicate()[0]
    tags = [tag
            for tag in output.splitlines()
            if tag.startswith('GAUDI/GAUDI_')]
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
    proc = Popen(cmd, stdout=PIPE)
    return proc.communicate()[0]

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
        notes_file.writelines(dropuntil(re.compile(r'^!?============').match, orig_data))

# guess current version
_req_version_pattern = re.compile(r"^\s*version\s*(v[0-9]+r[0-9]+(?:p[0-9]+)?)\s*$")
_cml_version_pattern = re.compile(r"^\s*gaudi_subdir\s*\(\s*\S+\s+(v[0-9]+r[0-9]+(?:p[0-9]+)?)\)\s*$")
def extract_version(path):
    """
    Find the version number of a subdirectory.
    """
    global _cml_version_pattern
    for l in open(os.path.join(path, 'CMakeLists.txt')):
        m = _cml_version_pattern.match(l)
        if m:
            return m.group(1)
    return None

def change_cml_version(cml, newversion):
    if os.path.exists(cml):
        out = []
        changed = False
        for l in open(cml):
            m = _cml_version_pattern.match(l)
            if m and m.group(1) != newversion:
                logging.debug('%s: %s -> %s', cml, m.group(1), newversion)
                l = l.replace(m.group(1), newversion)
                changed = True
            out.append(l)
        if changed:
            open(cml, "w").writelines(out)

def change_version(packagedir, newversion):
    """
    Compare the version of the package with the new one and update the package if
    needed.

    Returns true if the package have been modified.
    """
    global _req_version_pattern
    changed = False
    out = []
    req = os.path.join(packagedir, 'cmt', 'requirements')
    for l in open(req):
        m = _req_version_pattern.match(l)
        if m:
            if m.group(1) != newversion:
                logging.debug('%s: %s -> %s', packagedir, m.group(1), newversion)
                l = l.replace(m.group(1),newversion)
                changed = True
        out.append(l)
    if changed:
        open(req,"w").writelines(out)
    # verify the version.cmt file
    ver = os.path.join(packagedir,"version.cmt")
    if os.path.exists(ver):
        current = open(ver).read().strip()
        if current != newversion:
            open(ver,"w").write(newversion + "\n")
    # update CMakeLists.txt
    cml = os.path.normpath(os.path.join(packagedir, 'CMakeLists.txt'))
    change_cml_version(cml, newversion)
    if 'GaudiKernel' in packagedir:
        cml = os.path.normpath(os.path.join(packagedir, 'src', 'Util', 'CMakeLists.txt'))
        change_cml_version(cml, newversion)
    return changed

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
                               if not dirname.startswith('build.') and
                                  dirname != 'cmake']

    # Packages which version must match the version of the project
    special_subdirs = ["Gaudi", "GaudiExamples", "GaudiSys", "GaudiRelease"]

    # Ask for the version of the project
    latest_tag = findLatestTag()

    old_version = latest_tag.split('_')[-1]
    new_version = raw_input("The old version of the project is %s, which is the new one? " % old_version)

    old_versions = {}
    release_notes = {}
    new_versions = {}
    # for each package in the project check if there were changes and ask for the new version number
    for pkgdir in all_subdirs():
        cmlfile = os.path.join(pkgdir, 'CMakeLists.txt')
        reqfile = os.path.join(pkgdir, 'cmt', 'requirements')

        pkg = os.path.basename(pkgdir)
        old_vers = extract_version(pkgdir)

        vers = None
        if pkg in special_subdirs:
            vers = new_version
        else:
            git_log = Popen(['git', 'log', '-m', '--first-parent',
                             '--stat', latest_tag + '..master', pkgdir],
                            stdout=PIPE).communicate()[0].strip()
            if git_log:
                msg = ('\nThe old version of {0} is {1}, these are the changes:\n'
                       '{2}\n'
                       'Which version you want (old is {1})? ').format(pkg,
                                                                       old_vers,
                                                                       git_log)
                vers = raw_input(msg)
                while not vers or vers == old_vers:
                    vers = raw_input('give me a version, please. ')
        if vers:
            change_version(pkgdir, vers)
            updateReleaseNotes(pkgdir,
                               tag_bar(pkg, vers) + '\n\n' +
                               releaseNotes(pkgdir, latest_tag, 'master'))
        new_versions[pkg] = vers or old_vers
        print "=" * 80

    # The changes in the GaudiRelease requirements for the other packages can be postponed to now
    reqfile = os.path.join('GaudiRelease', 'cmt', 'requirements')
    out = []
    for l in open(reqfile):
        sl = l.strip().split()
        if sl and sl[0] == "use":
            if sl[1] in new_versions:
                if sl[2] != new_versions[sl[1]]:
                    l = l.replace(sl[2], new_versions[sl[1]])
        out.append(l)
    open(reqfile, "w").writelines(out)
    # Update project.info
    config = ConfigParser.ConfigParser()
    config.optionxform = str # make the options case sensitive
    if os.path.exists('project.info'):
        config.read('project.info')
    if not config.has_section('Packages'):
        config.add_section('Packages')
    for pack_vers in sorted(new_versions.items()):
        config.set('Packages', *pack_vers)
    config.write(open('project.info', 'wb'))

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
