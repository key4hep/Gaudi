# -*-coding:utf8 -*

from subprocess import *
import subprocess
import os

#-------------------------------------------------------------------------#

def main():

    print 'starting'
    import dependencyAnalysis as DA

    dir = "./newFormat"
    fileList = sorted([ l for l in os.listdir(dir) if not l.startswith('.')])
    for l in fileList:
        n=fileList.index(l)
        l = dir+'/'+l
        fileList[n]=l

    testList = ''
    for l in fileList :
        testList+= l+" "

#     print DA.sniffer('/afs/cern.ch/user/v/valentin/workspace/Gaudi/GaudiExamples/tests/qmtest')\
    proc= Popen(['/afs/cern.ch/user/v/valentin/workspace/Gaudi/build.x86_64-slc6-gcc48-opt/run', "/afs/cern.ch/sw/lcg/external/Python/2.7.3/x86_64-slc6-gcc48-opt/bin/python", '/afs/cern.ch/user/v/valentin/workspace/Gaudi/GaudiExamples/tests/qmtest/ScriptTest_V6.py']+fileList)
    proc.wait()

    print 'end'

main()