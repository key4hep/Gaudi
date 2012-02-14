#!/usr/bin/env python
"""
Small tool to generate the heptools toolchain from a given LCGCMT.
"""
__author__ = "Marco Clemencic <marco.clemencic@cern.ch>"

import os
import re

class HepToolsGenerator(object):
    """
    Class wrapping the details needed to generate the toolchain file from LCGCMT.
    """
    __header__ = """include(${CMAKE_CURRENT_LIST_DIR}/heptools-common.cmake)

# please keep alphabetic order and the structure (tabbing).
# it makes it much easier to edit/read this file!
"""
    __trailer__ = """
# Prepare the search paths according to the versions above
LCG_prepare_paths()"""

    __AA_projects__ = ("COOL", "CORAL", "RELAX", "ROOT")

    __special_dirs__ = {"CLHEP": "clhep",
                        "fftw": "fftw3",
                        "Frontier_Client": "frontier_client",
                        "GCCXML":  "gccxml",
                        }

    def __init__(self, lcgcmt_root):
        """
        Prepare the instance.

        @param lcgcmt_root: path to the root directory of a given LCGCMT version
        """
        self.lcgcmt_root = lcgcmt_root

    def __repr__(self):
        """
        Representation of the instance.
        """
        return "HepToolsGenerator(%r)" % self.lcgcmt_root

    @property
    def versions(self):
        """
        Extract the external names and versions from an installed LCGCMT.

        @return: dictionary mapping external names to versions
        """
        # We efxtract the lines defining the macros .*_config_version ...
        macro_re = re.compile(r'^\s*macro\s*(?P<name>\w*)_config_version\s*"(?P<version>[^"]*)"')
        # ... from the requirements file of the LCG_Configuration package
        req = open(os.path.join(self.lcgcmt_root, "LCG_Configuration", "cmt", "requirements"))

        return dict([(macro.group('name'), macro.group('version'))
                     for macro in map(macro_re.match, req)
                     if macro])

    def _content(self):
        """
        Generator producing the content (in blocks) of the toolchain file.
        """
        versions = self.versions

        yield self.__header__

        # first let's get rid of the special ones
        yield "set(LCG_config_version  %s)" % versions.pop("LCG")

        yield "\n# Application Area Projects"
        for name in self.__AA_projects__:
            # the width of the first string is bound to the length of the names
            # in self.__AA_projects__
            yield "LCG_AA_project(%-5s %s)" % (name, versions.pop(name))

        yield "\n# Compilers"
        # @FIXME: to be made cleaner and more flexible
        for compiler in [("gcc43", "gcc", "4.3.5"), ("gcc46", "gcc", "4.6.2")]:
            yield "LCG_compiler(%s %s %s)" % compiler

        yield "\n# Externals"
        lengths = (max(map(len, versions.keys())),
                   max(map(len, versions.values())),
                   max(map(len, self.__special_dirs__.values()))
                   )
        template = "LCG_external_package(%%-%ds %%-%ds %%-%ds)" % lengths
        for name in sorted(versions.keys()):
            # LCG_external_package(CLHEP            1.9.4.7             clhep)
            yield template % (name, versions[name], self.__special_dirs__.get(name, ""))

        yield self.__trailer__

    def __str__(self):
        """
        Return the content of the toolchain file.
        """
        return "\n".join(self._content())

if __name__ == '__main__':
    import sys
    if len(sys.argv) != 2 or not os.path.exists(sys.argv[1]):
        print "Usage : %s <path to LCGCMT version>" % os.path.basename(sys.argv[0])
        sys.exit(1)
    print HepToolsGenerator(sys.argv[1])
