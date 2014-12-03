#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
from xml.etree import ElementTree as ET
from subprocess import Popen, PIPE
import re
import logging

def indent(elem, level=0):
    '''
    Add spaces and newlines to elements to allow pretty-printing of XML.

    http://effbot.org/zone/element-lib.htm#prettyprint
    '''
    i = "\n" + level*"  "
    if len(elem):
        if not elem.text or not elem.text.strip():
            elem.text = i + "  "
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
        for elem in elem:
            indent(elem, level+1)
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
    else:
        if level and (not elem.tail or not elem.tail.strip()):
            elem.tail = i

def CMakeParseArguments(args, options=None, single_value=None, multi_value=None):
    '''
    Parse a list or strings using the logic of the CMake function
    CMAKE_PARSE_ARGUMENTS.

    >>> res = CMakeParseArguments(['USE', 'Gaudi', 'v25r0', 'DATA', 'Det/SQLDDDB'],
    ... multi_value=['USE', 'DATA'])
    >>> res['USE']
    ['Gaudi', 'v25r0']
    >>> res['DATA']
    ['Det/SQLDDDB']
    >>> res['UNPARSED']
    []
    >>> res = CMakeParseArguments('a b OPTION1 c FLAG1 d OPTION2 e f'.split(),
    ... options=['FLAG1', 'FLAG2'],
    ... single_value=['OPTION1', 'OPTION2'])
    >>> res['FLAG1']
    True
    >>> res['FLAG2']
    False
    >>> res['OPTION1']
    'c'
    >>> res['OPTION2']
    'e'
    >>> res['UNPARSED']
    ['a', 'b', 'd', 'f']
    '''
    args = list(args)
    options = set(options or [])
    single_value = set(single_value or [])
    multi_value = set(multi_value or [])
    all_keywords = options.union(single_value).union(multi_value)
    result = {'UNPARSED': []}
    result.update((k, False) for k in options)
    result.update((k, None) for k in single_value)
    result.update((k, []) for k in multi_value)

    while args:
        arg = args.pop(0)
        if arg in options:
            result[arg] = True
        elif arg in single_value:
            result[arg] = args.pop(0)
        elif arg in multi_value:
            while args and args[0] not in all_keywords:
                result[arg].append(args.pop(0))
        else:
            result['UNPARSED'].append(arg)
    return result

if __name__ == '__main__':
    from optparse import OptionParser
    parser = OptionParser(usage='%prog [options] <cmake_lists> <lcg_version> <platform>')
    parser.add_option('-o', '--output', action='store',
                      help='output filename')

    opts, args = parser.parse_args()

    if len(args) != 3:
        parser.error('wrong number of arguments')

    cmake_lists, lcg_version, platform = args

    logging.basicConfig(level=logging.INFO)

    # look for the CMake configuration file
    if not os.path.exists(cmake_lists):
        print 'The project does not have a CMake configuration, I cannot produce a manifest.xml'
        sys.exit(0)

    project_args = []
    m = re.search(r'gaudi_project\s*\(([^)]*)\)',
                  open(cmake_lists).read(), re.MULTILINE)
    if m:
        project_args = m.group(1).split()
    if len(project_args) < 2:
        print 'error: invalid content of CMakeLists.txt'
        sys.exit(1)

    # parse gaudi_project arguments
    name, version = project_args[:2]
    project_args = project_args[2:]

    parsed_args = CMakeParseArguments(project_args,
                                      options=['FORTRAN'],
                                      multi_value=['USE', 'DATA'])

    # set the output
    if opts.output in ('-', None):
        output = sys.stdout
    else:
        output = opts.output

    # prepare the XML content
    manifest = ET.Element('manifest')
    manifest.append(ET.Element('project', name=name, version=version))

    heptools = ET.Element('heptools')
    ht_version = ET.Element('version')
    ht_version.text = lcg_version
    ht_bin_tag = ET.Element('binary_tag')
    ht_bin_tag.text = platform
    ht_system = ET.Element('lcg_system')
    ht_system.text = '-'.join(platform.split('-')[:-1])
    heptools.extend([ht_version, ht_bin_tag, ht_system])
    manifest.append(heptools)

    if parsed_args['USE']:
        used_projects = ET.Element('used_projects')
        used_projects.extend(ET.Element('project', name=name, version=version)
                             for name, version in zip(parsed_args['USE'][::2],
                                                      parsed_args['USE'][1::2]))
        manifest.append(used_projects)

    if parsed_args['DATA']:
        used_data_pkgs = ET.Element('used_data_pkgs')
        def data_pkgs(args):
            '''helper to translate the list of data packages'''
            args = list(args) # clone
            while args:
                pkg = args.pop(0)
                if len(args) >= 2 and args[0] == 'VERSION':
                    args.pop(0)
                    vers = args.pop(0)
                else:
                    vers = '*'
                yield (pkg, vers)
        used_data_pkgs.extend(ET.Element('package', name=pkg, version=vers)
                              for pkg, vers in data_pkgs(parsed_args['DATA']))
        manifest.append(used_data_pkgs)


    logging.debug('collecting external dependencies info (with CMT)')
    p = Popen(['cmt', 'show', 'uses'], stdout=PIPE)
    externals = [l.split()[1]
                 for l in p.stdout
                 if l.startswith('use') and
                    'LCG_Interfaces' in l]
    externals.sort()

    # get the versions of the externals
    def get_ext_vers(ext):
        '''
        Ask CMT the version of an external.
        '''
        logging.debug('getting version of %s', ext)
        vers = Popen(['cmt', 'show', 'macro_value',
                      '%s_native_version' % ext],
                     stdout=PIPE).communicate()[0].strip()
        logging.debug('using %s %s', ext, vers)
        if vers == 'dummy': # special case in LCG
            vers = ''
        return vers

    # mapping between LCG_Interface name and RPM name for special cases
    rpm_names = {'Expat': 'expat'}
    fix_rpm_name = lambda n: rpm_names.get(n, n)

    packages = ET.Element('packages')
    packages.extend([ET.Element('package', name=fix_rpm_name(ext), version=vers)
                     for ext, vers in [(ext, get_ext_vers(ext))
                                       for ext in externals]
                     if vers])
    heptools.append(packages)

    destdir = os.path.dirname(output)
    if not os.path.exists(destdir):
        logging.debug('creating directory %s', destdir)
        os.makedirs(destdir)

    # finally write the produced XML
    logging.debug('writing manifest file %s', output)
    indent(manifest)
    ET.ElementTree(manifest).write(output,
                                   encoding="UTF-8", xml_declaration=True)

    logging.debug('%s written', output)
