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

from __future__ import print_function

import os
import sys


def _getPath(pathstring=""):
    pthlist = []
    if pathstring == "":
        if sys.platform == "linux2":
            pathstring = os.environ["LD_LIBRARY_PATH"]
        else:
            pathstring = os.environ["PATH"]
    pthlist += pathstring.split(os.pathsep)
    return pthlist


def _getEntry(line):
    ll = line.split()
    entry = ll[0]
    entry = entry.replace("Library.", "")
    entry = entry.replace(":", "")
    entry = entry.replace("@", ":")
    entry = entry.replace("-", " ")
    lib = ll[1]
    return entry, lib


def _getBlock(line):
    if line.find("Begin"):
        block = line.split()[-1]
    return block


def _procRootMap(rtmpfile, rtmapdict):
    block = ""
    for line in open(rtmpfile, "r"):
        line = line[:-1]
        if line:
            if line[0] != "#":
                entry, lib = _getEntry(line)
                if entry not in rtmapdict:
                    rtmapdict[entry] = []
                rtmapdict[entry].append(
                    (os.path.join(os.path.dirname(rtmpfile), lib), block)
                )
            else:
                block = _getBlock(line)


def _procSysRootMap(rtmapdict):
    if "ROOTSYS" in os.environ:
        rtmpfile = os.path.join(os.environ["ROOTSYS"], "etc", "system.rootmap")
        block = ""
        for line in open(rtmpfile, "r"):
            line = line[:-1]
            if line:
                if line[0] != "#":
                    entry, lib = _getEntry(line)
                    if entry not in rtmapdict:
                        rtmapdict[entry] = []
                    rtmapdict[entry].append(
                        (os.path.join(os.environ["ROOTSYS"], "lib", lib), block)
                    )
                else:
                    block = _getBlock(line)
    else:
        print("WARNING: No ROOTSYS defined!")


def _isRootMap(filename):
    # The file must begin with "rootmap"
    if filename.find("rootmap") == 0:
        return True
    return False


def getMaps(pathstring="", sysrtmap=False):
    rtmapdict = dict()
    pthlist = _getPath(pathstring)
    if sysrtmap:
        _procSysRootMap(rtmapdict)
    for p in pthlist:
        try:
            for f in filter(_isRootMap, os.listdir(p)):
                rtmpfile = os.path.join(p, f)
                if os.path.exists(rtmpfile):
                    _procRootMap(rtmpfile, rtmapdict)
        except:
            pass
    return rtmapdict


def _getLongestEntry(maps):
    sz = 0
    for k in maps.keys():
        if len(k) > sz:
            sz = len(k)
    return sz


def printMaps(maps, recomp=None):
    linelen = _getLongestEntry(maps)
    frmat = r"%-" + str(linelen) + "s\t"
    kys = maps.keys()
    kys.sort()
    if recomp:
        kys = filter(recomp.search, kys)
    for k in kys:
        if len(maps[k]) > 1:
            print("!!!!!!!!!!!! WARNING - More than one entry !!!!!!!!!!")
        for l in maps[k]:
            print(frmat % k, end=" ")
            for m in l:
                print(m, end=" ")
            print(" ")
    return


def shortPrintMaps(maps, recomp=None):
    kys = maps.keys()
    kys.sort()
    if recomp:
        kys = filter(recomp.search, kys)
    for k in kys:
        if len(maps[k]) > 1:
            print(k, "!!!!!!!!!!!! WARNING - More than one entry !!!!!!!!!!")
        else:
            print(k)
        for l in maps[k]:
            for m in l:
                print("\t%s" % m, end=" ")
            print(" ")
    return


def printKeys(maps, recomp=None):
    kys = maps.keys()
    kys.sort()
    if recomp:
        kys = filter(recomp.search, kys)
    for k in kys:
        if len(maps[k]) > 1:
            print("!!!!!!!!!!!! WARNING - More than one entry !!!!!!!!!!")
        for l in maps[k]:
            print(k)
    return


def checkDict(maps, recomp=None):
    kys = maps.keys()
    kys.sort()
    if recomp:
        kys = filter(recomp.search, kys)
    for k in kys:
        if len(maps[k]) > 1:
            print("!!!!!!!!!!!! WARNING - More than one entry !!!!!!!!!!")
            print(k)
            for l in maps[k]:
                for m in l:
                    print("\t%s" % m, end=" ")
                print(" ")
    return
